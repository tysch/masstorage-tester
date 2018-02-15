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

    for(uint32_t i = 0; i < bufsize; i += sizeof(uint32_t))
    {
        ptr = (uint32_t *)(buf + i);
        *ptr = xorshift128();
    }
}

//----------------------------------------------------------------------------------------//
// LFSR-based pseurandom number generator, for random and unique file/blocks selections.
// Tested up to 10^12 by verifying that sum of n rands with base n is equal to sum(0..n)
// Should work up to 2^63 - 2 
//----------------------------------------------------------------------------------------//

// Galouis LFSR taps
const uint64_t taps[] = 
{
    0x1ULL,                // 2^1
    0x3ULL,                // 2^2
    0x6ULL,                // 2^3

    0xCULL,                // 2^4
    0x1EULL,
    0x36ULL,
    0x78ULL,

    0xB8ULL,               // 2^8
    0x1B0ULL,
    0x360ULL,
    0x740ULL,

    0xCA0ULL,              // 2^12
    0x1F20ULL,
    0x3802ULL,
    0x7400ULL,

    0xB400ULL,             // 2^16
    0x1E000ULL,
    0x39000ULL,
    0x72000ULL,

    0x90000ULL,            // 2^20
    0x140000ULL,
    0x300000ULL,
    0x6A0000ULL,

    0xD80000ULL,           // 2^24
    0x1200000ULL,
    0x3880000ULL,
    0x7200000ULL,

    0x9000000ULL,          // 2^28
    0x14000000ULL,
    0x32800000ULL,
    0x48000000ULL,

    0x78000000ULL,         // 2^32
    0xa3000000ULL,
    0x194000000ULL,
    0x500000000ULL,

    0x801000000ULL,        // 2^36
    0x1940000000ULL,
    0x3180000000ULL,
    0x4400000000ULL,

    0x9c00000000ULL,       // 2^40
    0x12000000000ULL,
    0x29400000000ULL,
    0x63000000000ULL,

    0xa6000000000ULL,      // 2^44
    0x1b0000000000ULL,
    0x20e000000000ULL,
    0x420000000000ULL,

    0x894000000000ULL,     // 2^48
    0x2010000000000ULL,
    0x5c00000000000ULL,
    0xd200000000000ULL,

    0x12000000000000ULL,   // 2^52
    0x38800000000000ULL,
    0x49400000000000ULL,
    0xe2000000000000ULL,

    0x152000000000000ULL,  // 2^56
    0x2d0000000000000ULL,
    0x630000000000000ULL,
    0xa90000000000000ULL,

    0x1800000000000000ULL, // 2^60
    0x3900000000000000ULL,
    0x4b00000000000000ULL,
    0xc000000000000000ULL
};

static uint64_t sr;
static uint64_t rand_range;
static int lfsr_size;

static void galouis_lfsr_cycle(uint64_t taps) // Generate new random value
{
    uint64_t lsb = (sr & 1ULL);
    sr >>= 1ULL;
    if(lsb) sr ^= taps;
}

void uniq_rand_init(uint64_t range, uint32_t seed) // Determine minimal LFSR length that fits the range and sets seed
{
    rand_range = range;
    lfsr_size = 1ULL;
    while(range > 1ULL) 
    {
        lfsr_size++;
        range >>= 1;
    }
    sr = ((uint64_t) seed) % rand_range;
}

uint64_t uniq_rand(void)
{
    do
    {
        galouis_lfsr_cycle(taps[lfsr_size]);
    } 
    while (sr > rand_range);
    return (sr - 1ULL); // LFSR can not output zero
}

