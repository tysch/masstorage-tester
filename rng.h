/*
 * rng.h
 */

#ifndef RNG_H_
#define RNG_H_

#include <stdint.h>

void reseed(uint32_t seed);

// Extremely fast random number generator with 2^128 long cycle
uint32_t xorshift128(void);

// Fills buffer with a random data
void fillbuf(char * buf, uint32_t bufsize);

// Initialize random number generator that cover all values from 0 to range-1 in range cycles
void uniq_rand_init(uint64_t range, uint32_t seed);

// Get next value
uint64_t uniq_rand(void);

#endif /* RNG_H_ */
