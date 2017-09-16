/*
 * devread.c
 *
 */

#include <stdio.h>
#include <stdint.h>
#include "constants.h"
#include "rng.h"
#include "fec.h"

// Compares buffer data with a generated random values and counts read mismatches
// Returns error bytes count
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

// Recovers seed and size information from buffer
// Returns error bytes count
int32_t readseedandsize (char * buf, uint32_t bufsize, uint32_t *seed, uint64_t *size)
{
    // |64-bit seed | 64-bit size |64-bit seed | 64-bit size |...
    uint32_t bitcount[128] = {0}; // Count of individual bits in 128-bit sequence
    char rcblock[16] = {0};
    uint64_t * ptr;
    int32_t errcount = 0;

    // bit frequency counting
    for(uint32_t i = 0; i <  bufsize; i += 16) // per-128-bit block iteration
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
    for(uint32_t i = 0; i < 2 * sizeof(uint64_t); i++) // per-byte iteration
    {
        for(uint32_t j = 0; j < 8; j++) // per-bit iteration
        {
            if(bitcount[8*i + j] > (bufsize / (2*sizeof(uint64_t)*2)))  // set bit as the majority are
                rcblock[i] |= 1 << j;

            if(bitcount[8*i + j] == (bufsize / (2*sizeof(uint64_t)*2))) // unrecoverable data condition
                return -1;
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
