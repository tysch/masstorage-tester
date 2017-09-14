/*
 *  printprogress.h
 *
 */

#ifndef PRINTPROGRESS_H_
#define PRINTPROGRESS_H_
#include <stdint.h>
#include <stdio.h>

// Logging routine argument selection
enum printmode
{
    count,
    size,
    rspeed,
    wspeed,
    tbw,
    ioerror,
    mmerr,
    perc,
    readb,
    writeb,
    readp,
    writep,
    print,
    log,
    reset
};

void printprogress(enum printmode prflag , uint64_t val, FILE * logfile);

#endif /* PRINTPROGRESS_H_ */
