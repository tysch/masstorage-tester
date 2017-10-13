/*
 * init.h
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

void print_usage(int arg);

void parse_cmd_val(int argc, char ** argv, char * path, uint32_t * seed , uint32_t * iterations, int *isfectesting,
                   int *iswritingtofiles, int *notdeletefiles, int * headless, uint64_t * totsize, uint32_t * filesize);

enum prmode parse_cmd_mode(int argc, char ** argv);

void print_erasure_warning(char * path, uint64_t size);

void log_init(int argc, char ** argv);

void check_input_values(uint32_t seed, uint32_t iterations, uint64_t totsize, uint32_t bufsize, int iswritingtofiles);

long long unsigned read_device_size(char * path);

uint64_t free_space_in_dir(char * path);

void print_folder_size(uint64_t totsize, uint32_t bufsize);

void make_daemon(void);

#endif /* INIT_H_ */
