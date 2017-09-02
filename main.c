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


struct fecblock
{
	uint64_t errcnt;
	uint64_t errcntmax;
	uint64_t blocksize;
};

int nblocksizes;

struct fecblock * fectest_init(uint64_t byteswritten, uint64_t *pos , int *nblocksizes)
{
	uint64_t bsize = 512;
	struct fecblock * fecblocks;
	*pos = 0;
	for(uint64_t i = bsize; i < byteswritten; i *= 2LL) (*nblocksizes)++;
	fecblocks = malloc(sizeof(struct fecblock) * (*nblocksizes));
	for(int i = 0; i < (*nblocksizes); i++)
	{
		fecblocks[i].blocksize = bsize;
		fecblocks[i].errcnt = 0;
		fecblocks[i].errcntmax = 0;
		bsize *= 2;
	}
	return fecblocks;
}

void fecsize_test(struct fecblock * fecblocks, int nerror, uint64_t *pos, int nblocksizes)
{
	*pos += 4;

	for(int i = 0; i < nblocksizes; i++)
	{
		fecblocks[i].errcnt += nerror;

		if(fecblocks[i].errcntmax < fecblocks[i].errcnt)
			fecblocks[i].errcntmax = fecblocks[i].errcnt;

		if((*pos % fecblocks[i].blocksize) == 0)
			fecblocks[i].errcnt = 0;
	}
}

void readseedandsize_fectest (char * buf, uint32_t bufsize, uint32_t *seed, uint64_t *size, struct fecblock * fecblocks, uint64_t *pos, int nblocksizes)
{
	uint64_t * ptr = (uint64_t *) buf;
	for(uint32_t i = 0; i < bufsize; i += 16)
	{
		if((uint32_t)(*ptr) != *seed)
		{
			fecsize_test(fecblocks, 4, pos, nblocksizes);
			fecsize_test(fecblocks, 4, pos, nblocksizes);
		}
		else
		{
			fecsize_test(fecblocks, 0, pos, nblocksizes);
			fecsize_test(fecblocks, 0, pos, nblocksizes);
		}

		ptr++;

		if(*ptr != *size)
		{
			fecsize_test(fecblocks, 4, pos, nblocksizes);
			fecsize_test(fecblocks, 4, pos, nblocksizes);
		}
		else
		{
			fecsize_test(fecblocks, 0, pos, nblocksizes);
			fecsize_test(fecblocks, 0, pos, nblocksizes);
		}
		ptr++;
	}
}


void print_fec_summary(struct fecblock * fecblocks, int nblocksizes, FILE * logfile, int islogging)
{
	uint64_t nblocksize;
	uint64_t nerr;
	uint64_t sparebytes;
	double overhead;
	char bsstr[20];
	char sbstr[20];
	printf("\n Forward error correction code requirements:\n");


	for(int i = 0; i < nblocksizes; i++)
	{
		nerr = fecblocks[i].errcntmax;
		nblocksize = fecblocks[i].blocksize;
		bytestostr(nblocksize, bsstr);
		if(nerr > nblocksize / 2)
		{
			printf("block size: %-12s -- insufficient block size\n", bsstr);
			if(islogging)
				fprintf(logfile, "block size: %-12s -- insufficient block size\n", bsstr);
		}
		else
		{
			sparebytes = 2 * nerr;
			bytestostr(sparebytes, sbstr);
			overhead = 100.0 * (double) sparebytes / nblocksize;
			printf("block size: %-12s  spare bytes: %-12s   overhead: %.2f%%\n", bsstr, sbstr, overhead);
			if(islogging)
				fprintf(logfile, "block size: %-12s  spare bytes: %-12s   overhead: %.2f%%\n", bsstr, sbstr, overhead);
		}
	}
}

// Pseudorandom number generator state variables
uint32_t state0;
uint32_t state1;
uint32_t state2;
uint32_t state3;

// Ctrl+C interrupt handler

int stop_cycling = 0;
int stop_all = 0;

struct sigaction old_action;

void sigint_handler(int s)
{
	if(stop_cycling && stop_all)
	{
		printf("\n\nShutting down..\n\n");
		fflush(stdout);
		exit(1);
	}
	if(stop_cycling && (!stop_all))
	{
		stop_all = 1;
		printf("\nAborting current r/w operation and closing device...\n");
		fflush(stdout);
	}
	if(!stop_cycling)
	{
		stop_cycling = 1;
		printf("\nRepeated read-write cycling stopped\n");
		fflush(stdout);
	}
}


