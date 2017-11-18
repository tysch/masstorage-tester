/*
 * init.h
 */

#ifndef INIT_H_
#define INIT_H_
#include "constants.h"
#include "options.h"

// Program mode selection
enum prmode
{
    singlecycle,
    multicycle,
    singleread,
    singlewrite
};

void print_usage(int arg);

void parse_cmd_val(int argc, char ** argv, struct options_s * arguments);

enum prmode parse_cmd_mode(int argc, char ** argv);

void print_erasure_warning(char * path, uint64_t size);

void log_init(int argc, char ** argv, char * logpath);

void check_input_values(struct options_s * arguments);

long long unsigned read_device_size(char * path);

uint64_t free_space_in_dir(char * path);

void print_folder_size(uint64_t totsize, uint32_t bufsize);

void make_daemon(void);

#endif /* INIT_H_ */
