#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "init.h"
#include "print.h"
#include "constants.h"
#include "tests.h"

// Ctrl+C interrupt handler

int stop_cycling = 0;
int stop_all = 0;

struct sigaction old_action;

void sigint_handler(int s)
{
    if(stop_cycling && stop_all)
    {
        printf("\n\nShutting down..\n\n");
        fflush(stdout);
        exit(1);
    }

    if(stop_cycling && (!stop_all))
    {
        stop_all = 1;
        printf("\nAborting current r/w operation and closing device...\n");
        fflush(stdout);
    }

    if(!stop_cycling)
    {
        stop_cycling = 1;
        printf("\nRepeated read-write cycling stopped\n");
        fflush(stdout);
    }
}

/////////////////////////////////////////////////////////////////////////////

int main(int argc, char ** argv)
{
    enum prmode mod;

    char path[PATH_MAX] = ".";      // Path to device or directory with files being tested

    uint32_t seed = 1;
    uint32_t iterations = 1;

    int isfectesting = 0;
    int iswritingtofiles = 0;

    uint64_t totsize = 0;
    uint32_t bufsize = DISK_BUFFER;

    char * buf;

    struct sigaction siginthandler;
    siginthandler.sa_handler = sigint_handler;
    sigemptyset(&siginthandler.sa_mask);
    siginthandler.sa_flags = 0;
    sigaction(SIGINT, &siginthandler, NULL);

    print_usage(argc);

    parse_cmd_val(argc, argv, path, &seed, &iterations, &isfectesting, &iswritingtofiles, &totsize, &bufsize);

    mod = parse_cmd_mode(argc, argv);

    if((!iswritingtofiles) &&   // Writing to device
    		(totsize == 0))     // Size of device in not explicitly specified
    	totsize = read_device_size(path);

    if((iswritingtofiles) &&    // Writing to a directory
    		(totsize == 0))    	// Total size of files was not explicitly
    	totsize = free_space_in_dir(path);

    check_input_values(seed, iterations, totsize, bufsize, iswritingtofiles);

    if(!iswritingtofiles) print_erasure_warning(path, totsize);

    log_init(argc, argv);

    buf = malloc(sizeof * buf * bufsize);
    if(buf == NULL)
    {
        printf("\nnot enough RAM");
        exit(1);
    }

    switch(mod)
    {
        case singleread:
            singleread_f(path, buf, bufsize, totsize, seed, isfectesting, iswritingtofiles);
            break;

        case singlewrite:
            singlewrite_f(path, buf, bufsize, totsize, seed, iswritingtofiles);
            break;

        case singlecycle:
        	cycle_f(path, buf, seed, 1, isfectesting, iswritingtofiles, totsize, bufsize);
            break;

        case multicycle:
        	cycle_f(path, buf, seed, iterations, isfectesting, iswritingtofiles, totsize, bufsize);
            break;
    }

    printf("\n");

    print(LOGFILE_EXIT, " ");
    free(buf);
    return 0;
}
