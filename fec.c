/*
 * fec.c
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "constants.h"
#include "strconv.h"
#include "fec.h"

extern int stop_all;

// Makes array of test results data for Reed-Solomon algorithm for different block size
struct fecblock * fectest_init(uint64_t byteswritten, int *nblocksizes)
{
    uint64_t bsize = MIN_RS_BLOCKSIZE;
    struct fecblock * fecblocks;

    for(uint64_t i = MIN_RS_BLOCKSIZE; i < byteswritten/GF; i *= 2LL) (*nblocksizes)++;

    fecblocks = malloc(sizeof(struct fecblock) * (*nblocksizes));

    for(int i = 0; i < (*nblocksizes); i++)
    {
        fecblocks[i].blocksize = bsize;
        fecblocks[i].errcnt = 0;
        fecblocks[i].n_gf_pos = 0;
        fecblocks[i].n_gf_cnt = 0;
        fecblocks[i].n_gf_maxcnt = 0;

        bsize *= 2;
    }

    return fecblocks;
}

// Marks damaged Reed-Solomon algorithm blocks and finds worst GF-counted chunk
// of damaged blocks
void fecsize_test(struct fecblock * fecblocks, int nerror, uint64_t *pos, int nblocksizes)
{
    *pos += MIN_RS_BLOCKSIZE;

    for(int i = 0; i < nblocksizes; i++) // per-blocksize iteration
    {
        fecblocks[i].errcnt += nerror;

        if((*pos % fecblocks[i].blocksize) == 0)  // block boundary
        {
        // damaged block count per GF-counted blocks chunk
            if(fecblocks[i].errcnt)  fecblocks[i].n_gf_cnt++;

            fecblocks[i].n_gf_pos++;

            if(fecblocks[i].n_gf_pos % GF == 0 ) //GF-counts blocks boundary
            {
                fecblocks[i].n_gf_pos = 0;

                //Most error blocks in GF-count chunk
                if(fecblocks[i].n_gf_maxcnt < fecblocks[i].n_gf_cnt)
                    fecblocks[i].n_gf_maxcnt = fecblocks[i].n_gf_cnt;

                fecblocks[i].n_gf_cnt = 0;
            }

            fecblocks[i].errcnt = 0;
        }
    }
}

// Counts damaged Reed-Solomon algorithm blocks in initial DISK_BUFFER chunk of data
void readseedandsize_fectest (char * buf, uint32_t seed, uint64_t size,
                              struct fecblock * fecblocks, uint64_t *pos, int nblocksizes, uint32_t bufsize)
{
    uint64_t * ptr = (uint64_t *) buf;
    int posmod = 0;
    int err = 0;
    for(uint32_t i = 0; i < bufsize; i += 16)
    {
        // Data format:
        // |64-bit seed | 64-bit size |64-bit seed | 64-bit size |...
        if((uint32_t)(*ptr) != seed) err = 1;
        ptr++;

        if(*ptr != size) err = 1;
        ptr++;

        posmod += 16;

        if(posmod >= MIN_RS_BLOCKSIZE)
        {
            posmod = 0;
            if(err) fecsize_test(fecblocks, 1, pos, nblocksizes);
            else    fecsize_test(fecblocks, 0, pos, nblocksizes);
            err = 0;
        }
    }
}

void print_fec_summary(struct fecblock * fecblocks, int nblocksizes, FILE * logfile, int islogging)
{
    int spareblocks;
    double overhead;
    char bsstr[20];
    int iserror = 0; // do not print summary if there are no errors

    for(int i = 0; i < nblocksizes; i++)
    {
        if(fecblocks[i].n_gf_maxcnt > 0) iserror = 1;
    }

    if(iserror)
    {
        printf("\n Forward error correction code requirements:\n");

        for(int i = 0; i < nblocksizes; i++)
        {
            bytestostr(fecblocks[i].blocksize , bsstr);
            spareblocks = 2 * fecblocks[i].n_gf_maxcnt;

            if(spareblocks >= GF/2)
            {
                printf("block size: %-12s -- insufficient block size\n", bsstr);
                if(islogging)
                    fprintf(logfile, "block size: %-12s -- insufficient block size\n", bsstr);
            }
            else
            {
                overhead = 100.0 * (double) spareblocks / GF;
                printf("block size: %-12s  spare blocks: %-3i  overhead: %.1f%%\n", bsstr, spareblocks, overhead);
                    if(islogging)
                fprintf(logfile, "block size: %-12s  spare blocks: %-3i  overhead: %.1f%%\n", bsstr, spareblocks, overhead);
            }
        }
        if(stop_all)
        {
            printf("\nWarning! FEC data can be incorrect due to aborted read\n");
            if(islogging)
                fprintf(logfile, "Warning! FEC data can be incorrect due to aborted read\n");
        }
    }
}

