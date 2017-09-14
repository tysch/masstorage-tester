/*
 * devread.h
 *
 */

#ifndef DEVREAD_H_
#define DEVREAD_H_

// Compares buffer data with a generated random values and counts read mismatches
// Returns error bytes count
uint32_t chkbuf_dev(char * buf, uint32_t bufsize,  struct fecblock * fecblocks, uint64_t *fpos, int nblocksizes, int isfectesting);

// Recovers seed and size information from buffer
// Returns error bytes count
int32_t readseedandsize (char * buf, uint32_t bufsize, uint32_t *seed, uint64_t *size);

#endif /* DEVREAD_H_ */
