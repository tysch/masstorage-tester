/*
 * printprogress.c
 *
 */


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "printprogress.h"
#include "strconv.h"


// Prints and logs status
// Replace scattered prints with a lots of global variables
void printprogress(enum printmode prflag , uint64_t val, FILE * logfile)
{
    static char status;
    static uint64_t totalbyteswritten;
    static uint64_t mismatcherrors;
    static uint64_t ioerrors;
    static uint32_t passage;
    static uint64_t writespeed;
    static uint64_t readspeed;
    static uint64_t readbytes;
    static uint64_t writebytes;
    static uint64_t sizewritten;
    static time_t startrun;
    static time_t elapsed;
    static double percent;

    static char tbwstr[20];
    static char readstr[20];
    static char writestr[20];
    static char ioerrstr[20];
    static char mmerrstr[20];
    static char rspeedstr[20];
    static char wspeedstr[20];
    static char sizestr[20];
    static char datestr[20];

    const int readrun = 1;
    const int writerun = 2;

    switch(prflag)
    {
        case count:
            passage++;
            break;

        case size:
            sizewritten = val;
            bytestostr(sizewritten, sizestr);
            printf("\nactual device size is %s", sizestr);
            break;

        case rspeed:
            readspeed = val;
            bytestostr(readspeed,rspeedstr);
            break;

        case wspeed:
            writespeed = val;
            bytestostr(writespeed,wspeedstr);
            break;

        case tbw :
            totalbyteswritten += val;
            bytestostr(totalbyteswritten,tbwstr);
            break;

        case ioerror :
            totalbyteswritten += val;
            bytestostr(totalbyteswritten,tbwstr);
            break;

        case mmerr:
            mismatcherrors += val;
            bytestostr(mismatcherrors,mmerrstr);
            break;

        case perc:
            percent = val;
            percent /= 10000.0;
            break;

        case readb:
            readbytes = val;
            bytestostr(readbytes,readstr);
            break;

        case writeb:
            writebytes = val;
            bytestostr(writebytes, writestr);
            break;

        case readp:
            status = readrun;
            printf("\n");
            break;

        case writep:
            status = writerun;
            printf("\n");
            break;

        case reset:
            passage = 1;
            sizewritten = 0;
            bytestostr(sizewritten, sizestr);
            readspeed = 0;
            bytestostr(readspeed,rspeedstr);
            writespeed = 0;
            bytestostr(writespeed,wspeedstr);
            totalbyteswritten = 0;
            bytestostr(totalbyteswritten,tbwstr);
            readbytes = 0;
            bytestostr(readbytes,readstr);
            writebytes = 0;
            bytestostr(writebytes, writestr);
            ioerrors = 0;
            bytestostr(ioerrors, ioerrstr);
            mismatcherrors = 0;
            bytestostr(mismatcherrors,mmerrstr);
            percent = 0;
            startrun = time(NULL);
            break;

        case log:
            if(status == writerun)
            {
                todate(time(NULL) - startrun, datestr);
                fprintf(logfile,
                "\nPassage = %-9i%-3.3f%%     write = %-12s %12s/s    TBW = %-12s   I/O errors = %-12s  data errors = %-12s time = %s",
                (int) passage,
                percent, writestr, wspeedstr, tbwstr, ioerrstr , mmerrstr,
                datestr);
            }
            if(status == readrun)
            {
                todate(time(NULL) - startrun, datestr);
                fprintf(logfile,
                "\nPassage = %-9i%-3.3f%%     read  = %-12s %12s/s    TBW = %-12s   I/O errors = %-12s  data errors = %-12s time = %s",
                (int) passage,
                percent, readstr, rspeedstr, tbwstr, ioerrstr , mmerrstr,
                datestr);
            }

            // Intentionally left no break statement
        case print:
            if(status == writerun)
            {
                todate(time(NULL) - startrun, datestr);
                printf("                    ");
                printf(
                "\rPassage = %-9i%-3.3f%%     write = %-12s %12s/s    TBW = %-12s   I/O errors = %-12s  data errors = %-12s time = %s",
                (int) passage,
                percent, writestr, wspeedstr, tbwstr, ioerrstr , mmerrstr,
                datestr);
            }
            if(status == readrun)
            {
                todate(time(NULL) - startrun, datestr);
                printf("                    ");
                printf(
                "\rPassage = %-9i%-3.3f%%     read  = %-12s %12s/s    TBW = %-12s   I/O errors = %-12s  data errors = %-12s time = %s",
                (int) passage,
                percent, readstr, rspeedstr, tbwstr, ioerrstr , mmerrstr,
                datestr);
            }

            fflush(stdout);
            break;
    }
}
