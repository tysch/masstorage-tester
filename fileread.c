/*
 * fileread.c
 *
 */

#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include "constants.h"
#include "read.h"

int32_t readfile(uint64_t nfile, char * path, char * buf,  FILE * logfile , int islogging, uint32_t bufsize)
{
    char filename[PATH_MAX];
    int fd;
    int32_t ret;

    sprintf(filename, "%s/%lli.dat", path, nfile);
    fd = open(filename, O_RDONLY);

    if(fd == -1)
    {
        printf("\nfile %lli.dat access error!\n", nfile);
        if(islogging) fprintf(logfile, "\nfile %lli.dat access error!\n", nfile);
    }

    else
    {
        ret = fileread(fd, buf, bufsize, logfile, islogging);

        if(close(fd) == -1)
        {
            printf("\file %lli.dat is not closed properly!\n", nfile);
            if(islogging) fprintf(logfile, "\nfile %lli.dat is not closed properly!", nfile);
        }
    }
    return ret;
}
