/*
 * filetest.c
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "constants.h"
#include "strconv.h"
#include "rng.h"
#include "print.h"
#include "fileio.h"

extern int stop_all;

void fillfiles(char * path, char * buf, uint32_t seed, uint64_t totsize, uint32_t bufsize)
{
    char filename[PATH_MAX + 24];
    char errmesg[128];

    uint64_t nfiles = totsize / bufsize;
    time_t startrun = time(NULL);
    uint64_t byteswritten = 0;

    uint32_t ioerrors = 0;
    uint64_t totioerrors = 0;

    reseed(seed);

    for(uint64_t i = 0; i < nfiles; i++)
    {
        if(stop_all) break;

        fillbuf(buf, bufsize);

        sprintf(filename, "%s/%lli.jnk", path, (long long) i);
        ioerrors = nofail_writefile(filename, buf, bufsize);

        byteswritten += bufsize - ioerrors;
        totioerrors += ioerrors;

        if (ioerrors)
        {
            sprintf(errmesg, "\n%lli.jnk write error\n", (long long) i);
            print(ERROR, errmesg);
        }

        printprogress(writeb, byteswritten);
        printprogress(mmerr, 0); //All data at write moment is considered correct
        printprogress(ioerror, totioerrors);
        printprogress(tbw, bufsize);
        if(time(NULL) - startrun)
            printprogress(wspeed, byteswritten / (time(NULL) - startrun));
        printprogress(show, 0);
        printprogress(perc, (uint64_t)(1000000.0*((double)(i + 1) / nfiles)));
    }

    printprogress(log, 0);
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


void readfiles(char * path, char * buf, uint32_t seed, uint64_t totsize, uint32_t bufsize, int notdeletefiles)
{

    char filename[PATH_MAX + 24];
    char errmesg[128];
    char errsize[24];

    uint64_t nfiles = totsize / bufsize;
    time_t startrun = time(NULL);
    uint64_t bytesread = 0;

    uint32_t ioerrors = 0;
    uint64_t totioerrors = 0;
    uint64_t memerrors = 0;

    reseed(seed);

    for(uint64_t i = 0; i < nfiles; i++)
    {
        if(stop_all) break;
        sprintf(filename, "%s/%lli.jnk", path, (long long) i);

        ioerrors = nofail_readfile(filename, buf, bufsize, notdeletefiles);
        totioerrors += ioerrors;
        bytesread += bufsize - ioerrors;
        memerrors += chkbuf_file(buf, bufsize);

        if(ioerrors)
        {
            bytestostr(ioerrors, errsize);
            sprintf(errmesg, "\n--------------------file %-15lli.jnk is damaged, %-9s errors\n", (long long) i, errsize);
            print(ERROR, errmesg);
        }

        printprogress(readb, bytesread);
        printprogress(perc, (uint64_t)(1000000.0*((double)(i + 1) / nfiles)));
        printprogress(mmerr, memerrors);
        printprogress(ioerror, totioerrors);
        if(time(NULL) - startrun)
            printprogress(rspeed, bytesread / (time(NULL) - startrun));
        printprogress(show, 0);
    }

    printprogress(log, 0);
}

