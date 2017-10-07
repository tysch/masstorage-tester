/*
 * devtest.h
 */

#ifndef DEVTEST_H_
#define DEVTEST_H_

// Fills device with a random data, measured device size and RNG seed information
void filldevice(char * path, char * buf, uint32_t bufsize, uint64_t count, uint32_t seed);

// Reads and checks written data to the device
void readdevice(char * path, char * buf, uint32_t bufsize, uint64_t count, uint32_t seed, int isfectesting);

#endif /* DEVTEST_H_ */