// Program mode selection
enum prmode
{
	singlecycle,
	multicycle,
	singleread,
	singlewrite
};

// Logging routine argument selection
enum printmode
{
	count,
	size,
	rspeed,
	wspeed,
	tbw,
	ioerror,
	mmerr,
	perc,
	readb,
	writeb,
	readp,
	writep,
	print,
	log,
	reset
};

// Converts bytes count to human readable string
void bytestostr(uint64_t bytes, char * str)
{
	if(bytes < 1024)
		sprintf(str, "%i B", (int) bytes );
	if((bytes >= 1024) && (bytes < 1024*1024))
		sprintf(str, "%.3f KiB", bytes/1024.0);
	if((bytes >= 1024*1024) && (bytes < 1024*1024*1024))
		sprintf(str, "%.3f MiB", bytes/(1024.0*1024));
	if((bytes >= 1024*1024*1024) && (bytes < 1024LL*1024LL*1024LL*1024LL))
		sprintf(str, "%.3f GiB", bytes/(1024.0*1024.0*1024.0));
	if((bytes >= 1024LL*1024LL*1024LL*1024LL))
		sprintf(str, "%.3f TiB", bytes/(1024.0*1024.0*1024.0*1024.0));
}

// Prints and logs status
// Replace scattered prints with a lots of global variables
void printprogress(enum printmode prflag , uint64_t val, FILE * logfile)
{
	static char status;
	static uint64_t totalbyteswritten;
	static uint64_t mismatcherrors;
	static uint64_t ioerrors;
	static uint32_t passage;
	static uint64_t writespeed;
	static uint64_t readspeed;
	static uint64_t readbytes;
	static uint64_t writebytes;
	static uint64_t sizewritten;
	static time_t startrun;
	static time_t elapsed;
	static double percent;

	static char tbwstr[20];
	static char readstr[20];
	static char writestr[20];
	static char ioerrstr[20];
	static char mmerrstr[20];
	static char rspeedstr[20];
	static char wspeedstr[20];
	static char sizestr[20];

	const int readrun = 1;
	const int writerun = 2;

	switch(prflag)
	{
		case count:
			passage++;
			break;
		case size:
			sizewritten = val;
			bytestostr(sizewritten, sizestr);
			printf("\nactual device size is %s\n", sizestr);
			break;
		case rspeed:
			readspeed = val;
			bytestostr(readspeed,rspeedstr);
			break;
		case wspeed:
			writespeed = val;
			bytestostr(writespeed,wspeedstr);
			break;
		case tbw :
			totalbyteswritten += val;
			bytestostr(totalbyteswritten,tbwstr);
			break;
		case ioerror :
			totalbyteswritten += val;
			bytestostr(totalbyteswritten,tbwstr);
			break;
		case mmerr:
			mismatcherrors += val;
			bytestostr(mismatcherrors,mmerrstr);
			break;
		case perc:
			percent = val;
			percent /= 10000.0;
			break;
		case readb:
			readbytes = val;
			bytestostr(readbytes,readstr);
			break;
		case writeb:
			writebytes = val;
			bytestostr(writebytes, writestr);
			break;
		case readp:
			status = readrun;
			break;
		case writep:
			status = writerun;
			break;
		case reset:
			passage = 1;
			sizewritten = 0;
			bytestostr(sizewritten, sizestr);
			readspeed = 0;
			bytestostr(readspeed,rspeedstr);
			writespeed = 0;
			bytestostr(writespeed,wspeedstr);
			totalbyteswritten = 0;
			bytestostr(totalbyteswritten,tbwstr);
			readbytes = 0;
			bytestostr(readbytes,readstr);
			writebytes = 0;
			bytestostr(writebytes, writestr);
			ioerrors = 0;
			bytestostr(ioerrors, ioerrstr);
			mismatcherrors = 0;
			bytestostr(mismatcherrors,mmerrstr);
			percent = 0;
			startrun = time(NULL);
			break;
		case log:
			if(status == writerun)
			{
				fprintf(logfile,
				"\nPassage = %-9i%-3.3f%% write = %-12s, %12s/s    TBW = %-12s   I/O errors = %-12s  data errors = %-12s time = %llis",
				(int) passage,
				percent, writestr, wspeedstr, tbwstr, ioerrstr , mmerrstr,
				(long long int) (time(NULL) - startrun));
			}
			if(status == readrun)
			{
				fprintf(logfile,
				"\nPassage = %-9i%-3.3f%% read = %-12s, %12s/s    TBW = %-12s   I/O errors = %-12s  data errors = %-12s time = %llis",
				(int) passage,
				percent, readstr, rspeedstr, tbwstr, ioerrstr , mmerrstr,
				(long long int) (time(NULL) - startrun));
			}
			/*Intentionally left no break statement*/
		case print:
			if(status == writerun)
			{
				printf("                    ");
				printf(
				"\rPassage = %-9i%-3.3f%% write = %-12s, %12s/s    TBW = %-12s   I/O errors = %-12s  data errors = %-12s time = %llis",
				(int) passage,
				percent, writestr, wspeedstr, tbwstr, ioerrstr , mmerrstr,
				(long long int) (time(NULL) - startrun));
			}
			if(status == readrun)
			{
				printf("                    ");
				printf(
				"\rPassage = %-9i%-3.3f%% read = %-12s, %12s/s    TBW = %-12s   I/O errors = %-12s  data errors = %-12s time = %llis",
				(int) passage,
				percent, readstr, rspeedstr, tbwstr, ioerrstr , mmerrstr,
				(long long int) (time(NULL) - startrun));
			}
			fflush(stdout);
			break;
	}
}

