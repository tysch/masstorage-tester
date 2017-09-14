/*
 * constants.h
 *
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

// Galouis field size for Reed-Solomon algorithm
#define GF 256

// Minimal Reed-Solomon block being tested
// Should be multiplier of 16
// Small blocks significantly slows down readbacks
#define MIN_RS_BLOCKSIZE 256

// Buffer size for R/W operations
// First block stores total device size and seed
// for random number generator in (bufsize/16)-modular redundancy
// Must be multiplier of 16
#define DISK_BUFFER 1024*1024

// Number of files to find size of single file
#define MAX_CHECKED_FILES 15


#endif /* CONSTANTS_H_ */
