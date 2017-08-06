#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

uint64_t totalbyteswritten;
uint64_t mismatcherrors;
uint64_t ioerrors;
uint32_t passage;
uint64_t writespeed;
uint64_t readspeed;
FILE* logfile;
char path[300];
time_t startrun;

volatile int shutdown = 0;

struct sigaction old_action;

uint32_t state0;
uint32_t state1;
uint32_t state2;
uint32_t state3;

void reseed(uint32_t seed)
{
	state0 = seed;
	state1 = 0;
	state2 = 0;
	state3 = 0;
}

uint32_t xorshift128()
{
	uint32_t t = state3;
	t ^= t << 11;
	t ^= t >> 8;
	state3 = state2; state2 = state1; state1 = state0;
	t ^= state0;
	t ^= state0 >> 19;	
	state0 = t;
	return t;
}

uint64_t tobytes(char * x)
{
	uint64_t bytes = 0;
	uint64_t multiplier = 1;
	int len = 0;
	while(x[len++]);	
	for(int i = 0; i < len - 1; i++)
	{
		if((x[i] >= '0') && (x[i] < '9'))
		{
			bytes *= 10;
			bytes += (x[i] - 48);
		}
		if((x[i] > 'A') && (x[i] < 'z'))
		{
			switch (x[i])
			{
				case 'T':
					multiplier = 1024LL*1024LL*1024LL*1024LL;
					break;
				case 't':
					multiplier = 1024LL*1024LL*1024LL*1024LL;
					break;
				case 'G':
					multiplier = 1024*1024*1024;
					break;
				case 'g':
					multiplier = 1024*1024*1024;
					break;
				case 'M':
					multiplier = 1024*1024;
					break;
				case 'm':
					multiplier = 1024*1024;
					break;
				case 'K':
					multiplier = 1024;
					break;
				case 'k':
					multiplier = 1024;
					break;
			}
		}
		bytes *= multiplier;
	}
	return bytes;
}

void bytestostr(uint64_t bytes, char * str)
{
	if(bytes < 1024) sprintf(str, "%i B", (int) bytes );
	if((bytes > 1024) && (bytes < 1024*1024)) sprintf(str, "%.1f KiB", bytes/1024.0);
	if((bytes > 1024*1024) && (bytes < 1024*1024*1024)) sprintf(str, "%.1f MiB", bytes/(1024.0*1024));
	if((bytes > 1024*1024*1024) && (bytes < 1024LL*1024LL*1024LL*1024LL)) sprintf(str, "%.1f GiB", bytes/(1024.0*1024.0*1024.0));
	if((bytes > 1024LL*1024LL*1024LL*1024LL)) sprintf(str, "%.1f TiB", bytes/(1024.0*1024.0*1024.0*1024.0));
}

void progress(int flag, double percent)
{
	percent *= 100;  
	char status[10];
	char tbwstr[20];
	char mmerrstr[20];
	char rspeedstr[20];
	char wspeedstr[20];
	if(flag) strcpy(status, "read");
	else strcpy(status,"write");
	bytestostr(totalbyteswritten,tbwstr);
	bytestostr(mismatcherrors,mmerrstr);
	bytestostr(readspeed,rspeedstr);
	bytestostr(writespeed,wspeedstr);
	printf("\rPassage = %-9i%-3.1f%% %-6s read = %9s/s write = %9s/s TBW = %-8s   I/O errors = %-18llu data errors = %-10s", 
		passage, percent ,status, rspeedstr, wspeedstr, tbwstr, ioerrors , mmerrstr);
	fflush(stdout);
}

void fill(uint64_t bytes, uint64_t filesize)
{ 
	time_t start = time(NULL);
	uint64_t bytes_time = bytes;
	int nfiles = bytes / filesize;
	bytes %= filesize;
	char filename[300];
	uint32_t buf[256];
	for(int i = 0; i < nfiles; i++) 
	{
		sprintf(filename, "%s/%i.dat",path,i);
		FILE* destfile = fopen(filename, "wb");
		for(int j = 0; j < filesize/1024;j++)
		{
			for(int k = 0; k < 256; k++)
			{
				buf[k] = xorshift128();
			}
			totalbyteswritten += 4*fwrite(buf, 4, 256, destfile);
		}
		fclose(destfile);
		progress(0, (double)i/nfiles);
	}
	/*writing remaining data*/
	sprintf(filename, "%s/%i.dat",path,nfiles);
	FILE* destfile = fopen(filename, "wb");
	for(int i = 0; i < (bytes / 1024);i++)
	{
		for(int j = 0; j < 256; j++)
		{
			buf[j] = xorshift128();
		}
		totalbyteswritten += 4*fwrite(buf, 4, 256, destfile);
	}
	fclose(destfile);
	if((time(NULL) - start) != 0) writespeed = bytes_time/(time(NULL) - start);
	progress(0, 1.0);
}