void reseed(uint32_t seed)
{
	state0 = seed;
	state1 = 0;
	state2 = 0;
	state3 = 0;
}

// Extremely fast random number generator with 2^128 long cycle
uint32_t xorshift128(void)
{
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

// Fills buffer with a random data
void fillbuf(char * buf, uint32_t bufsize)
{
	uint32_t * ptr;
	for(uint32_t i = 0; i < bufsize; i += 4)
	{
		ptr = (uint32_t *)(buf + i);
		*ptr = xorshift128();
	}
}

// Compares buffer data with a generated random values and counts read mismatches
uint32_t chkbuf(char * buf, uint32_t bufsize, struct fecblock * fecblocks, uint64_t *fpos, int nblocksizes, int isfectesting)
{
	uint32_t * ptr;
	uint32_t nerr = 0;
	uint32_t err_curr;
	for(uint32_t i = 0; i < bufsize; i += 4)
	{
		ptr = (uint32_t *)(buf + i);
		err_curr = ((*ptr) - xorshift128());
		if(err_curr != 0) nerr += 4;
		if(isfectesting)
		{
			if((err_curr != 0))
				fecsize_test(fecblocks, 4, fpos, nblocksizes);
			else
				fecsize_test(fecblocks, 0, fpos, nblocksizes);
		}
	}
	return nerr;
}

// Embeds seed and size information in buffer with (bufsize/16)-modular redundancy
void writeseedandsize (char * buf, uint32_t bufsize, uint32_t seed, uint64_t size)
{
	// |64-bit seed | 64-bit size |64-bit seed | 64-bit size |...
	uint64_t * ptr;
	for(uint32_t i = 0; i < bufsize; i += 2*sizeof(uint64_t))
	{
		ptr = (uint64_t *)(buf + i);
		*ptr = (uint64_t) seed;
		ptr = (uint64_t *)(buf + i + sizeof(uint64_t));
		*ptr = size;
	}
}

// Recovers seed and size information from buffer
int32_t readseedandsize (char * buf, uint32_t bufsize, uint32_t *seed, uint64_t *size)
{
	// |64-bit seed | 64-bit size |64-bit seed | 64-bit size |...
	uint32_t bitcount[128] = {0};
	char rcblock[16] = {0};
	uint64_t * ptr;
	int32_t errcount = 0;
	// bit frequency counting
	for(uint32_t i = 0; i < bufsize; i += 16) // per-128-bit block iteration
	{
		for(uint32_t j = 0; j < 2*sizeof(uint64_t); j++) // per-byte iteration
		{
			for(uint32_t k = 0; k < 8; k++) // per-bit iteration
			{
				if((buf[i + j] & (1 << k)))
					bitcount[8*j + k]++;
			}
		}
	}
	// reconstruction of 128-bit block value
	for(uint32_t i = 0; i < 2*sizeof(uint64_t); i++) // per-byte iteration
	{
		for(uint32_t j = 0; j < 8; j++) // per-bit iteration
		{

			if(bitcount[8*i + j] > (bufsize / (16*2)))  // set bit as the majority are
				rcblock[i] |= 1 << j;
			if(bitcount[8*i + j] == (bufsize / (16*2))) // unrecoverable data condition
				return -1;
			/*
			if(rcblock[i] & (1 << j)) // majority are 1es
				errcount += (bufsize - bitcount[8*i + j]); // 0es are errors
			if(!(rcblock[i] & (1 << j))) // majority is 0
				errcount += bitcount[8*i + j]; // 1es are errors*/
		}
	}
	// stripping 128-bit block for seed and size values
	ptr = (uint64_t *)rcblock;
	*seed = (uint32_t)*ptr;
	ptr = (uint64_t *)(rcblock + sizeof(uint64_t));
	*size = *ptr;
	// counting read mismatches
	ptr = (uint64_t *)buf;
	for(uint32_t i = 0; i < bufsize; i += 16)
	{
		if((uint32_t)(*ptr) != *seed)  errcount += 8;
		ptr++;
		if(*ptr != *size) errcount += 8;
		ptr++;
	}
	return errcount;
}

//

uint64_t fillfile(char * path, char *buf, uint32_t seed, uint32_t bufsize, FILE * logfile , int islogging)
{
	char sizestr[20];
	static uint64_t prevbyteswritten = 0;
	uint64_t byteswritten = 0;
	ssize_t ret;
	int firstcycle = 1;
	time_t startrun = time(NULL);
	int fd = open(path, O_WRONLY | O_SYNC | O_TRUNC);

	if(fd == -1)
	{
		printf("\nDevice access error!\n");
		if(islogging) fprintf(logfile, "\nDevice access error!\n");
		exit(1);
	}

	while(1)
	{
		if(stop_all) break;
		//fillbuf(buf, bufsize);
		if(firstcycle)
		{
			firstcycle = 0;
			if(lseek(fd, (off_t) bufsize, SEEK_SET) != bufsize )
						{
							printf("\ndevice seek failure!\n");
							if(islogging) fprintf(logfile, "\ndevice seek failure!");
						}
			continue;
		}

		fillbuf(buf, bufsize);

		ret = write (fd, buf, bufsize);
		if (ret == -1)
		{
			if((errno == EINVAL) && (errno == EIO) && (errno == ENOSPC))
			{
				printf("\ndevice write failure\n");
				if(islogging) fprintf(logfile, "\ndevice write failure");
				break;
			}
		}

		byteswritten += ret;
		/* Progress print section */
		if(prevbyteswritten)
			printprogress(perc, (uint64_t)(1000000.0*((double)byteswritten / prevbyteswritten)), logfile);
		printprogress(writeb, byteswritten, logfile);
		printprogress(tbw, ret, logfile);
		if(time(NULL) - startrun)
			printprogress(wspeed, byteswritten / (time(NULL) - startrun), logfile);
		printprogress(print, 0, logfile);
		if(ret < bufsize) break;
//if(byteswritten > 50*1000*1000) break; // simulating smaller size
	}


	writeseedandsize(buf, bufsize, seed, byteswritten);
	pwrite (fd, buf, bufsize, 0);

	if(close(fd) == -1)
	{
		printf("\ndevice is not closed properly!\n");
		if(islogging) fprintf(logfile, "\ndevice is not closed properly!");
	}
	if(islogging) printprogress(log, 0, logfile);
	if(!prevbyteswritten)
	{
		prevbyteswritten = byteswritten;
		printprogress(size, byteswritten, logfile);
	}
	return byteswritten;
}

/* partial read tolerant code */

uint32_t fileread(int fd, char * buf, uint32_t bufsize , FILE * logfile, int islogging)
{
	ssize_t ret;
	uint32_t bufread = 0;

	if(fd == -1)
	{
		printf("\nDevice access error!\n");
		if(islogging) fprintf(logfile, "\nDevice access error!\n");
		exit(1);
	}
	while (bufsize != 0 && (ret = read (fd, buf, bufsize)) != 0)
	{
		if(stop_all) break;
		if (ret == -1)
		{
			if (errno == EINTR)
				continue;
			printf("\nDevice read error!\n");
			if(islogging) fprintf(logfile, "\nDevice read error!\n");
			break;
		}
		bufsize -= ret;
		buf += ret;
		bufread += ret;
	}
	return bufread;
}



void readback(char * path, char *buf, uint32_t bufsize, FILE * logfile, uint64_t byteswritten , int islogging, int isfectesting)
{
	uint64_t bytesread = 0;
	ssize_t bufread;
	uint32_t seed;
	time_t startrun = time(NULL);
	int firstcycle = 1;
	uint32_t errcnt = 0;
	char * tmpptr;
	char rdstr[20];

	struct fecblock * fecblocks;
	int nblocksizes = 0;
	uint64_t fpos = 0;

	uint32_t nerr;


	int fd = open(path, O_RDONLY);

	if(fd == -1)
	{
		printf("\nDevice access error!\n");
		if(islogging) fprintf(logfile, "\nDevice access error!\n");
		exit(1);
	}

	while (!stop_all)
	{
		bufread = fileread(fd, buf, bufsize , logfile, islogging);
		bytesread += bufread;
		/* Progress print section */
		printprogress(readb, bytesread, logfile);

		if(firstcycle)
		{
			firstcycle = 0;
			errcnt = readseedandsize(buf, bufread, &seed, &byteswritten);
			printprogress(mmerr, errcnt, logfile);
			printf("\n\n seed = %i size = %lli from readback\n\n", (int) seed, (long long int) byteswritten);
			reseed(seed);
			//chkbuf(buf, bufsize); //

			if(isfectesting)
			{
				fecblocks = fectest_init(byteswritten, &fpos , &nblocksizes);
				readseedandsize_fectest(buf, bufsize, &seed, &byteswritten, fecblocks, &fpos, nblocksizes);
			}
			continue;
		}
		nerr = chkbuf(buf, bufread, fecblocks, &fpos, nblocksizes, isfectesting);
		printprogress(mmerr, nerr, logfile);

		if(byteswritten)
			printprogress(perc, (uint64_t)(1000000.0*((double)bytesread / byteswritten)), logfile);
		if(time(NULL) - startrun)
			printprogress(rspeed, bytesread / (time(NULL) - startrun), logfile);
		printprogress(print, 0, logfile);

		if (bufread < bufsize) break;
	}

	bytestostr(bytesread, rdstr);
	printf("\nRead back %s of data\n", rdstr);
	if(islogging) fprintf(logfile, "\nRead back %s of data\n", rdstr);

	if(close(fd) == -1)
	{
		printf("\ndevice is not closed properly!\n");
		if(islogging) fprintf(logfile, "\ndevice is not closed properly!");
	}

	if(islogging) printprogress(log, 0, logfile);

	if(isfectesting) print_fec_summary(fecblocks, nblocksizes, logfile, islogging);
	if(isfectesting) free(fecblocks);

}

void print_usage(int arg)
{
	if(arg < 3)
	{
		printf("\n\nUsage: -d <path> [-o|r|w|c] [-i <salt>] [-l]");
		printf("\n -d -- path to test device");
		printf("\n -o -- single read/write cycle, for speed and volume measurement, default)");
		printf("\n -c -- continuous read/write cycling, for endurance tests");
		printf("\n -w -- single write only");
		printf("\n -r -- single read only");
		printf("\n -w, -r are useful for long term data retention tests");
		printf("\n           -w and -r must be launched with the same salt");
		printf("\n -i -- integer salt for random data being written, default 1\n");
		printf("\n -l -- write a log file\n");
		printf("\n -f -- estimates forward error correction techniques requirements ");
		printf("(i.e. Reed-Solomon) to handle errors being countered\n");
		exit(1);
	}
	if(getuid())
	{
		printf("\n Writing to a raw device requires root privileges\n");
		exit(1);
	}
}

void parse_cmd_val(int argc, char ** argv, char * path, uint32_t * seed , int *islogging, int *isfectesting)
{
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
				sscanf(argv[i + 1], "%i", seed);
		}
		if(strcmp(argv[i],"-l") == 0)
		{
			*islogging = 1;
		}
		if(strcmp(argv[i],"-f") == 0)
		{
			*isfectesting = 1;
		}
	}
}

