/*
 * fec.c
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "constants.h"
#include "strconv.h"
#include "print.h"
#include "fec.h"

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

void print_fec_summary(struct fecblock * fecblocks, int nblocksizes)
{
    int spareblocks;
    double overhead;
    char bsstr[DIGITS_MAX];
    char str[5 * DIGITS_MAX];
    int iserror = 0; // do not print summary if there are no errors

    for(int i = 0; i < nblocksizes; i++)
    {
        if(fecblocks[i].n_gf_maxcnt > 0) iserror = 1;
    }

    if(iserror)
    {
        print(OUT, "\n Forward error correction code requirements:\n");

        for(int i = 0; i < nblocksizes; i++)
        {
            bytestostr(fecblocks[i].blocksize , bsstr);
            spareblocks = 2 * fecblocks[i].n_gf_maxcnt;

            if(spareblocks >= GF/2)
            {
                sprintf(str, "block size: %-12s -- insufficient block size\n", bsstr);
                print(OUT, str);
            }
            else
            {
                overhead = 100.0 * (double) spareblocks / GF;
                sprintf(str, "block size: %-12s  spare blocks: %-3i  overhead: %.1f%%\n", bsstr, spareblocks, overhead);
                print(OUT, str);
            }
        }
    }
}
