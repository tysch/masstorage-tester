/*
 * fec.h
 *
 */

#ifndef FEC_H_
#define FEC_H_

// Reed-Solomon algorithm test results data, per block size
struct fecblock
{
    uint32_t errcnt;      // marks current block as errorneous
    uint64_t blocksize;
    uint32_t n_gf_pos;    // block position, modulo GF
    uint32_t n_gf_cnt;    // current GF-chunk error count
    uint32_t n_gf_maxcnt; // maximum encountered GF-chunk error count
};

struct fecblock * fectest_init(uint64_t byteswritten, int *nblocksizes);

// Marks damaged Reed-Solomon algorithm blocks and finds worst GF-counted chunk
// of damaged blocks
void fecsize_test(struct fecblock * fecblocks, int nerror, uint64_t *pos, int nblocksizes);

void print_fec_summary(struct fecblock * fecblocks, int nblocksizes);

#endif /* FEC_H_ */
