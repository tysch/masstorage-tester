/*
 * fileio.h
 */
#include <stdint.h>

#ifndef FILEIO_H_
#define FILEIO_H_

// Create and write buf to file; returns number of i/o errors
uint32_t nofail_writefile(char * path, char * buf, uint32_t bufsize);

// Read and delete file; returns number of i/o errors
uint32_t nofail_readfile(char * path, char * buf, uint32_t bufsize, int notdeletefile);

#endif /* FILEIO_H_ */