enum prmode parse_cmd_mode(int argc, char ** argv)
{
	enum prmode ret = singlecycle;
	for(int i = 0; i < argc; i++)
	{
		if(strcmp(argv[i],"-o") == 0)
		{
			ret = singlecycle;
			break;
		}
		if(strcmp(argv[i],"-c") == 0)
		{
			ret = multicycle;
			break;
		}
		if(strcmp(argv[i],"-w") == 0)
		{
			ret = singlewrite;
			break;
		}
		if(strcmp(argv[i],"-r") == 0)
		{
			ret = singleread;
			break;
		}
	}
	return ret;
}

void print_erasure_warning(char * path)
{
	char scmd[256];
	printf("\nWARNING! All data on the device would be lost!\n");
	strcpy(scmd, "fdisk -l | grep ");
	strcat(scmd, path);
	system(scmd);
	printf("\nIs the device correct? y/n\n");
	if(getchar() != 'y') exit(1);
}

void log_init(int argc, char ** argv, FILE **logfileptr)
{
	char logname[25];
	struct tm * timeinfo;
	time_t startrun = time(NULL);
	time_t rawtime;
	sprintf(logname, "test-%lli.log", (long long int) startrun);
	*logfileptr = fopen(logname, "w+");
	if(*logfileptr == NULL)
	{
		printf("\nCannot create logfile!\n");
		exit(1);
	}
	for(int i = 0; i < argc; i++)
		fprintf(*logfileptr ,"%s ",argv[i]);
	fprintf(*logfileptr ,"\n\n");
	time ( &rawtime );
	timeinfo = localtime(&rawtime);
	fprintf (*logfileptr , "Started at: %s\n", asctime (timeinfo));
}

