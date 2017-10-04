/*
 * init.c
 * Routines for input of application
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include "init.h"
#include "strconv.h"
#include "constants.h"
#include "print.h"

void print_usage(int arg)
{
    if(arg < 3)
    {
        printf("\n\nUsage: -d <path> -s  <total size[kKmMgGt]> [-o|r|w|c <iterations>] [-i <salt>][--fec] [-f <file size[kKmMgGt]>]\n");
        printf("\n -d     -- path to test device or file\n");
        printf("\n -s     -- total bytes count to be written, rounded down to the size of a buffer\n");
        printf("\n -o     -- single read/write cycle, for speed and volume measurement, default\n");
        printf("\n -c     -- <iterations> read/write cycles, for endurance tests\n");
        printf("\n -w     -- single write only");
        printf("\n -r     -- single read only");
        printf("\n           -w, -r are useful for long term data retention tests");
        printf("\n           -w and -r must be launched with the same salt\n");
        printf("\n -i     -- non-zero integer salt for random data being written, default 1\n");
        printf("\n --fec  -- estimates Reed-Solomon forward error correction code requirement for raw device");
        printf(" for GF=256, spare blocks count vs block size \n");
        printf("\n -f     -- write to a bunch of files instead of device with defined size\n");
        printf("\n           single file should be less than 2 GiB and no more than total data size\n");
        printf("\n           total file size can be rounded down to a more optimal values\n\n");
        exit(1);
    }
}

void parse_cmd_val(int argc, char ** argv, char * path, uint32_t * seed , uint32_t * iterations, int *isfectesting,
                   int *iswritingtofiles, uint64_t * totsize, uint32_t * filesize)
{
	uint32_t filesiz;

	for(int i = 0; i < argc; i++)
    {
        if(strcmp(argv[i],"-d") == 0)
        {
            if(i + 1 == argc) exit(1);
            else
                strcpy(path, argv[i + 1]);
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

        if(strcmp(argv[i],"-f") == 0)
        {
            if((i + 1 == argc)) exit(1);
            else
            {
            	*iswritingtofiles = 1;
            	filesiz = tobytes(argv[i + 1]);
            	if(filesiz >= 0x7FFFFFFFLL) filesiz = 0x7FFFFFFF;
            	if(filesiz < 16) filesiz = 16;
            	filesiz -= (filesiz % 16);
                *filesize = (uint32_t) filesiz;
            }
        }

        if(strcmp(argv[i],"-s") == 0)
        {
            if((i + 1 == argc)) exit(1);
            else
            {
            	*totsize = tobytes(argv[i + 1]);
            }
        }

        if(strcmp(argv[i],"--fec") == 0)
            *isfectesting = 1;
    }

	if(strcmp(path, ".") == 0)
	{
        printf("\n No directory or device specified, exiting now\n");
        fflush(stdout);
        exit(1);
	}

    if((getuid()) && (!(*iswritingtofiles)))
    {
        printf("\n Writing to a raw device requires root privileges\n");
        fflush(stdout);
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
	//TODO: omit fdisk call

    char scmd[PATH_MAX];
    int ret;
    printf("\nWARNING! All data on the device would be lost!\n");
    strcpy(scmd, "fdisk -l | grep ");
    strcat(scmd, path);
    ret = system(scmd);
    if(ret) printf("WARNING: device seems to be missing!");
    printf("\nIs the device correct? y/n\n");
    if(getchar() != 'y') exit(0);
}

void log_init(int argc, char ** argv)
{
    char argstr[PATH_MAX];
    char logname[48];
    struct tm * timeinfo;
    time_t startrun = time(NULL);
    time_t rawtime;
    sprintf(logname, "test-%llu.log", (unsigned long long int) startrun);

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

void check_input_values(uint32_t seed, uint32_t iterations, uint64_t totsize, uint32_t bufsize, int iswritingtofiles)
{
	if(iswritingtofiles)
	{
		if(bufsize > (uint32_t) totsize)
		{
			printf("\nSingle file size is bigger than the total data size\n");
			exit(1);
		}
	}
	if(totsize < 4)
	{
		printf("\nTotal size should not be zero\n");
		exit(1);
	}
	if(seed == 0)
	{
		printf("\nseed value can't be zero\n");
		exit(1);
	}
}

long long unsigned read_device_size(char * path)
{
	char syspath[64];
	int slashpos = 0;
	int i = 0;
	FILE * fp;
	long long unsigned size = 0;
	// Find last / in path string
	for(; i < PATH_MAX; i++)
	{
		if(path[i] == '\0') break;
		if(path[i] == '/') slashpos = i;
	}

	// Strip device name
	path += slashpos;

	strcpy(syspath, "/sys/block");
	strcat(syspath, path);
	strcat(syspath, "/size");
	fp = fopen(syspath, "r");
	if(fp != NULL)
	{
		if(fscanf(fp, "%llu", &size) != 1)
		{
			printf("\nCannot read device size information from a system\n");
			exit(1);
		}
		fclose(fp);
		return (size * SYSFS_SIZE_BLOCKSIZE) ;
	}
	else
	{
		printf("\nCannot open %s\n", syspath);
		exit(1);
	}
	return 0;
}

