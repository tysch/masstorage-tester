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
        printf("\n\nUsage: -d <path> [-s  <total size[kKmMgGt]>] [-o|r|w|c <iterations>] [-i <salt>][--fec] [-f <file size[kKmMgGt]> [-p]]\n");
        printf("\n -d     -- Path to test device or file.");
        printf("\n -s     -- Total bytes count to be written, rounded down to the size of a buffer.");
        printf("\n           If not set, massstoragetester would fill entire device or all free space in target location.");
        printf("\n           Please note that files may have filesystem-specific storage overhead and automatic free space estimation");
        printf("\n           while setting file size too small may cause overflow. It shoudn't be considered as a bug.");
        printf("\n");
        printf("\n -o     -- Single read/write cycle, for speed and volume measurement, default.");
        printf("\n -c     -- <iterations> read/write cycles, for endurance tests.");
        printf("\n -w     -- Single write only.");
        printf("\n -r     -- Single read only.");
        printf("\n           -w, -r are useful for long term data retention tests.");
        printf("\n           -w and -r must be launched with the same salt.");
        printf("\n");
        printf("\n -f     -- Write to a bunch of files instead of device with defined size.");
        printf("\n           Single file should be less than 2 GiB and no more than total data size.");
        printf("\n");
        printf("\n -i     -- Non-zero integer salt for random data being written, default 1.");
        printf("\n --fec  -- Estimates Reed-Solomon forward error correction code requirement for raw device.");
        printf(" for GF=256,\n           spare blocks count vs block size.");
        printf("\n           Total file size can be rounded down to a more optimal values.");
        printf("\n -p     -- Do not clean up files while reading back in in file test mode.");
        printf("\n           Useful for long-term storage reliability testing.");
        printf("\n -h     -- Skip all confirmations and start a new test in background.");
        printf("\n\n.");
        exit(1);
    }
}

void parse_cmd_val(int argc, char ** argv, char * path, uint32_t * seed , uint32_t * iterations, int *isfectesting,
                   int *iswritingtofiles, int *notdeletefiles, int * headless, uint64_t * totsize, uint32_t * filesize)
{
    uint32_t filesiz;

    for(int i = 0; i < argc; i++)
    {
        if(strcmp(argv[i], "-d") == 0)
        {
            if(i + 1 == argc) exit(1);
            else
                strcpy(path, argv[i + 1]);
        }

        if(strcmp(argv[i], "-i") == 0)
        {
            if((i + 1 == argc)) exit(1);
            else
                sscanf(argv[i + 1], "%i", seed);
        }

        if(strcmp(argv[i], "-c") == 0)
        {
            if((i + 1 == argc)) exit(1);
            else
                sscanf(argv[i + 1], "%i", iterations);
        }

        if(strcmp(argv[i], "-f") == 0)
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

        if(strcmp(argv[i], "-s") == 0)
        {
            if((i + 1 == argc)) exit(1);
            else
            {
                *totsize = tobytes(argv[i + 1]);
            }
        }

        if(strcmp(argv[i], "--fec") == 0) *isfectesting = 1;

        if(strcmp(argv[i], "-p") == 0) *notdeletefiles = 1;
        if(strcmp(argv[i], "-h") == 0) *headless = 1;
    }

	if(strcmp(path, "-") == 0)
	{
        printf("\n No directory or device specified, exiting now\n");
        fflush(stdout);
        exit(1);
	}

	if(((path[0] != '/') && (*isfectesting == 1)))
	{
        printf("\nBackground runs require absolute path to the file/device.\n");
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

void print_erasure_warning(char * path, uint64_t size)
{
    char str[PATH_MAX];
    char sizestr[32];
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
    if(res != 'y') exit(0);
    else printf("\nSearching for a previous run...\n");
    fflush(stdout);
}

void log_init(int argc, char ** argv)
{
    static char argstr[PATH_MAX];
    static char logname[96];
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
    static char syspath[128];
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

uint64_t free_space_in_dir(char * path)
{
    uint64_t freesize = 0;
    uint64_t blocksize;
    uint64_t freeblocks;

    struct statvfs buf;
    char * errmesg;

    int ret = statvfs(path, &buf);
    if(ret == -1)
    {
        switch (errno)
        {
            case EACCES:
                errmesg = "\nSearch permission is denied for a component of the path prefix of path\n";
                break;
            case EFAULT:
                errmesg = "\nPath points to an invalid address.\n";
                break;
            case EINTR:
                errmesg = "\nThis call was interrupted by a signal.\n";
                break;
            case EIO:
                errmesg = "\nAn I/O error occurred while reading from the file system.\n";
                break;
            case ELOOP:
                errmesg = "\nToo many symbolic links were encountered in translating path.\n";
                break;
            case ENAMETOOLONG:
                errmesg = "\nPath is too long.\n";
                break;
            case ENOENT:
                errmesg = "\nThe file referred to by path does not exist.\n";
                break;
            case ENOMEM:
                errmesg = "\nInsufficient kernel memory was available.\n";
                break;
            case ENOSYS:
                errmesg = "\nThe file system does not support this call.\n";
                break;
            case ENOTDIR:
                errmesg = "\nA component of the path prefix of path is not a directory.\n";
                break;
            case EOVERFLOW:
                errmesg = "\nSome values were too large to be represented in the returned struct.\n";
                break;
            default:
                errmesg = "\nUnknown error.\n";
        }

        printf("\nUnable to determine remaining free space:");
        printf("%s", errmesg);
        exit(1);
    }

    blocksize = buf.f_bsize;
    freeblocks = buf.f_bavail;
    freesize = blocksize * freeblocks;

    return freesize;
}

void print_folder_size(uint64_t totsize, uint32_t bufsize)
{
    static char sizestr[32];
    totsize -= totsize % bufsize;
    bytestostr(totsize, sizestr);
    printf("\nTesting %s free space\n", sizestr);
}

void make_daemon(void)
{
	pid_t pid;
	// create new process
	pid = fork ( );
	if(pid == -1)
	{
		printf("\nFauled to create a daemon process\n");
		exit(1);
	}
	else if(pid != 0)
	{
			printf("\nGoint to the background\n");
			exit(EXIT_SUCCESS);
	}

	// create new session and process group
	if (setsid() == -1) exit(1);

	// set the working directory to the root directory
	if (chdir ("/") == -1) exit(1);

	// close stdin, stdout and stderr
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	// redirect fd's 0,1,2 to /dev/null
	open ("/dev/null", O_RDWR);
	open ("/dev/null", O_RDWR);
	open ("/dev/null", O_RDWR);
}
