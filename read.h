/*
 * read.h
 *
 */

#ifndef READ_H_
#define READ_H_

uint32_t fileread(int fd, char * buf, uint32_t bufsize , FILE * logfile, int islogging);

#endif /* READ_H_ */