void singlewrite_f(char * path, char * buf, uint32_t bufsize, uint32_t seed, FILE * logfile , int islogging)
{
	reseed(seed);
	printprogress(reset, 0, logfile);
	printprogress(writep, 0, logfile);
	fillfile(path, buf, seed, bufsize, logfile , islogging);
}

void singleread_f(char * path, char * buf, uint32_t bufsize, uint32_t seed, FILE * logfile , int islogging, int isfectesting)
{
	//reseed(seed);
	printprogress(reset, 0, logfile);
	printprogress(readp, 0, logfile);

	readback(path, buf, bufsize, logfile, 0 , islogging, isfectesting);
}


void cycle_f(char * path, char * buf, uint32_t bufsize, uint32_t seed, FILE * logfile , int islogging, int isfectesting)
{
	uint64_t byteswritten;
	printprogress(reset, 0, logfile);
	do
	{
		reseed(seed);
		printprogress(writep, 0, logfile);
		byteswritten = fillfile(path, buf, seed, bufsize, logfile , islogging);
		if(stop_all) break;
		//reseed(seed);
		printprogress(readp, 0, logfile);
		readback(path, buf, bufsize, logfile, byteswritten , islogging, isfectesting);
		printprogress(count, 0, logfile);
		if(stop_all) break;
		seed++;
	} while (!stop_cycling);
}

