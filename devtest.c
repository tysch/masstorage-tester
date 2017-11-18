#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "rng.h"
#include "print.h"
#include "nofailio.h"
#include "fec.h"
#include "options.h"

extern int stop_all;

// Fills device with a random data
void filldevice(char * buf, struct options_s * option)
{
    uint64_t byteswritten = 0;
    time_t startrun = time(NULL);

    uint32_t ioerrors = 0;
    uint64_t totioerrors = 0;

    if(stop_all) return;

    reseed(option->seed);
    int fd = nofail_open(option->path);
    if(fd == -1)
    {
        print(ERROR, "\nFatal error\n");
        stop_all = 1;
    }
    else
    {
        for(uint64_t wrpos = 0; wrpos < option->totsize; wrpos += option->bufsize)
        {
            if(stop_all) break;
            fillbuf(buf, option->bufsize);

            ioerrors = nofail_pwrite(fd, buf, option->bufsize, wrpos);
            byteswritten += option->bufsize - ioerrors;
            totioerrors += ioerrors;
            // Printing stats
            printprogress(perc, (uint64_t)(1000000.0*((double)byteswritten / option->totsize)));
            printprogress(mmerr, 0); //All data at write moment is considered correct
            printprogress(writeb, byteswritten);
            printprogress(tbw, option->bufsize - ioerrors);
            printprogress(ioerror, totioerrors);
            if(time(NULL) - startrun)
                printprogress(wspeed, byteswritten / (time(NULL) - startrun));
            printprogress(show, 0);
        }
    }

    nofail_close(fd);
    printprogress(log, 0);
}

// Compares buffer data with a generated random values and counts read mismatches
// Returns error bytes count and marks damaged blocks for Reed-Solomon FEC testing
uint32_t chkbuf_dev(char * buf, uint32_t bufsize,  struct fecblock * fecblocks, uint64_t *fpos, int nblocksizes, int isfectesting)
{
    uint32_t * ptr;
    uint32_t nerr = 0;
    uint32_t err_rs_block = 0;
    uint32_t posmod = 0;

    for(uint32_t i = 0; i <  bufsize; i += sizeof(uint32_t))
    {
        ptr = (uint32_t *)(buf + i);

        if((*ptr) != xorshift128())
        {
            nerr += sizeof(uint32_t);
            err_rs_block = 1;
        }

        // Reed-Solomon damaged blocks counting
        if(isfectesting)
        {
            posmod += sizeof(uint32_t);
            if(posmod == MIN_RS_BLOCKSIZE)
            {
                posmod = 0;

                if((err_rs_block != 0))
                    fecsize_test(fecblocks, 1, fpos, nblocksizes);
                else
                    fecsize_test(fecblocks, 0, fpos, nblocksizes);

                err_rs_block = 0;
            }
        }
    }
    return nerr;
}

// Reads and checks written data to the device
void readdevice(char * buf, struct options_s * option)

//(char * path, char * buf, uint32_t bufsize, uint64_t count, uint32_t seed, int isfectesting)
{
    uint64_t bytesread = 0;
    uint32_t ioerrors = 0;
    uint64_t totioerrors = 0;
    uint64_t memerrors = 0;

    int nblocksizes = 0;
    struct fecblock * fecblocks = NULL;
    uint64_t fpos = 0;

    if(stop_all) return;

    time_t startrun = time(NULL);

    int fd = nofail_open(option->path);
    if(fd == -1)
    {
        print(ERROR, "\nFatal error\n");
        stop_all = 1;
    }
    else
    {
        reseed(option->seed);

        if(option->isfectesting) fecblocks = fectest_init(option->totsize, &nblocksizes);

        for(uint64_t rdpos = 0; rdpos < option->totsize; rdpos += option->bufsize)
        {
            if(stop_all) break;
            ioerrors = nofail_pread(fd, buf, option->bufsize, rdpos);
            totioerrors += ioerrors;

            bytesread += option->bufsize - ioerrors;
            memerrors += chkbuf_dev(buf, option->bufsize, fecblocks, &fpos, nblocksizes, option->isfectesting);

            printprogress(readb, bytesread);
            printprogress(ioerror, totioerrors);
            printprogress(mmerr, memerrors);
            printprogress(perc, (uint64_t)(1000000.0*((double)bytesread / option->totsize)));
            if(time(NULL) - startrun)
                printprogress(rspeed, bytesread / (time(NULL) - startrun));
            printprogress(show, 0);

        }
    }

    nofail_close(fd);
    printprogress(log, 0);

    if(option->isfectesting)
    {
        print_fec_summary(fecblocks, nblocksizes);
        free(fecblocks);
    }
}

