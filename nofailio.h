/*
 * nofailio.h
 */

#ifndef NOFAILIO_H_
#define NOFAILIO_H_

#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include "errmesg.h"
#include "constants.h"
#include <errno.h>

void nofail_fsync(int fd);

int nofail_open(char * path);

void nofail_close(int fd);

int nofail_unlink(char * path);

// Error-tolerant pread() ; returns bytes lost by i/o errors and pads them with zeroes
uint32_t nofail_pread(int fd, char * buf, uint32_t bufsize, uint64_t offset);

// Error-tolerant pwrite() ; returns bytes that was failed to be written
uint32_t nofail_pwrite(int fd, char * buf, uint32_t bufsize, uint64_t offset);

#endif /* NOFAILIO_H_ */
