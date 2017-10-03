/*
 * saveload.h
 *
 */

#ifndef SAVELOAD_H_
#define SAVELOAD_H_

#include <stdio.h>
#include <stdint.h>

FILE * initsave (char * path);

// Loads previous seed and size information
// Format: seed=uint32_t size=uint64_t filesize=uint32_t
void load(uint32_t *seed, uint64_t *size, uint32_t *filesize);
void save(uint32_t seed, uint64_t size, uint32_t filesize);

#endif /* SAVELOAD_H_ */
