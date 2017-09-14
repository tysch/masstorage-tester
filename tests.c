/*
 * tests.c
 *
 */
#include "printprogress.h"
#include "rng.h"
#include "devtest.h"
#include "filetest.h"

extern int stop_all;
extern int stop_cycling;

void singlewrite_f(char * path, char * buf, uint32_t seed, FILE * logfile , int islogging,
             int iswritingtofiles, uint64_t totsize, uint32_t bufsize)
{
    reseed(seed);
    printprogress(writep, 0, logfile);
  //  if(iswritingtofiles)
  //  	fillfiles(path, buf, seed, logfile , islogging , totsize, bufsize);
  //  else
    	filldevice(path, buf, seed, logfile, islogging, bufsize);
}

void singleread_f(char * path, char * buf, uint32_t seed, FILE * logfile , int islogging, int isfectesting,
             int iswritingtofiles, uint64_t totsize, uint32_t bufsize)
{
    printprogress(readp, 0, logfile);
//    if(iswritingtofiles)
;//readfiles(path, buf, logfile, 0 , islogging, isfectesting, islogging , totsize, bufsize);
 //   else
    	readback(path, buf, logfile, 0 , islogging, isfectesting, bufsize);
}

void cycle_f(char * path, char * buf, uint32_t seed, uint32_t iterations, FILE * logfile , int islogging, int isfectesting,
             int iswritingtofiles, uint64_t totsize, uint32_t bufsize)
{
    uint64_t byteswritten;
    do
    {
        iterations--;
        reseed(seed);
        printprogress(writep, 0, logfile);
   //     if(iswritingtofiles)
  //      	fillfiles(path, buf, seed, logfile , islogging , totsize, bufsize);
  //      else
        	byteswritten = filldevice(path, buf, seed, logfile , islogging, bufsize);
        if(stop_all) break;

        printprogress(readp, 0, logfile);

  //      if(iswritingtofiles)
;//readfiles(path, buf, logfile, 0 , islogging, isfectesting, islogging , totsize, bufsize);
  //      else
        	readback(path, buf, logfile, 0 , islogging, isfectesting, bufsize);

        printprogress(count, 0, logfile);
        if(stop_all) break;

        seed++;
    }
    while ((!stop_cycling) && iterations);
}
