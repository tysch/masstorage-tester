#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

uint64_t totalbyteswritten = 0;
uint64_t mismatcherrors = 0;
uint64_t ioerrors = 0;
uint32_t passage = 1;
uint64_t writespeed;
uint64_t readspeed;

volatile int shutdown = 0;

struct sigaction old_action;

void filefailure(void)
{
	printf("\n\nfatal I/O error, terminating...\n\n");
	exit(1);
}

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

void fill(uint64_t bytes, uint64_t filesize, char * path)
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
		if(destfile == NULL) filefailure();
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
	sprintf(filename, "%s/%i.dat",path,nfiles);
	FILE* destfile = fopen(filename, "wb");
	if(destfile == NULL) filefailure();
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

void readback(uint64_t bytes, uint64_t filesize , char * path)
{ 
	time_t start = time(NULL);
	uint64_t bytes_time = bytes;
	int nfiles = bytes / filesize;
	bytes %= filesize;
	char filename[300];
	uint32_t readbuf[256];
	uint32_t genbuf[256];
	for(int i = 0; i < nfiles; i++)
	{
		sprintf(filename, "%s/%i.dat",path,i);
		FILE* destfile = fopen(filename, "rb");
		if(destfile == NULL) filefailure();
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
	sprintf(filename, "%s/%i.dat",path,nfiles);
	FILE* destfile = fopen(filename, "rb");
	if(destfile == NULL) filefailure();
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

void savelog(time_t startrun, FILE * logfile)
{
	char tbwstr[20];
	char mmerrstr[20];
	char rspeedstr[20];
	char wspeedstr[20];
	bytestostr(totalbyteswritten,tbwstr);
	bytestostr(mismatcherrors,mmerrstr);
	bytestostr(readspeed,rspeedstr);
	bytestostr(writespeed,wspeedstr);
	fprintf(logfile,"Passage = %-9i read = %9s/s write = %9s/s TBW = %-8s   I/O errors = %-18llu data errors = %-10s time = %llis \n" , 
		passage, rspeedstr, wspeedstr, tbwstr, ioerrors , mmerrstr, time(NULL) - startrun); fflush(logfile);
}

void cycle(uint64_t bytes, uint64_t filesize, time_t startrun, char * path, FILE * logfile, int singlerun)
{
	for(; !shutdown; passage++) 
	{
		reseed(passage);
		fill(bytes, filesize, path);
		reseed(passage);
		readback(bytes, filesize, path);
		savelog(startrun, logfile);	
		if(singlerun) break;
	}
}

void sigint_handler(int s){
           printf("\n\nCTRL+C detected, shutting down..\n\n");
           shutdown = 1;
}

int main(int argc, char ** argv)
{	
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = sigint_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);
	char path[1024];
	uint64_t bytes;
	uint64_t filesize = 32*1024*1024;
	time_t startrun = time(NULL);
	char logname[20];
	int singlerun = 0;
	int singleread = 0;
	int singlewrite = 0;
	int cycling = 0;
	if(argc < 3) 
	{
		printf("\n\nUsage: -d <path> -s <size[kKmMgGtT]> [-b <size[kKmMgGt]>]  [-o|r|w|c] [-i <salt>]");
		printf("\n -d -- path to test destination");
		printf("\n -s -- total file size");
		printf("\n -b -- size of files, rounded to 1kiB");
		printf("\n -o -- single read/write cycle, for speed measurement)");
		printf("\n -c -- continuous read/write cycling, for endurance tests");
		printf("\n -w -- single write only");
		printf("\n -r -- single read only");
		printf("\n -w, -r are useful for long term data retention tests");
		printf("\n           -w and -r must be launched with the same");
		printf("\n -i -- integer salt for random data being written, default 1");
		exit(1);
	}
	for(int i = 0; i < argc; i++)
	{	
		if(strcmp(argv[i],"-d") == 0) 
		{
			if(i + 1 == argc) 
				exit(1);
			else 
				strcpy(path,argv[i + 1]);
		}
		if(strcmp(argv[i],"-s") == 0) 
		{
			if((i + 1 == argc)) 
				exit(1);
			else 
				bytes = tobytes(argv[i + 1]);
		}
		if(strcmp(argv[i],"-b") == 0) 
		{
			if((i + 1 == argc)) 
				exit(1);
			else 
				filesize = (uint64_t)(1024*(tobytes(argv[i + 1])/1024.0));
		}
		if(strcmp(argv[i],"-i") == 0) 
		{
			if((i + 1 == argc)) 
				exit(1);
			else 
				sscanf(argv[i + 1], "%i", &passage);
		}
		if(strcmp(argv[i],"-o") == 0) 
			singlerun = 1;
		if(strcmp(argv[i],"-c") == 0) 
			cycling = 1;
		if(strcmp(argv[i],"-w") == 0) 
			singlewrite = 1;
		if(strcmp(argv[i],"-r") == 0) 
			singleread = 1;
	}
	sprintf(logname, "test%i.log", startrun);
	FILE *logfile = fopen(logname, "w");
	for(int i = 0; i < argc; i++) fprintf(logfile,"%s ",argv[i]);
	fprintf(logfile,"\n\n");
	if(singlewrite) 	
	{
		reseed(passage);
		fill(bytes, filesize, path);
		savelog(startrun, logfile);
	}
	if(singleread) 	
	{
		reseed(passage);
		readback(bytes, filesize, path);
		savelog(startrun, logfile);
	}
	if(cycling || singlerun) cycle(bytes, filesize, startrun, path, logfile, singlerun);
	printf("\n");
	fclose(logfile);
	return 0;
}
