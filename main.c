#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "init.h"
#include "printprogress.h"
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
    FILE * logfile;
    char path[PATH_MAX];
    uint32_t seed = 1;
    uint32_t iterations;
    int islogging = 0;
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

    print_usage(argc, iswritingtofiles);
    parse_cmd_val(argc, argv, path, &seed, &iterations, &islogging, &isfectesting, &iswritingtofiles, &totsize, &bufsize);
    mod = parse_cmd_mode(argc, argv);
    check_input_values(seed, iterations, totsize, bufsize, iswritingtofiles);
    if(!iswritingtofiles) print_erasure_warning(path);

    if(islogging) log_init(argc, argv, &logfile);

    buf = malloc(sizeof * buf * bufsize);
    if(buf == NULL)
    {
        printf("\nnot enough RAM");
        exit(1);
    }

    printprogress(reset, 0, logfile);

    switch(mod)
    {
        case singleread:
            singleread_f(path, buf, seed, logfile , islogging, isfectesting, iswritingtofiles, totsize, bufsize);
            break;

        case singlewrite:
            singlewrite_f(path, buf, seed, logfile, islogging, iswritingtofiles, totsize, bufsize);
            break;

        case singlecycle:
            cycle_f(path, buf, seed, 1, logfile, islogging, isfectesting, iswritingtofiles, totsize, bufsize);
            break;

        case multicycle:
            cycle_f(path, buf, seed, iterations, logfile, islogging, isfectesting, iswritingtofiles, totsize, bufsize);
            break;
    }
    printf("\n");
    if(islogging) fclose(logfile);
    free(buf);
    return 0;
}
