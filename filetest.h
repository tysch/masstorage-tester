/*
 * filetest.h
 *
 */

#ifndef FILETEST_H_
#define FILETEST_H_

void fillfiles(char * path, char * buf, uint32_t seed, FILE * logfile , int islogging , uint64_t totsize, uint32_t bufsize);

void readfiles(char * path, char * buf, FILE * logfile, int islogging);

#endif /* FILETEST_H_ */
