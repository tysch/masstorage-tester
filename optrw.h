/*
 * optrw.h
 *
 */

#ifndef OPTRW_H_
#define OPTRW_H_

// Attempts to recover file size from MAX_CHECKED_FILES,
// Error-tolerant
uint32_t bufsize_retrieve(char * path, FILE * logfile, int islogging);

// Compares buffer data with a generated random values and counts read mismatches
// Returns error bytes count
uint32_t chkbuf_file(char * buf, uint32_t bufsize);

#endif /* OPTRW_H_ */