int main(int argc, char ** argv)
{
	struct sigaction siginthandler;
	siginthandler.sa_handler = sigint_handler;
	sigemptyset(&siginthandler.sa_mask);
	siginthandler.sa_flags = 0;
	sigaction(SIGINT, &siginthandler, NULL);

	enum prmode mod;
	FILE * logfile;
	char path[256];

	uint32_t bufsize = 1024*1024; /* must be multiplier of 16 */
	uint32_t seed = 1;
	char * buf = malloc(sizeof * buf * bufsize);
	int islogging = 0;
	int isfectesting = 0;

	print_usage(argc);
	parse_cmd_val(argc, argv, path, &seed, &islogging, &isfectesting);
	mod = parse_cmd_mode(argc, argv);
	print_erasure_warning(path);
	if(islogging) log_init(argc, argv, &logfile);

	switch(mod)
	{
		case singleread:
			singleread_f(path, buf, bufsize, seed, logfile , islogging, isfectesting);
			break;
		case singlewrite:
			singlewrite_f(path, buf, bufsize, seed, logfile, islogging);
			break;
		case singlecycle:
			stop_cycling = 1;
			cycle_f(path, buf, bufsize, seed, logfile, islogging, isfectesting);
			break;
		case multicycle:
			cycle_f(path, buf, bufsize, seed, logfile, islogging, isfectesting);
			break;
	}
	if(islogging) fclose(logfile);
	free(buf);
	return 0;
}

