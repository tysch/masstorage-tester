/*
 * tests.h
 *
 */

#ifndef TESTS_H_
#define TESTS_H_

void singlewrite_f(char * path, char * buf, uint32_t seed, FILE * logfile , int islogging,
             int iswritingtofiles, uint64_t totsize, uint32_t bufsize);

void singleread_f(char * path, char * buf, uint32_t seed, FILE * logfile , int islogging, int isfectesting,
             int iswritingtofiles, uint64_t totsize, uint32_t bufsize);

void cycle_f(char * path, char * buf, uint32_t seed, uint32_t iterations, FILE * logfile , int islogging, int isfectesting,
             int iswritingtofiles, uint64_t totsize, uint32_t bufsize);

#endif /* TESTS_H_ */
