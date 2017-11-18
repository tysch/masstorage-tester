/*
 * devtest.h
 */

#ifndef DEVTEST_H_
#define DEVTEST_H_
#include "options.h"

// Fills device with a random data, measured device size and RNG seed information
void filldevice(char * buf, struct options_s * option);

// Reads and checks written data to the device
void readdevice(char * buf, struct options_s * option);

#endif /* DEVTEST_H_ */
