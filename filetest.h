/*
 * filetest.h
 */

#ifndef FILETEST_H_
#define FILETEST_H_

void fillfiles(char * path, char * buf, uint32_t seed, uint64_t totsize, uint32_t bufsize);

void readfiles(char * path, char * buf, uint32_t seed, uint64_t totsize, uint32_t bufsize, int notdeletefiles);

#endif /* FILETEST_H_ */
