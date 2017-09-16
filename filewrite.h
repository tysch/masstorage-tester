/*
 * filewrite.h
 *
 */

#ifndef FILEWRITE_H_
#define FILEWRITE_H_

#include <stdint.h>

void writefile(uint64_t nfile, char * path, char * buf,  FILE * logfile , int islogging, uint32_t bufsize);

#endif /* FILEWRITE_H_ */
