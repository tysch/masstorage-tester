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

void sigint_handler(int s)
{
    if(stop_cycling && stop_all)
    {
        printf("\n\nShutting down..\n\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
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

void sigterm_handler(int s)
{
    stop_all = 1;
    stop_cycling = 1;
}

int main(int argc, char ** argv)
{
    struct sigaction siginthandler;
    struct sigaction sigtermhandler;

    enum prmode mod;
    struct options_s arguments =
    {
        .path = "-",                                    // Path to device or directory with files being tested
        .logpath = " ",                                 // Path to a directory with working files
        .seed = 1,                                      // Seed for RNG
        .iterations = 1,                                // Number of read-write cycles
        .files_per_folder =  FILES_PER_FOLDER_DEFAULT,
        .isfectesting = 0,
        .iswritingtofiles = 0,
        .islogging = 0,
        .notdeletefiles = 0,
        .measure_fs_overhead = 0,
        .randomize = 0,                                 // Randomize order of writing files or blocks
        .errcntmax = "1000",
        .per_run_errcntmax = 0,
        .totsize = 0LL,
        .bufsize = DISK_BUFFER_DEFAULT
    };

    char * buf;

    print_usage(argc);

    parse_cmd_val(argc, argv, &arguments);

    // Set SIGTERM handler for gentle shutdown along with OS.
    sigtermhandler.sa_handler = sigterm_handler;
    sigemptyset(&sigtermhandler.sa_mask);
    sigtermhandler.sa_flags = 0;
    sigaction(SIGTERM, &sigtermhandler, NULL);

    // Daemonize application
    if(arguments.background)
    {
        if(!(arguments.islogging))
        {
            puts("\nLog file is required for running into background, exiting now.");
            exit(EXIT_FAILURE);
        }
        make_daemon();
    }
    else // Enable Ctrl+C interrupts
    {
        siginthandler.sa_handler = sigint_handler;
        sigemptyset(&siginthandler.sa_mask);
        siginthandler.sa_flags = 0;
        sigaction(SIGINT, &siginthandler, NULL);
    }

    mod = parse_cmd_mode(argc, argv);

    if(arguments.totsize == 0LL) // Size of device/file in not explicitly specified
    {
        if(arguments.iswritingtofiles) arguments.totsize = free_space_in_dir(arguments.path);
        else                           arguments.totsize = read_device_size( arguments.path);
    }

    check_input_values(&arguments);

    if(!arguments.background)
    {
        if(arguments.iswritingtofiles) print_folder_size(arguments.totsize, arguments.bufsize);
        else                           print_erasure_warning(arguments.path, arguments.totsize);
    }

    if(arguments.islogging) log_init(argc, argv, arguments.logpath);

    buf = allocate_buffer(arguments.bufsize);

    switch(mod)
    {
        case singleread:
            singleread_f(buf, &arguments);
            break;

        case singlewrite:
            singlewrite_f(buf, &arguments);
            break;

        case singlecycle:
            arguments.iterations = 1;
            cycle_f(buf, &arguments);
            break;

        case multicycle:
            cycle_f(buf, &arguments);
            break;
    }

    printf("\n");
    print(LOGFILE_EXIT, " ");
    free(buf);
    return 0;
}
