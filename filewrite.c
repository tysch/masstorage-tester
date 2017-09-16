/*
 * filewrite.c
 *
 */
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include "constants.h"

void writefile(uint64_t nfile, char * path, char * buf,  FILE * logfile , int islogging, uint32_t bufsize)
{
    char filename[PATH_MAX];
    int fd;
    int32_t ret;

    sprintf(filename, "%s/%lli.dat", path, nfile);
    fd = open(filename, O_WRONLY | O_SYNC | O_TRUNC | O_CREAT, 0666);

    if(fd == -1)
    {
        printf("\nfile %lli.dat access error!\n", nfile);
        if(islogging) fprintf(logfile, "\nfile %lli.dat access error!\n", nfile);
    }
    else
    {
        ret = write (fd, buf, bufsize);

        if ((ret == -1))
        {
            printf("\nfile %lli.dat write failure\n", nfile);
            if(islogging) fprintf(logfile, "\nfile %lli.dat write failure", nfile);
        }

        if ((ret < bufsize))
        {
            printf("\nfile %lli.dat write incomplete\n", nfile);
            if(islogging) fprintf(logfile, "\nfile write incomplete", nfile);
        }

        if(close(fd) == -1)
        {
            printf("\file %lli.dat is not closed properly!\n", nfile);
            if(islogging) fprintf(logfile, "\nfile %lli.dat is not closed properly!", nfile);
        }
    }
}
