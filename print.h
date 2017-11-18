/*
 * print.h
 */

#ifndef PRINT_H_
#define PRINT_H_

#include <stdint.h>
#include <string.h>

enum print_mode
{
    LOGFILE_INIT,
    ERRCNT_INIT,
    LOGFILE_EXIT,
    OUT,
    ERROR,
    LOG
};

enum print_val
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
    show,
    log,
    reset,
};

void print(enum print_mode action, const char * string);

void printprogress(enum print_val prflag , uint64_t val);

#endif /* PRINT_H_ */
