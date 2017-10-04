/*
 * tests.c
 */
#include <stdint.h>
#include "print.h"
#include "devtest.h"
#include "filetest.h"
#include "saveload.h"

extern int stop_cycling;

void singlewrite_f(char * path, char * buf, uint32_t bufsize, uint64_t totsize, uint32_t seed, int iswritingtofiles)
{
    printprogress(reset, 0);
	printprogress(writep, 0);

	//load(&seed, &totsize, &bufsize);

    if(iswritingtofiles)
    	fillfiles(path, buf, seed, totsize, bufsize);
    else
    	filldevice(path, buf, bufsize, totsize, seed);

	save(seed, totsize, bufsize);
}

void singleread_f(char * path, char * buf, uint32_t bufsize, uint64_t totsize, uint32_t seed, int isfectesting, int iswritingtofiles)
{
    printprogress(reset, 0);
	printprogress(readp, 0);
	load(&seed, &totsize, &bufsize);

    if(iswritingtofiles)
    	readfiles(path, buf, seed, totsize, bufsize);
    else
    	readdevice(path, buf, bufsize, totsize, seed, isfectesting);

    //save(seed, totsize, bufsize);
}

void cycle_f(char * path, char * buf, uint32_t seed, uint32_t iterations, int isfectesting,
             int iswritingtofiles, uint64_t totsize, uint32_t bufsize)
{
    printprogress(reset, 0);
    load(&seed, &totsize, &bufsize);
	do
    {
        iterations--;

        printprogress(writep, 0);
        if(iswritingtofiles)
        	fillfiles(path, buf, seed, totsize, bufsize);
        else
        	filldevice(path, buf, bufsize, totsize, seed);

        save(seed, totsize, bufsize);

        printprogress(readp, 0);

        if(iswritingtofiles)
        	readfiles(path, buf, seed, totsize, bufsize);
        else
        	readdevice(path, buf, bufsize, totsize, seed, isfectesting);

        printprogress(count, 0);
        seed++;

        //save(seed, totsize, bufsize);
    }
    while ((!stop_cycling) && iterations);
}
