/*
 * devwrite.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include "rng.h"

// Opens file and reserve space for size and seed information
int device_init_write(char * path, FILE * logfile,  int islogging, uint32_t bufsize)
{
    int fd = open(path, O_WRONLY | O_SYNC | O_TRUNC);

    if(fd == -1)
    {
        printf("\nDevice access error!\n");
        if(islogging) fprintf(logfile, "\nDevice access error!\n");
        exit(1);
    }

    if(lseek(fd, bufsize, SEEK_SET) != bufsize )
    {
        printf("\ndevice seek failure!\n");
        if(islogging) fprintf(logfile, "\ndevice seek failure!");
    }
    return fd;
}

// Fills device with a random data
int32_t write_rand_block(char *buf, int fd, FILE * logfile , int islogging, uint32_t bufsize)
{
    int32_t ret;
    fillbuf(buf, bufsize);
    ret = write (fd, buf, bufsize);

    if (ret == -1)
    {
        if((errno == EINVAL) && (errno == EIO) && (errno == ENOSPC))
        {
            printf("\ndevice write failure\n");
            if(islogging) fprintf(logfile, "\ndevice write failure");
        }
    }
    return ret;
}

// Embeds seed and size information in buffer with (DISK_BUFFER/16)-modular redundancy
void writeseedandsize (char * buf, uint32_t seed, uint64_t size, uint32_t bufsize)
{
    // Data format:
    // |64-bit seed | 64-bit size |64-bit seed | 64-bit size |...
    uint64_t * ptr;

    for(uint32_t i = 0; i < bufsize; i += 2*sizeof(uint64_t))
    {
        ptr = (uint64_t *)(buf + i);
        *ptr = (uint64_t) seed;
        ptr = (uint64_t *)(buf + i + sizeof(uint64_t));
        *ptr = size;
    }
}