void cmpbuf(uint32_t * buf1, uint32_t * buf2)
{
	for(int i = 0; i < 256;i++)
	{
		if(buf1[i] != buf2[i]) mismatcherrors += 4;
	}
}

void readback(uint64_t bytes, uint64_t filesize)
{ 
	time_t start = time(NULL);
	uint64_t bytes_time = bytes;
	int nfiles = bytes / filesize;
	bytes %= filesize;
	char filename[300];
	uint32_t readbuf[256];
	uint32_t genbuf[256];
	for(int i = 0; i < nfiles; i++) /*writing 32MiB files*/
	{
		sprintf(filename, "%s/%i.dat",path,i);
		FILE* destfile = fopen(filename, "rb");
		for(int j = 0; j < filesize/1024;j++)
		{
			for(int k = 0; k < 256; k++)
			{
				genbuf[k] = xorshift128();
			}
			ioerrors += (4*fread(readbuf, 4, 256, destfile) - 1024);
			cmpbuf(readbuf, genbuf);
		}
		fclose(destfile);
		remove(filename);
		progress(1, (double)i/nfiles);
	}
	/*writing remaining data*/
	sprintf(filename, "%s/%i.dat",path,nfiles);
	FILE* destfile = fopen(filename, "rb");
	for(int i = 0; i < (bytes / 1024);i++)
	{
		for(int j = 0; j < 256; j++)
		{
			genbuf[j] = xorshift128();
		}
		ioerrors += (4*fread(readbuf, 4, 256, destfile) - 1024);
		cmpbuf(readbuf, genbuf);
	}
	progress(1, 1.0);
	fclose(destfile);
	remove(filename);
	if((time(NULL) - start) != 0) readspeed = bytes_time/(time(NULL) - start);
}

void savelog(void)
{
	char tbwstr[20];
	char mmerrstr[20];
	char rspeedstr[20];
	char wspeedstr[20];
	bytestostr(totalbyteswritten,tbwstr);
	bytestostr(mismatcherrors,mmerrstr);
	bytestostr(readspeed,rspeedstr);
	bytestostr(writespeed,wspeedstr);
	fprintf(logfile,"Passage = %-9i read = %9s/s write = %9s/s TBW = %-8s   I/O errors = %-18llu data errors = %-10s time = %9is \n" , 
		passage, rspeedstr, wspeedstr, tbwstr, ioerrors , mmerrstr, time(NULL) - startrun); fflush(logfile);
}

void cycle(uint64_t bytes, uint64_t filesize)
{
	for(passage = 1; !shutdown; passage++) 
	{
		reseed(passage);
		fill(bytes, filesize);
		reseed(passage);
		readback(bytes, filesize);
		savelog();	
	}
}

void sigint_handler(int s){
           printf("Caught signal %d\n",s);
           shutdown = 1;
}

int main(int argc, char ** argv)
{	
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = sigint_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);
	uint64_t bytes;
	uint64_t filesize = 32*1024*1024;
	startrun = time(NULL);
	char logname[20];
	sprintf(logname, "test%i.log", startrun);
	printf("\n\nUsage: <path> <total size of written files[kKmMgGtT]> [<size of written blocks[kKmMgGt]>]\n\n");
	if(logfile = fopen(logname, "r+"))
	{
		//fscanf(logfile, "Passage = %-9i read = %9s/s write = %9s/s TBW = %-8s   I/O errors = %-18llu data errors = %-10s time = %9is \n")
	}
	else
	{
		logfile = fopen(logname, "w");
		bytes = tobytes(argv[2]);
		if(argc == 4) filesize = tobytes(argv[3]);
		strcpy(path,argv[1]);
		totalbyteswritten = 0;
		mismatcherrors = 0;
		ioerrors = 0;
		passage = 1;
	}
	cycle(bytes, filesize);
	fclose(logfile);
	return 0;
}

