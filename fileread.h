/*
 * fileread.h
 *
 *  Created on: Sep 13, 2017
 *      Author: styshchenko
 */

#ifndef FILEREAD_H_
#define FILEREAD_H_

#include <stdint.h>

int32_t readfile(uint64_t nfile, char * path, char * buf,  FILE * logfile , int islogging, uint32_t bufsize);


#endif /* FILEREAD_H_ */
