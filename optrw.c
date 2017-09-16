/*
 * optrw.c
 *
 */

#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "constants.h"
#include "errmesg.h"
#include "rng.h"

extern int stop_all;

// Sorting routines for uint32[];
int cmpuint32_t (const void * elem1, const void * elem2)
{
    int f = *((uint32_t*)elem1);
    int s = *((uint32_t*)elem2);
    if (f > s) return  1;
    if (f < s) return -1;
    return 0;
}

void qsortuint32_t(uint32_t * arr, int size)
{
	qsort(arr, size, sizeof(*arr), cmpuint32_t);
}

// Returns most frequent result or 0 if there is more than one most occurent result

uint32_t major_occurence (uint32_t * data, int n)
{
	uint32_t curres = 0;
	uint32_t currlength = 0;
	uint32_t mstfreqres = 0;
	uint32_t maxlength = 0;
	uint32_t prevlength = 0;

	// flags that there is more than one most frequent value
	int eqflag = 0;

	// sort all results
	qsortuint32_t(data, n);
	// finds longest
	for(int i = 0; i < n; i++)
	{
		if(data[i] != curres)
		{
			curres = data[i];
			prevlength = currlength;
			currlength = 0;
		}
		if(currlength == prevlength)
		{
			eqflag = 1;
		}
		if(currlength > maxlength)
		{
			eqflag = 0;
			maxlength = currlength;
			mstfreqres = curres;
		}
		currlength++;
	}
	if(eqflag) return 0;
	else return mstfreqres;
}

// Attempts to recover file size from MAX_CHECKED_FILES,
// Error-tolerant
uint32_t bufsize_retrieve(char * path, FILE * logfile, int islogging)
{
	char filename[PATH_MAX];
    uint32_t sizelist[MAX_CHECKED_FILES];
    int fd;
    int32_t ret;
    uint32_t bufsize;
    uint32_t nfile = 0;

	for(; nfile < MAX_CHECKED_FILES; nfile++)
	{
		if(stop_all) break;
		sprintf(filename, "%s/%lli.dat", path, nfile);
		fd = open(filename, O_RDWR);

		if(fd == -1)
		{
			printf("\nfile %i/%i opening error\n", nfile, MAX_CHECKED_FILES);
			fprintf(logfile, "\nfile %i/%i opening error\n", nfile, MAX_CHECKED_FILES);
			printopenerr(logfile, islogging);
			continue;
		}

		ret = lseek(fd, 0, SEEK_END);


		if(ret == -1)
		{
			printf("\nfile %i/%i size measurement error\n", nfile, MAX_CHECKED_FILES);
			fprintf(logfile, "\nfile %i/%i size measurement error\n", nfile, MAX_CHECKED_FILES);
			printlseekerr(logfile, islogging);
			continue;
		}

		sizelist[nfile] = (uint32_t) ret;

		ret = close(fd);
        if(ret == -1)
        {
            printf("\nfile %lli.dat is not closed properly!\n", nfile);
            if(islogging) fprintf(logfile, "\nfile %lli.dat is not closed properly!", nfile);
            printcloseerr(logfile, islogging);
        }
	}

	bufsize = major_occurence (sizelist, MAX_CHECKED_FILES);

    if((bufsize == 0) &&    // no explicit maximum in file sizes
       (bufsize % 16 != 0)) // file size is not aligned properly
    {
    	printf("\nError: size and seed information file is unretrievable\n");
		if(islogging) fprintf(logfile, "\nError: size and seed information file is unretrievable\n");
    	exit(1);
    }
    else return bufsize;
}

// Compares buffer data with a generated random values and counts read mismatches
// Returns error bytes count
uint32_t chkbuf_file(char * buf, uint32_t bufsize)
{
    uint32_t * ptr;
    uint32_t nerr = 0;

    for(uint32_t i = 0; i <  bufsize; i += sizeof(uint32_t))
    {
        ptr = (uint32_t *)(buf + i);

        if((*ptr) != xorshift128())
        {
            nerr += sizeof(uint32_t);
        }
    }
    return nerr;
}
