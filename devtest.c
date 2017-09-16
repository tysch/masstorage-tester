/*
 * devtest.c
 *
 */

#define _XOPEN_SOURCE 500
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include "fec.h"
#include "rng.h"
#include "devwrite.h"
#include "devread.h"
#include "read.h"
#include "printprogress.h"
#include "strconv.h"


extern int stop_all;

// Fills file with a random data, measured device size and RNG seed information
uint64_t filldevice(char * path, char *buf, uint32_t seed, FILE * logfile , int islogging, uint32_t bufsize)
{
    static uint64_t prevbyteswritten = 0; // Previously measured device size for progress counting
    uint64_t byteswritten = 0;
    int32_t ret;
    time_t startrun = time(NULL);

    int fd = device_init_write(path, logfile, islogging, bufsize);

    while(!stop_all)   // Stopping with Ctrl+C
    {
        ret = write_rand_block(buf, fd, logfile, islogging, bufsize);
        if (ret == -1) break;

        byteswritten += ret;
        // Progress print section
        if(prevbyteswritten)
            printprogress(perc, (uint64_t)(1000000.0*((double)byteswritten / prevbyteswritten)), logfile);

        printprogress(writeb, byteswritten, logfile);
        printprogress(tbw, ret, logfile);

        if(time(NULL) - startrun)
            printprogress(wspeed, byteswritten / (time(NULL) - startrun), logfile);

        printprogress(print, 0, logfile);
        // Breaks if no more data can be write to device
        if(ret < bufsize ) break;
    }

    byteswritten += bufsize ;
    writeseedandsize(buf, seed, byteswritten, bufsize);

    printprogress(writeb, byteswritten, logfile);
    printprogress(tbw, bufsize, logfile);

    if(prevbyteswritten)
        printprogress(perc, (uint64_t)(1000000.0*((double)byteswritten / prevbyteswritten)), logfile);

    ret = pwrite (fd, buf, bufsize , 0);

    if(ret != bufsize)
    {
        printf("\nHeader write failure!\n");
        if(islogging) fprintf(logfile, "\nHeader write failure!");
    }


    if(close(fd) == -1)
    {
        printf("\ndevice is not closed properly!\n");
        if(islogging) fprintf(logfile, "\ndevice is not closed properly!");
    }

    printprogress(print, 0, logfile);
    if(islogging) printprogress(log, 0, logfile);

    if(!prevbyteswritten)
    {
        prevbyteswritten = byteswritten;
        printprogress(size, byteswritten, logfile);
    }
    return byteswritten;
}

// Reads and checks written data to the device
void readback(char * path, char *buf, FILE * logfile, uint64_t byteswritten , int islogging, int isfectesting, uint32_t bufsize)
{
    uint64_t bytesread = 0;
    ssize_t bufread;
    uint32_t seed;
    time_t startrun = time(NULL);
    int firstcycle = 1;
    static int firstrun = 1;
    uint32_t errcnt = 0;
    char * tmpptr;
    char rdstr[20];

    struct fecblock * fecblocks;
    int nblocksizes = 0;
    uint64_t fpos = 0;

    if(isfectesting) fecblocks = fectest_init(byteswritten, &nblocksizes);

    int fd = open(path, O_RDONLY);

    if(fd == -1)
    {
        printf("\nDevice access error!\n");
        if(islogging) fprintf(logfile, "\nDevice access error!\n");
        exit(1);
    }

    while (!stop_all)
    {
        bufread = fileread(fd, buf, bufsize  , logfile, islogging);
        bytesread += bufread;
        // Progress print section
        printprogress(readb, bytesread, logfile);

        // Recovers size and seed information and prepares forward error correction testing routine
        if(firstcycle)
        {
            firstcycle = 0;
            errcnt = readseedandsize(buf, bufread, &seed, &byteswritten);
            printprogress(mmerr, errcnt, logfile);
            reseed(seed);

            if(isfectesting)
                readseedandsize_fectest(buf, seed, byteswritten, fecblocks, &fpos, nblocksizes, bufsize);

            continue;
        }

        errcnt = chkbuf_dev(buf, bufread, fecblocks, &fpos, nblocksizes, isfectesting);
        printprogress(mmerr, errcnt, logfile);

        if(byteswritten)
            printprogress(perc, (uint64_t)(1000000.0*((double)bytesread / byteswritten)), logfile);

        if(time(NULL) - startrun)
            printprogress(rspeed, bytesread / (time(NULL) - startrun), logfile);

        printprogress(print, 0, logfile);

        if (bufread < bufsize) break;
    }

    if(close(fd) == -1)
    {
        printf("\ndevice is not closed properly!\n");
        if(islogging) fprintf(logfile, "\ndevice is not closed properly!");
    }

    if(firstrun)
    {
        bytestostr(bytesread, rdstr);
        printf("\nRead back %s of data\n", rdstr);
        if(islogging) fprintf(logfile, "\nRead back %s of data\n", rdstr);
        firstrun = 0;
    }

    if(islogging) printprogress(log, 0, logfile);

    if(isfectesting)
    {
        print_fec_summary(fecblocks, nblocksizes, logfile, islogging);
        free(fecblocks);
    }
}

