/*
 * tests.h
 *
 */

#ifndef TESTS_H_
#define TESTS_H_

void singlewrite_f(char * path, char * buf, uint32_t bufsize, uint64_t totsize, uint32_t seed, int iswritingtofiles, int notdeletefiles);

void singleread_f(char * path, char * buf, uint32_t bufsize, uint64_t totsize, uint32_t seed, int isfectesting, int iswritingtofiles, int notdeletefiles);

void cycle_f(char * path, char * buf, uint32_t seed, uint32_t iterations, int isfectesting,
             int iswritingtofiles, int notdeletefiles, uint64_t totsize, uint32_t bufsize);

#endif /* TESTS_H_ */
