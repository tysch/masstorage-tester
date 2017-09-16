/*
 * read.c
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

extern int stop_all;

// partial read tolerant code
uint32_t fileread(int fd, char * buf, uint32_t bufsize , FILE * logfile, int islogging)
{
    ssize_t ret;
    uint32_t bufread = 0;

    if(fd == -1)
    {
        printf("\nDevice access error!\n");
        if(islogging) fprintf(logfile, "\nDevice access error!\n");
        exit(1);
    }

    while (bufsize != 0 && (ret = read (fd, buf, bufsize)) != 0)
    {
        if(stop_all) break;

        if (ret == -1)
        {
            if (errno == EINTR) continue;
            printf("\nDevice read error!\n");
            if(islogging) fprintf(logfile, "\nDevice read error!\n");
            break;
        }

        bufsize -= ret;
        buf += ret;
        bufread += ret;
    }
    return bufread;
}
