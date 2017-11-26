/*
 * init.c
 * Routines for input of application
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "init.h"
#include "strconv.h"
#include "constants.h"
#include "print.h"
#include <sys/statvfs.h>
#include <errno.h>

void print_usage(int arg)
{
    if(arg < 3)
    {
        puts("\n\n\n                                    Universal mass storage tester");
        puts("\nUsage: massstoragetester -d <path> [-b] [-l <path>] [-e] [-s  <total size[kKmMgGtT]>] [-o|r|w|c <iterations>]");
        puts("                          [-i <salt>][-t] [-f <file size[kKmMgGtT]> [-m <count>] [-q|-a] [-p]]\n");
        puts(" -d --destination <path>  -- Path to test device or files root.");
        puts(" -s --total-size  <size>  -- Total bytes count to be written, rounded down to the size of a buffer.");
        puts("                             If not set, massstoragetester would fill entire device or all free space in target location.");
        puts("                             Please note that files may have filesystem-specific storage overhead and automatic free space estimation");
        puts("                             while setting file size too small may cause overflow. It shoudn't be considered as a bug.");
        puts("");
        puts(" -o --single              -- Single read/write cycle, for speed and volume measurement, default.");
        puts(" -c --cycle <iterations>  -- <iterations> read/write cycles, for endurance tests.");
        puts(" -w --write               -- Single write only.");
        puts(" -r --read                -- Single read only.");
        puts("                             -w, -r are useful for long term data retention tests.");
        puts("                             -w and -r must be launched with the same salt.");
        puts("");
        puts(" -f --file <size>         -- Write to a bunch of files instead of device with defined size.");
        puts("                             Single file should be less than 2 GiB and not larger than total data size.");
        puts(" -m --per-folder <count>  -- Set number of files per folder. If total files count exceeds <count>, they would be placed to a recursively created subdirectories");
        puts(" -p --preserve-files      -- Do not clean up files while reading back in file test mode.");
        puts("                             Useful for long-term storage reliability testing.");
        puts(" -q --overhead            -- Print filesystem storage overhead for the first run.");
        puts(" -a --overhead-continuous -- Print filesystem storage overhead for each read-write cycle.");
        puts("");
        puts(" -t --fec                 -- Estimates Reed-Solomon forward error correction code requirement for raw device for GF=256,");
        puts("                             spare blocks count vs block size.");
        puts("                             Total file size can be rounded down to a more optimal values.");
        puts("");
        puts(" -i --salt                -- Non-zero integer salt for random data being written, default 1.");
        puts(" -b --background          -- Skip all confirmations and start a new test as a daemon.");
        puts(" -l --logfile-path <path> -- Path to save and log file.");
        puts(" -e --error-cap           -- Max number of system detected i/o errors before stopping test, default 1000.");
        puts(" -h --help                -- Prints this message.");
        puts("");
        exit(EXIT_SUCCESS);
    }
}

void missing_argument(char * str)
{
    printf("\nMissing argument for %s option, exiting now\n", str);
    exit(EXIT_FAILURE);
}

void parse_cmd_val(int argc, char ** argv, struct options_s * arguments)
{
    uint32_t filesiz;

    for(int i = 0; i < argc; i++)
    {
        if((strcmp(argv[i], "-d") == 0) || (strcmp(argv[i], "--destination") == 0))
        {
            if(i + 1 == argc) missing_argument(argv[i]);
            else
                strcpy(arguments->path, argv[i + 1]);
        }

        if((strcmp(argv[i], "-l") == 0) || (strcmp(argv[i], "--logfile-path") == 0))
        {
            if(i + 1 == argc) missing_argument(argv[i]);
            else
                strcpy(arguments->logpath, argv[i + 1]);
        }

        if((strcmp(argv[i], "-i") == 0) || (strcmp(argv[i], "--salt") == 0))
        {
            if((i + 1 == argc)) missing_argument(argv[i]);
            else
                arguments->seed = atoi(argv[i + 1]);
        }

        if((strcmp(argv[i], "-c") == 0) || (strcmp(argv[i], "---cycle") == 0))
        {
            if((i + 1 == argc)) missing_argument(argv[i]);
            else
                arguments->iterations = atoi(argv[i + 1]);
        }

        if((strcmp(argv[i], "-f") == 0) || (strcmp(argv[i], "--file") == 0))
        {
            if((i + 1 == argc)) missing_argument(argv[i]);
            else
            {
                arguments->iswritingtofiles = 1;
                filesiz = tobytes(argv[i + 1]);
                if(filesiz >= 0x7FFFFFFFLL) filesiz = 0x7FFFFFFFLL;
                if(filesiz < 16) filesiz = 16;
                filesiz -= (filesiz % 16);
                arguments->bufsize = (uint32_t) filesiz;
            }
        }

        if((strcmp(argv[i], "-s") == 0) || (strcmp(argv[i], "--total-size") == 0))
        {
            if((i + 1 == argc)) missing_argument(argv[i]);
            else
            {
                arguments->totsize = tobytes(argv[i + 1]);
            }
        }

        if((strcmp(argv[i], "-e") == 0) || (strcmp(argv[i], "--error-cap") == 0))
        {
            if((i + 1 == argc)) missing_argument(argv[i]);
            else
            {
                arguments->errcntmax = argv[i + 1];
            }
        }

        if((strcmp(argv[i], "-m") == 0) || (strcmp(argv[i], "--per-folder") == 0))
        {
            if((i + 1 == argc)) missing_argument(argv[i]);
            else
            {
                arguments->files_per_folder = (uint32_t) tobytes(argv[i + 1]);
                if(arguments->files_per_folder > FILES_PER_FOLDER_MAX) arguments->files_per_folder = FILES_PER_FOLDER_MAX;
            }
        }

        if((strcmp(argv[i], "-t") == 0) || (strcmp(argv[i], "--fec") == 0)) arguments->isfectesting = 1;

        if((strcmp(argv[i], "-p") == 0) || (strcmp(argv[i], "--preserve-files") == 0))      arguments->notdeletefiles = 1;
        if((strcmp(argv[i], "-b") == 0) || (strcmp(argv[i], "--background") == 0))          arguments->background = 1;
        if((strcmp(argv[i], "-q") == 0) || (strcmp(argv[i], "--overhead") == 0))            arguments->measure_fs_overhead = ONESHOT;
        if((strcmp(argv[i], "-a") == 0) || (strcmp(argv[i], "--overhead-continuous") == 0)) arguments->measure_fs_overhead = REPEATED;

        if((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0)) print_usage(0);
    }

    if(strcmp(arguments->path, "-") == 0)
    {
        printf("\n No directory or device specified, exiting now\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    if(((arguments->path[0] != '/') && (arguments->isfectesting == 1)))
    {
        printf("\nBackground run requires absolute path to the file/device.\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }

    if((getuid()) && (!(arguments->iswritingtofiles)))
    {
        printf("\nWriting to a raw device requires root privileges\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
}

enum prmode parse_cmd_mode(int argc, char ** argv)
{
    for(int i = 0; i < argc; i++)
    {
        if((strcmp(argv[i],"-o") == 0) || (strcmp(argv[i],"--single") == 0))
        {
            return singlecycle;
        }
        if((strcmp(argv[i],"-c") == 0) || (strcmp(argv[i],"--cycle") == 0))
        {
            return multicycle;
        }
        if((strcmp(argv[i],"-w") == 0) || (strcmp(argv[i],"--write") == 0))
        {
            return singlewrite;
        }
        if((strcmp(argv[i],"-r") == 0) || (strcmp(argv[i],"--read") == 0))
        {
            return singleread;
        }
    }
    return singlecycle;
}

void print_erasure_warning(char * path, uint64_t size)
{
    char str[PATH_LENGTH];
    char sizestr[DIGITS_MAX];
    int res;

    printf("\nWARNING! All data on the device would be lost!\n");

    strcpy(str, "Target device ");
    strcat(str, path);
    strcat(str, " with total size ");
    bytestostr(size, sizestr);
    strcat(str, sizestr);
    printf("%s",str);

    printf("\nIs the device correct? y/n\n");
    res = getchar();
    fflush(stdin);
    if(res != 'y') exit(EXIT_SUCCESS);

    else           printf("\nSearching for a previous run...\n");
    fflush(stdout);
}

void log_init(int argc, char ** argv, char * logpath)
{
    static char argstr[PATH_LENGTH];
    static char logname[PATH_LENGTH];
    struct tm * timeinfo;
    time_t startrun = time(NULL);
    time_t rawtime;
    sprintf(logname, "%s/test-%llu.log", logpath, (unsigned long long int) startrun);

    print(LOGFILE_INIT, logname);

    for(int i = 0; i < argc; i++)
    {
        strcat(argstr, argv[i]);
        strcat(argstr, " ");
    }

    print(LOG, argstr);

    time (&rawtime);
    timeinfo = localtime(&rawtime);
    sprintf(argstr, "\nStarted at: %s\n", asctime (timeinfo));

    print(LOG, argstr);
}

void check_input_values(struct options_s * arguments)
{
    if(arguments->iswritingtofiles)
    {
        if(arguments->bufsize > (uint32_t) arguments->totsize)
        {
            printf("\nSingle file size is bigger than the total data size\n");
            exit(EXIT_FAILURE);
        }
    }
    if(arguments->totsize < 4)
    {
        printf("\nTotal size should not be zero\n");
        exit(EXIT_FAILURE);
    }
    if(arguments->seed == 0)
    {
        printf("\nseed value cannot be be zero\n");
        exit(EXIT_FAILURE);
    }
}

long long unsigned read_device_size(char * path)
{
    static char syspath[128];
    int slashpos = 0;
    int i = 0;
    FILE * fp;
    long long unsigned size = 0;
    // Find last / in path string
    for(; i < PATH_LENGTH; i++)
    {
        if(path[i] == '\0') break;
        if(path[i] == '/') slashpos = i;
    }

    // Strip device name
    path += slashpos;

    // Compose /sys/class/block/sd*/size path
    strcpy(syspath, "/sys/block");
    strcat(syspath, path);
    strcat(syspath, "/size");
    fp = fopen(syspath, "r");
    if(fp != NULL)
    {
        if(fscanf(fp, "%llu", &size) != 1)
        {
            printf("\nCannot read device size information from a system\n");
            exit(EXIT_FAILURE);
        }
        fclose(fp);
        return (size * SYSFS_SIZE_BLOCKSIZE) ;
    }
    else
    {
        printf("\nCannot open %s\n", syspath);
        exit(EXIT_FAILURE);
    }
    return 0;
}

