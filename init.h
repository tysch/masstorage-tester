/*
 * init.h
 *
 */

#ifndef INIT_H_
#define INIT_H_

// Program mode selection
enum prmode
{
    singlecycle,
    multicycle,
    singleread,
    singlewrite
};

void print_usage(int arg, int iswritingtofiles);

void parse_cmd_val(int argc, char ** argv, char * path, uint32_t * seed , uint32_t * iterations, int *islogging, int *isfectesting,
                   int *iswritingtofiles, uint64_t * totsize, uint32_t * filesize);


enum prmode parse_cmd_mode(int argc, char ** argv);

void print_erasure_warning(char * path);

void log_init(int argc, char ** argv, FILE **logfileptr);

#endif /* INIT_H_ */
