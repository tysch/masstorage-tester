/*
 * options.h
 */

#ifndef PARAMS_H_
#define PARAMS_H_
#include "constants.h"

struct options_s
{
    char path[PATH_LENGTH];      // Path to device or directory with files being tested
    char logpath[PATH_LENGTH];   // Path to a directory with working files

    uint32_t seed;
    uint32_t iterations;

    uint32_t files_per_folder;

    int isfectesting;
    int iswritingtofiles;
    int islogging;
    int notdeletefiles;
    int background;
    int measure_fs_overhead;
    char *errcntmax;

    uint64_t totsize;
    uint32_t bufsize;
};

#endif /* PARAMS_H_ */
