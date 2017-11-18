/*
 * saveload.h
 *
 */

#ifndef SAVELOAD_H_
#define SAVELOAD_H_

#include <stdio.h>
#include <stdint.h>
#include "options.h"

FILE * initsave (char * path);

// Loads previous seed and size information
// Format: seed=uint32_t size=uint64_t filesize=uint32_t
void load(struct options_s * options);
void save(struct options_s * options);

#endif /* SAVELOAD_H_ */
