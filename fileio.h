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

// Initialize directory tree for nfiles
void create_dirs(char * path, uint64_t nfiles, uint32_t files_per_folder);

// Generate filename for n-th file in a directory tree
void path_append(char * path, char * fullpath, uint64_t n, uint64_t totnfiles, uint32_t files_per_folder);

// rm -rf-like routine; warns about files that was not deleted properly
void delall(char * path);

#endif /* FILEIO_H_ */
