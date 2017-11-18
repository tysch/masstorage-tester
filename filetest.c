/*
 * filetest.c
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "constants.h"
#include "strconv.h"
#include "rng.h"
#include "print.h"
#include "fileio.h"
#include "init.h"
#include "options.h"

extern int stop_all;

void fillfiles(char * buf, struct options_s * options)
{
	char filename[PATH_LENGTH];
    char fileindir[PATH_LENGTH];
    char errmesg[128];

    uint64_t startspace = 0;
    uint64_t stopspace;
    double overhead;

    uint64_t nfiles = options->totsize / options->bufsize;
    time_t startrun = time(NULL);
    uint64_t byteswritten = 0;

    uint32_t ioerrors = 0;
    uint64_t totioerrors = 0;

    if(stop_all) return;

    reseed(options->seed);

    if(options->measure_fs_overhead) startspace = free_space_in_dir(options->path);

    create_dirs(options->path, nfiles, options->files_per_folder); // Create directory tree to contain all files for tested data

    for(uint64_t i = 0; i < nfiles; i++)
    {
        if(stop_all) break;

        fillbuf(buf, options->bufsize);

        path_append(options->path, fileindir, i, nfiles, options->files_per_folder);                   // Append path for additional folders
        sprintf(filename, "%s/%lli.jnk", fileindir, (long long) i);// Generate file name

        ioerrors = nofail_writefile(filename, buf, options->bufsize);

        byteswritten += options->bufsize - ioerrors;
        totioerrors += ioerrors;

        if(ioerrors)
        {
            sprintf(errmesg, "\n%lli.jnk write error\n", (long long) i);
            print(ERROR, errmesg);
        }

        printprogress(writeb, byteswritten);
        printprogress(mmerr, 0); //All data at write moment is considered correct
        printprogress(ioerror, totioerrors);
        printprogress(tbw, options->bufsize);
        if(time(NULL) - startrun)
            printprogress(wspeed, byteswritten / (time(NULL) - startrun));
        printprogress(show, 0);
        printprogress(perc, (uint64_t)(1000000.0*((double)(i + 1) / nfiles)));
    }

    if((byteswritten != 0) && options->measure_fs_overhead)
    {
    	// Skip overhead computation for the next runs
    	if(options->measure_fs_overhead == ONESHOT) options->measure_fs_overhead = 0;

    	stopspace = free_space_in_dir(options->path);
        overhead = ((double)(stopspace - startspace - byteswritten) / (double)byteswritten);
        sprintf(errmesg, "\nFilesystem overhead is %.3f %%", 100*overhead);
        print(OUT, errmesg);
    }

    printprogress(log, 0);
}

// Compares buffer data with a generated random values and counts read mismatches
// Returns error bytes count
uint32_t chkbuf_file(char * buf, uint32_t bufsize)
{
    uint32_t * ptr;
    uint32_t nerr = 0;

    for(uint32_t i = 0; i <  bufsize; i += sizeof(uint32_t))
    {
        ptr = (uint32_t *)(buf + i);

        if((*ptr) != xorshift128())
        {
            nerr += sizeof(uint32_t);
        }
    }
    return nerr;
}


void readfiles(char * buf, struct options_s * options)
{

    char filename[PATH_LENGTH];
    char fileindir[PATH_LENGTH];
    char errmesg[128];
    char errsize[24];

    uint64_t nfiles = options->totsize / options->bufsize;
    time_t startrun = time(NULL);
    uint64_t bytesread = 0;

    uint32_t ioerrors = 0;
    uint64_t totioerrors = 0;
    uint64_t memerrors = 0;

    reseed(options->seed);

    for(uint64_t i = 0; i < nfiles; i++)
    {
        if(stop_all) break;

        path_append(options->path, fileindir, i, nfiles, options->files_per_folder);              // Append path for additional folders
        sprintf(filename, "%s/%lli.jnk", options->path, (long long) i);// Generate file name

        ioerrors = nofail_readfile(filename, buf, options->bufsize, options->notdeletefiles);
        totioerrors += ioerrors;
        bytesread += options->bufsize - ioerrors;
        memerrors += chkbuf_file(buf, options->bufsize);

        if(ioerrors)
        {
            bytestostr(ioerrors, errsize);
            sprintf(errmesg, "\n--------------------file %-15lli.jnk is damaged, %-9s errors\n", (long long) i, errsize);
            print(OUT, errmesg);
        }

        printprogress(readb, bytesread);
        printprogress(perc, (uint64_t)(1000000.0*((double)(i + 1) / nfiles)));
        printprogress(mmerr, memerrors);
        printprogress(ioerror, totioerrors);
        if(time(NULL) - startrun)
            printprogress(rspeed, bytesread / (time(NULL) - startrun));
        printprogress(show, 0);
    }

    if(!(options->notdeletefiles)) delall(options->path);

    printprogress(log, 0);
}

