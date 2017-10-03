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

// # chars in a path name including nul
#define PATH_MAX        4096

// Maximum number of files to work with
#define MAX_FILE_COUNT  (1LL << 60)

// Maximum number of repeats to read or write to file/device
#define MAX_RETRIES 16

// Bytes to be skipped in case of unrecoverable read/write errors
#define SKIP_BYTES 8

#endif /* CONSTANTS_H_ */
