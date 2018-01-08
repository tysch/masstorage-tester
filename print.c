#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "print.h"
#include "strconv.h"

extern int stop_all;

void print(enum print_mode action, const char * string)
{
    static FILE * logfile = NULL;
    static int islogging = 0;
    static long long errorcount = 0;
    static long long errcntmax = 0;
    if((errcntmax) &&(errorcount > errcntmax))
    {
        printf("\nToo many i/o errors happened, exiting now...\n");
        fflush(stdout);
        if(islogging)
        {
            fprintf(logfile, "\nToo many i/o errors happened, exiting now...\n");
            fflush(logfile);
        }
        // Stop test routine and gentle exit
        stop_all = 1;
    }

    switch (action)
    {
        case LOGFILE_INIT :
            logfile = fopen(string, "w");
            if(logfile == NULL)
            {
                printf("\nCannot create logfile\n");
                fflush(stdout);
                exit(EXIT_FAILURE);
            }
            islogging = 1;
            break;

        case ERRCNT_INIT :
            sscanf(string, "%lli", &errcntmax);
            break;

        case LOGFILE_EXIT :
            if(islogging) fclose(logfile);
            break;

        case OUT :
            printf("%s", string);
            fflush(stdout);
            if(islogging)
            {
                fprintf(logfile, "%s", string);
                fflush(logfile);
            }
            break;

        case ERROR :
            errorcount++;
            printf("%s", string);
            fflush(stdout);
            if(islogging) 
            {
                fprintf(logfile, "%s", string);
                fflush(logfile);
            }
            break;

        case LOG :
            if(islogging)
            {
                fprintf(logfile, "%s", string);
                fflush(logfile);
            }
            break;
    }
}

// Prints and logs status
// Replace scattered prints with a lots of global variables
void printprogress(enum print_val prflag , uint64_t val)
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
    static double percent;

    static char tbwstr[80];
    static char readstr[80];
    static char writestr[80];
    static char ioerrstr[80];
    static char mmerrstr[80];
    static char rspeedstr[80];
    static char wspeedstr[80];
    static char sizestr[80];
    static char datestr[80];
    static char logstr[1024];

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
            bytestostr(readspeed, rspeedstr);
            break;

        case wspeed:
            writespeed = val;
            bytestostr(writespeed, wspeedstr);
            break;

        case tbw :
            totalbyteswritten += val;
            bytestostr(totalbyteswritten, tbwstr);
            break;

        case ioerror :
           ioerrors = val;
            bytestostr(ioerrors, ioerrstr);
            break;

        case mmerr:
            mismatcherrors = val;
            bytestostr(mismatcherrors, mmerrstr);
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
                sprintf(logstr,
                "\nPassage = %-9i %-6.4f%%     write = %-12s %12s/s    TBW = %-12s   I/O errors = %-12s  data errors = %-12s time = %s",
                (int) passage,
                percent, writestr, wspeedstr, tbwstr, ioerrstr , mmerrstr,
                datestr);
                print(LOG,logstr);
            }
            if(status == readrun)
            {
                todate(time(NULL) - startrun, datestr);
                sprintf(logstr,
                "\nPassage = %-9i %-6.4f%%     read  = %-12s %12s/s    TBW = %-12s   I/O errors = %-12s  data errors = %-12s time = %s",
                (int) passage,
                percent, readstr, rspeedstr, tbwstr, ioerrstr , mmerrstr,
                datestr);
                print(LOG,logstr);
            }

            // Intentionally left no break statement
        case show:
            if(status == writerun)
            {
                todate(time(NULL) - startrun, datestr);
                printf("                    ");
                printf(
                "\rPassage = %-9i %-6.4f%%     write = %-12s %12s/s    TBW = %-12s   I/O errors = %-12s  data errors = %-12s time = %s",
                (int) passage,
                percent, writestr, wspeedstr, tbwstr, ioerrstr , mmerrstr,
                datestr);
            }
            if(status == readrun)
            {
                todate(time(NULL) - startrun, datestr);
                printf("                    ");
                printf(
                "\rPassage = %-9i %-6.4f%%     read  = %-12s %12s/s    TBW = %-12s   I/O errors = %-12s  data errors = %-12s time = %s",
                (int) passage,
                percent, readstr, rspeedstr, tbwstr, ioerrstr , mmerrstr,
                datestr);
            }

            fflush(stdout);
            break;
    }
}
