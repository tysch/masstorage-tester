/*
 * init.c
 * Routines for input of application
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "init.h"
#include "strconv.h"

void print_usage(int arg, int iswritingtofiles)
{
    if(arg < 3)
    {
        printf("\n\nUsage: -d <path> [-o|r|w|c <iterations>] [-i <salt>] [-l] [-f]");
        printf("[-u <totalsize[kKmMgGt]> <filesize[kKmMgGt]>]");
        printf("\n -d -- path to test device or file");
        printf("\n -o -- single read/write cycle, for speed and volume measurement, default");
        printf("\n -u -- test ");
        printf("\n -c -- <iterations> read/write cycles, for endurance tests");
        printf("\n -w -- single write only");
        printf("\n -r -- single read only");
        printf("\n -w, -r are useful for long term data retention tests");
        printf("\n           -w and -r must be launched with the same salt");
        printf("\n -i -- integer salt for random data being written, default 1\n");
        printf("\n -l -- write a log file\n");
        printf("\n -f -- estimates Reed-Solomon forward error correction code requirement");
        printf("            for GF=256, spare blocks count vs block size \n\n");
        printf("\n -u -- write to a bunch of files instead of device with defined size\n");
        printf("\n       filesize should be less than 4 GiB\n");
        exit(1);
    }
}

void parse_cmd_val(int argc, char ** argv, char * path, uint32_t * seed , uint32_t * iterations, int *islogging, int *isfectesting,
                   int *iswritingtofiles, uint64_t * totsize, uint32_t * filesize)
{
	uint32_t filesiz;

	for(int i = 0; i < argc; i++)
    {
        if(strcmp(argv[i],"-d") == 0)
        {
            if(i + 1 == argc) exit(1);
            else
                strcpy(path,argv[i + 1]);
        }

        if(strcmp(argv[i],"-i") == 0)
        {
            if((i + 1 == argc)) exit(1);
            else
                sscanf(argv[i + 1], "%i", seed);
        }

        if(strcmp(argv[i],"-c") == 0)
        {
            if((i + 1 == argc)) exit(1);
            else
                sscanf(argv[i + 1], "%i", iterations);
        }

        if(strcmp(argv[i],"-u") == 0)
        {
            if((i + 1 == argc)) exit(1);
            if((i + 2 == argc)) exit(1);
            else
            {
            	*iswritingtofiles = 1;
            	*totsize = tobytes(argv[i + 1]);
            	filesiz = tobytes(argv[i + 2]);
            	if(filesiz >= 0x7FFFFFFFLL) filesiz = 0x7FFFFFFF;
            	if(filesiz < 16) filesiz = 16;
            	filesiz -= (filesiz % 16);
                *filesize = (uint32_t) filesiz;
            }
        }

        if(strcmp(argv[i],"-l") == 0)
            *islogging = 1;

        if(strcmp(argv[i],"-f") == 0)
            *isfectesting = 1;
    }

    if((getuid()) && (!(*iswritingtofiles)))
    {
        printf("\n Writing to a raw device requires root privileges\n");
        exit(1);
    }
}

enum prmode parse_cmd_mode(int argc, char ** argv)
{
    enum prmode ret = singlecycle;
    for(int i = 0; i < argc; i++)
    {
        if(strcmp(argv[i],"-o") == 0)
        {
            ret = singlecycle;
            break;
        }
        if(strcmp(argv[i],"-c") == 0)
        {
            ret = multicycle;
            break;
        }
        if(strcmp(argv[i],"-w") == 0)
        {
            ret = singlewrite;
            break;
        }
        if(strcmp(argv[i],"-r") == 0)
        {
            ret = singleread;
            break;
        }
    }
    return ret;
}

void print_erasure_warning(char * path)
{
    char scmd[256];
    printf("\nWARNING! All data on the device would be lost!\n");
    strcpy(scmd, "fdisk -l | grep ");
    strcat(scmd, path);
    system(scmd);
    printf("\nIs the device correct? y/n\n");
    if(getchar() != 'y') exit(1);
}

void log_init(int argc, char ** argv, FILE **logfileptr)
{
    char logname[25];
    struct tm * timeinfo;
    time_t startrun = time(NULL);
    time_t rawtime;
    sprintf(logname, "test-%lli.log", (long long int) startrun);
    *logfileptr = fopen(logname, "w+");

    if(*logfileptr == NULL)
    {
        printf("\nCannot create logfile!\n");
        exit(1);
    }

    for(int i = 0; i < argc; i++)
        fprintf(*logfileptr ,"%s ",argv[i]);

    fprintf(*logfileptr ,"\n\n");
    time (&rawtime);
    timeinfo = localtime(&rawtime);
    fprintf (*logfileptr , "Started at: %s\n", asctime (timeinfo));
}
