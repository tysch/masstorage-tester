#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

int cycling = 0;
struct sigaction old_action;

void sigint_handler(int s)
{
	printf("\n\nCTRL+C detected, shutting down..\n\n");
	if(cycling) cycling = 0;
	else exit(1);
}

void bytestostr(uint64_t bytes, char * str)
{
	if(bytes < 1024) sprintf(str, "%i B", (int) bytes );
	if((bytes > 1024) && (bytes < 1024*1024)) sprintf(str, "%.3f KiB", bytes/1024.0);
	if((bytes > 1024*1024) && (bytes < 1024*1024*1024)) sprintf(str, "%.3f MiB", bytes/(1024.0*1024));
	if((bytes > 1024*1024*1024) && (bytes < 1024LL*1024LL*1024LL*1024LL)) sprintf(str, "%.3f GiB", bytes/(1024.0*1024.0*1024.0));
	if((bytes > 1024LL*1024LL*1024LL*1024LL)) sprintf(str, "%.3f TiB", bytes/(1024.0*1024.0*1024.0*1024.0));
}

void printprogress(enum prflag , uint64_t val)
{
	static uint64_t totalbyteswritten;
	static uint64_t mismatcherrors;
	static uint64_t ioerrors;
	static uint32_t passage;
	static uint64_t writespeed;
	static uint64_t readspeed;
	static double percent; 
	static char status[10];
	static char tbwstr[20];
	static char mmerrstr[20];
	static char rspeedstr[20];
	static char wspeedstr[20];
	
	switch(prflag)
	{
		case rspeed:
			readspeed = val;
			bytestostr(readspeed,rspeedstr);
			break;
		case wspeed: 
			writespeed = val;
			bytestostr(writespeed,wspeedstr);
			break;
		case tbw :
			totalbyteswritten = val;
			bytestostr(totalbyteswritten,tbwstr);
			break;
		case mmerr:
			mmerrstr = val;
			bytestostr(mismatcherrors,mmerrstr);
			break;
		case perc:
			percent = val;
			percent /= 10000.0
			break;
		case readp:
			strcpy(status, "read");
			break;
		case writep:
			strcpy(status,"write");
			break;	
		case print:
			printf("\rPassage = %-9i%-3.3f%% %-9s read = %12s/s write = %12s/s TBW = %-12s   I/O errors = %-18llu data errors = %-12s", 
			                  passage, percent ,status, rspeedstr, wspeedstr, tbwstr, ioerrors , mmerrstr);
			fflush(stdout);
			break;
	}
}

uint32_t xorshift128(uint32_t seed)
{
	static uint32_t state0;
	static uint32_t state1;
	static uint32_t state2;
	static uint32_t state3;
	
	if(seed)
	{
		state0 = seed;
		state1 = 0;
		state2 = 0;
		state3 = 0;
	}
	
	uint32_t t = state3;
	t ^= t << 11;
	t ^= t >> 8;
	state3 = state2; 
	state2 = state1; 
	state1 = state0;
	t ^= state0;
	t ^= state0 >> 19;	
	state0 = t;
	return t;
}

void fillbuf(char * buf, uint32_t bufsize)
{
	uint32_t * ptr;
	for(uint32_t i = 0; i < bufsize; i += 4) 
	{
		ptr = (uint32_t *)(buf + i);
		*ptr = xorshift128(0);
	}
}

uint32_t chkbuf(char * buf, uint32_t bufsize)
{
	uint32_t * ptr;
	uint32_t nerr = 0;
	for(uint32_t i = 0; i < bufsize; i += 4) 
	{
		ptr = (uint32_t *)(buf + i);
		if((*ptr) != xorshift128(0)) nerr += 4;
	}
	return nerr;
}

uint64_t fillfile(char * path, char *buf, uint32_t bufsize)
{ 
	static uint64_t prevbyteswritten = 0;
	int64_t byteswritten = 0;
	ssize_t ret;
	int fd = open(path, O_WRONLY | O_CREAT | O_SYNC | O_TRUNC );
	if(fd == -1)
	{
		puts("\nDevice access error!\n"); 
		exit(1);
	}
	while(1)
	{
		fillbuf(buf, bufsize);
		ret = write (fd, buf, bufsize);
		if (ret == -1)
		{
			if((errno == EINVAL) && (errno == EIO) && (errno == ENOSPC)) return -1;
		}
		byteswritten += ret;
		if(ret < bufsize) 
		{
			if(!prevbyteswritten) prevbyteswritten = byteswritten;
			return byteswritten;
		}
		if(!prevbyteswritten)
		printf("\n%lliM\n",byteswritten/1048576);
	}
}

int main(int argc, char ** argv)
{	
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = sigint_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);
	
	time_t startrun;
	char logname[20];
	
	char path[256];
	char scmd[256];
	uint32_t bufsize = 1024*1024;
	uint32_t passage = 0;
	uint64_t totalbyteswritten = 0;
	char * buf = malloc(sizeof * buf * bufsize);	
	
	if(argc < 6) 
	{
		printf("\n\nUsage: -d <path> [-o|r|w|c] [-i <salt>]");
		printf("\n -d -- path to test device");
		printf("\n -o -- single read/write cycle, for speed and volume measurement, default)");
		printf("\n -c -- continuous read/write cycling, for endurance tests");
		printf("\n -w -- single write only");
		printf("\n -r -- single read only");
		printf("\n -w, -r are useful for long term data retention tests");
		printf("\n           -w and -r must be launched with the same salt");
		printf("\n -i -- integer salt for random data being written, default 1\n");
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
	
	puts("\nWARNING! All data on the device would be lost!\n");
	strcpy(scmd, "fdisk -l | grep ");
	strcat(scmd, path);
	system(scmd);
	printf("\nIs the device correct? y/n\n");
	if(getchar() != 'y') exit(1);	
	startrun = time(NULL);
	sprintf(logname, "test%i.log", startrun);
	FILE *logfile = fopen(logname, "w");
	for(int i = 0; i < argc; i++) fprintf(logfile,"%s ",argv[i]);
	fprintf(logfile,"\n\n");
	
	do
	{
		passage++;
		xorshift128(passage);
		totalbyteswritten += fillfile(path, buf, bufsize);
		xorshift128(passage);
		/*readback(bytes, filesize, path);*/
	} while(cycling);
	
	free(buf);
	return 0;
}