uint64_t free_space_in_dir(char * path)
{
    uint64_t freesize = 0;
    uint64_t blocksize;
    uint64_t freeblocks;

    struct statvfs buf;

    int ret = statvfs(path, &buf);
    if(ret == -1)
    {
        perror("\nUnable to determine remaining free space");
        printf("\nNo space information provided, exiting now.");
        exit(EXIT_FAILURE);
    }

    blocksize = buf.f_bsize;
    freeblocks = buf.f_bavail;
    freesize = blocksize * freeblocks;

    return freesize;
}

void print_folder_size(uint64_t totsize, uint32_t bufsize)
{
    static char sizestr[DIGITS_MAX];
    totsize -= totsize % bufsize;
    bytestostr(totsize, sizestr);
    printf("\nTesting %s free space\n", sizestr);
}

// TODO: replace custom prints to perror
void make_daemon(void)
{
    pid_t pid;
    // create new process
    pid = fork();
    if(pid == -1)
    {
        perror("\nFailed to create a daemon process");
        exit(EXIT_FAILURE);
    }
    else if(pid != 0)
    {
        printf("\nGoing to the background\n");
        exit(EXIT_SUCCESS);
    }

    // create new session and process group
    if(setsid() == -1)
    {
        perror("Failed to create new sid");
        exit(EXIT_FAILURE);
    }

    // set the working directory to the root directory
    if(chdir("/") == -1)
    {
        perror("Failed to set working directory to /");
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // redirect fd's 0,1,2 to /dev/null
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);
}
