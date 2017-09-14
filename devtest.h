/*
 * devtest.h
 *
 */

#ifndef DEVTEST_H_
#define DEVTEST_H_

// Fills file with a random data, measured device size and RNG seed information
uint64_t filldevice(char * path, char *buf, uint32_t seed, FILE * logfile , int islogging, uint32_t bufsize);

// Reads and checks written data to the device
void readback(char * path, char *buf, FILE * logfile, uint64_t byteswritten , int islogging, int isfectesting, uint32_t bufsize);

#endif /* DEVTEST_H_ */
