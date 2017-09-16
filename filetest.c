/*
 * filetest.c
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <time.h>
#include "printprogress.h"
#include "strconv.h"
#include "constants.h"
#include "filewrite.h"
#include "fileread.h"
#include "optrw.h"
#include "devread.h"
#include "devwrite.h"
#include "rng.h"

extern int stop_cycling;

void fillfiles(char * path, char * buf, uint32_t seed, FILE * logfile , int islogging , uint64_t totsize, uint32_t bufsize)
{
	uint64_t pow = 1;
	uint64_t nfiles = totsize / bufsize;
	time_t startrun = time(NULL);
	uint64_t byteswritten = 0;

	for(uint64_t i = 0; i < nfiles; i++)
	{
		if(stop_cycling) break;

		if(i == (pow - 1)) //Use 2^n-th file to store seed and size information
		{
			writeseedandsize(buf, seed, totsize, bufsize);
			pow <<= 1;
		}
		else fillbuf(buf, bufsize);
		writefile(i, path, buf, logfile, islogging, bufsize);

		byteswritten += bufsize;

        printprogress(writeb, byteswritten, logfile);
        printprogress(tbw, bufsize, logfile);
        if(time(NULL) - startrun)
            printprogress(wspeed, byteswritten / (time(NULL) - startrun), logfile);
        printprogress(print, 0, logfile);
        printprogress(perc, (uint64_t)(1000000.0*((double)(i + 1) / nfiles)), logfile);
	}
    if(islogging) printprogress(log, 0, logfile);
}

void readfiles(char * path, char * buf, FILE * logfile, int islogging)
{
	uint64_t pow = 1;
	uint64_t nfiles;
	uint64_t totsize;
	uint32_t bufsize;
	uint32_t seed;

	uint32_t dummyseed; // variables to fit  readseedandsize() for file errors only
	uint64_t dummysize;

	uint32_t currerr = 0;
	uint64_t toterr  = 0;
	uint64_t bytesread = 0;

    time_t startrun = time(NULL);
	char errsize[24];

	int32_t res = -1;

	// Attempts to recover file size from MAX_CHECKED_FILES,
	bufsize = bufsize_retrieve(path, logfile, islogging);

	do //Read 2^n-th files until seed and size information can be retrieved
	{
		if(stop_cycling) break;
		if(pow > MAX_FILE_COUNT) break;

		readfile((pow - 1), path, buf, logfile, islogging, bufsize);

		pow <<= 1;

		res = readseedandsize(buf, bufsize, &seed, &totsize);

        if(time(NULL) - startrun)
            printprogress(rspeed, bytesread / (time(NULL) - startrun), logfile);

        printprogress(print, 0, logfile);
	}
	while (res == -1);

	if(res == -1)
	{
		printf("\nseed and size information is unrecoverable\n");
		exit(1);
	}

	nfiles = totsize / bufsize;
	reseed(seed);

	pow = 1;

	for(uint64_t i = 0; i < nfiles; i++)
	{
		if(stop_cycling) break;

		bytesread += readfile(i, path, buf, logfile, islogging, bufsize);

		if(i == (pow - 1)) //Skip each 2^n-th file
		{
			res += readseedandsize(buf, bufsize, &dummyseed, &dummysize);
			if(res >= 0)
				currerr += res;
			else
				currerr += bufsize;
			pow <<= 1;
			printprogress(readb, bytesread, logfile);
	        printprogress(perc, (uint64_t)(1000000.0*((double)(i + 1) / nfiles)), logfile);
			printprogress(mmerr, currerr, logfile);
		    if(time(NULL) - startrun)
		        printprogress(rspeed, bytesread / (time(NULL) - startrun), logfile);
			printprogress(print, 0, logfile);
			continue;
		}
		else
		{
			currerr = chkbuf_file(buf, bufsize);
			toterr += currerr;
			if(currerr)
			{
				bytestostr(currerr, errsize);
				printf("\n--------------------file %-15lli is damaged, %-9s errors\n", i, errsize);
				if(islogging) fprintf(logfile, "\n--------------------file %-15lli is damaged, %-9s errors\n", i, errsize);
			}
			printprogress(readb, bytesread, logfile);
	        printprogress(perc, (uint64_t)(1000000.0*((double)(i + 1) / nfiles)), logfile);
			printprogress(mmerr, currerr, logfile);
		    if(time(NULL) - startrun)
		        printprogress(rspeed, bytesread / (time(NULL) - startrun), logfile);
			printprogress(print, 0, logfile);
		}
	}
	if(islogging) printprogress(log, 0, logfile);
}


