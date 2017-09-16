/*
 * errmesg.h
 *
 */

#ifndef ERRMESG_H_
#define ERRMESG_H_

void printopenerr(FILE * logfile, int islogging);

void printcloseerr(FILE * logfile, int islogging);

void printlseekerr(FILE * logfile, int islogging);

void printreaderr(FILE * logfile, int islogging);

void printwriteerr(FILE * logfile, int islogging);


#endif /* ERRMESG_H_ */
