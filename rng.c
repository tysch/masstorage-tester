#include <stdint.h>
#include "constants.h"

// Pseudorandom number generator state variables
uint32_t state0;
uint32_t state1;
uint32_t state2;
uint32_t state3;

void reseed(uint32_t seed)
{
    state0 = seed;
    state1 = 0;
    state2 = 0;
    state3 = 0;
}

// Extremely fast random number generator with 2^128 long cycle
uint32_t xorshift128(void)
{
    uint32_t t = state3;
    t ^= t << 11;
    t ^= t >> 8;
    state3 = state2;
    state2 = state1;
    state1 = state0;
    t ^= state0;
    t ^= state0 >> 19;
    state0 = t;
    return t;
}

// Fills buffer with a random data
void fillbuf(char * buf, uint32_t bufsize)
{
    uint32_t * ptr;

    for(uint32_t i = 0; i < bufsize; i += 4)
    {
        ptr = (uint32_t *)(buf + i);
        *ptr = xorshift128();
    }
}
