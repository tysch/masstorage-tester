/*
 * tests.c
 */
#include <stdint.h>
#include "print.h"
#include "devtest.h"
#include "filetest.h"
#include "saveload.h"
#include "options.h"

extern int stop_cycling;
extern int stop_all;

void singlewrite_f(char * buf, struct options_s * options)
{
    printprogress(reset, 0);
    printprogress(writep, 0);
    print(ERRCNT_INIT, options->errcntmax);

    if(options->iswritingtofiles)
        fillfiles(buf, options);
    else
        filldevice(buf, options);

    if(stop_all) return;
        if(options->islogging) save(options);
}

void singleread_f(char * buf, struct options_s * options)
{
    printprogress(reset, 0);
    printprogress(readp, 0);
    print(ERRCNT_INIT, options->errcntmax);

    if(options->islogging) load(options);
    if(stop_all) return;

    if(options->iswritingtofiles)
        readfiles(buf, options);
    else
        readdevice(buf, options);
}

void cycle_f(char * buf, struct options_s * options)
{
    printprogress(reset, 0);
    print(ERRCNT_INIT, options->errcntmax);
    if(options->islogging)
    {
        load(options);
        options->seed++;
    }

    if(stop_all) return;
    do
    {
        if(stop_all) break;

        options->iterations--;

        if(options->per_run_errcntmax) print(ERRCNT_INIT, options->errcntmax);

        printprogress(writep, 0);

        if(options->iswritingtofiles)
            fillfiles(buf, options);
        else
            filldevice(buf, options);

        if(stop_all) break;
// TODO: automated tests with summary
        if(options->islogging) save(options);
        if(stop_all) break;

        printprogress(readp, 0);

        if(options->iswritingtofiles)
            readfiles(buf, options);
        else
            readdevice(buf, options);

        if(stop_all) break;

        printprogress(count, 0);
        options->seed++;
    }
    while ((!stop_cycling) && options->iterations);
}
