/*
 * rng.h
 *
 */

#ifndef RNG_H_
#define RNG_H_

#include <stdint.h>

void reseed(uint32_t seed);

// Extremely fast random number generator with 2^128 long cycle
uint32_t xorshift128(void);

// Fills buffer with a random data
void fillbuf(char * buf, uint32_t bufsize);

#endif /* RNG_H_ */
