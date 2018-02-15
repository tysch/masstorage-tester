/*
 * nofailio.c
 */
#define _GNU_SOURCE

#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "errmesg.h"
#include "constants.h"
#include "print.h"

static char errmesg[PATH_LENGTH];
extern int stop_all;

void nofail_fsync(int fd)
{
    int err = fsync(fd);
    if(err == -1)
    {
        printerr("\nFile/device sync error:");
    }
}

int nofail_open(char * path)
{
    int fd = open64(path, O_RDWR | O_SYNC | O_CREAT | O_DIRECT, 0666);

    if(fd == -1) printerr("\nFile/device open error:");
    if((fd == -1) && (errno == EROFS))
    {
        print(ERROR, "\nCannot open file/device for reading and writing, falling back to read-only\n");
        fd = open64(path, O_RDONLY);
        if(fd == -1) printerr("\nFile/device open error:");
    }

    return fd;
}

int nofail_rd_open(char * path)
{
    int fd = open64(path, O_RDONLY | O_DIRECT);
    if(fd == -1) printerr("\nFile/device open error:");

    return fd;
}

void nofail_close(int fd)
{
    int ret = close(fd);
    if(ret == -1) printerr("\nFile/device close error:");
}

int nofail_unlink(char * path)
{
    int ret = unlink(path);
    if(ret == -1) printerr("\nFile remove error:");
    return ret;
}

// Error-tolerant pread() ; returns bytes lost by i/o errors and pads them with zeroes
uint32_t nofail_pread(int fd, char * buf, uint32_t bufsize, uint64_t offset)
{
    char * offtbuf;
    uint32_t bufpos;
    uint32_t remsize;

    int32_t res;
    uint64_t curr_pos;

    uint32_t padzero_start = 0;
    uint32_t padzero_end = 0;
    uint32_t skipbytes = SKIP_BYTES;

    uint32_t retries = MAX_RETRIES;

    uint32_t ioreaderrcnt = 0;
    int firsterr = 0;

    bufpos = 0;

    do
    {
    	if(stop_all) return 0;
    	curr_pos = offset + bufpos; // Absolute file position
        offtbuf = buf + bufpos;     // Pointer to the beginning of the unread buffer section
        remsize = bufsize - bufpos; // Remaining bytes to read

        res = pread64(fd, offtbuf, remsize, curr_pos);
        if(res == remsize) return ioreaderrcnt; // Nothing left to read

        if(res > 0)    // Something have been read successfully
        {
            retries = MAX_RETRIES;
        	bufpos += res;
            continue;  // Move to the next unread position
        }

        if(res <= 0)// Try to read more times if nothing have been read out
        {
            if(!firsterr)
            {
                print(ERROR, "\n");
                firsterr = 1;
                sprintf(errmesg, "\rWarning: nothing have been read at position %llu, retrying...",
                                                                     (unsigned long long) curr_pos);
                print(ERROR, errmesg);
            }

            while(retries > 0) // Try to read again if error is recoverable
            {
            	if(stop_all) return 0;
            	retries--;
                if((res == 0) || // No data read while no errors specified
                      (
                          (res == -1) &&  // Read was interrupted or may be recoverable
                          ((errno == EINTR) || (errno == EAGAIN) || (errno == EIO))
                      )
                  )
                {
                    if(errno == EIO) printerr("\nFile/device read error:"); // Warn about hardware error
                    res = pread64(fd, offtbuf, remsize, curr_pos);
                }
                else break; // Error is too serious to retry

                if(res > 0) break; // Something have been read out by retries;
            }
        }

        if(res > 0)   // Something have been read successfully
        {
            bufpos += res;
            continue; // Move to the next unread position
        }
        if(res <= 0) // Read failed even after retries
        {
            sprintf(errmesg, "\rError: unreadable block starting at %llu, skipping %i bytes",
                                                    (unsigned long long) curr_pos, skipbytes);
            print(ERROR, errmesg);

            printerr("\nFile/device read error:");  //Print error message

            // Pad next SKIP_BYTES with zeroes and move forward
            padzero_start = bufpos;
            padzero_end = bufpos + skipbytes;
            if(padzero_end > bufsize) padzero_end = bufsize;

            // Count padded bytes as a read errors
            ioreaderrcnt += padzero_end - padzero_start;

            for(int i = padzero_start; i < padzero_end; i++) buf[i] = 0;

            bufpos = padzero_end;

            skipbytes = skipbytes + skipbytes / SKIP_DIV;
            if(skipbytes > MAX_SKIP_BYTES ) skipbytes = MAX_SKIP_BYTES;
        }
    }
    while (bufpos != bufsize); // Buffer is full
    print(ERROR, "\n");
    return ioreaderrcnt;
}

// Error-tolerant pwrite() ; returns bytes that was failed to be written
uint32_t nofail_pwrite(int fd, char * buf, uint32_t bufsize, uint64_t offset)
{
    char * offtbuf;
    uint32_t bufpos;
    uint32_t remsize;

    int32_t res;
    uint64_t curr_pos;

    uint32_t skipbytes = SKIP_BYTES; //initial count of bytes to skip

    uint32_t retries = MAX_RETRIES;

    int32_t iowriteerrcnt = 0;
    int firsterr = 0;

    bufpos = 0;

    do
    {
    	if(stop_all) return 0;
    	curr_pos = offset + bufpos; // Absolute file position
        offtbuf = buf + bufpos;     // Pointer to the beginning of the unwritten buffer section
        remsize = bufsize - bufpos; // Remaining bytes to write

        res = pwrite64(fd, offtbuf, remsize, curr_pos);
        if(res == remsize) return (uint32_t) iowriteerrcnt; // Nothing left to be written

        if(res > 0)    // Something have been written successfully
        {
            retries = MAX_RETRIES;
        	bufpos += res;
            continue;  // Move to the next unwritten position
        }

        if(res <= 0)   // Try to write more times if nothing have been written
        {
            if(!firsterr)
            {
                print(ERROR, "\n");
                firsterr = 1;
                sprintf(errmesg, "\rWarning: nothing have been written at position %llu, retrying...",
                                                                        (unsigned long long) curr_pos);
                print(ERROR, errmesg);
            }

            while(retries > 0) // Try to write again if error is recoverable
            {
            	if(stop_all) return 0;
            	retries--;
                if(
                      (res == 0) ||       // No data written while no errors specified
                      (
                          (res == -1) &&  // Write was interrupted or may be recoverable
                          (
                                   (errno == EINTR) || (errno == EAGAIN) || (errno == EIO)
                          )
                      )
                  )
                {
                    if(errno == EIO) printerr("\nFile/device write error:");  // Warn about hardware error
                    res = pwrite64(fd, offtbuf, remsize, curr_pos);
                }
                else break; // Error is too serious to retry
                if(res > 0) break; // Something have been written by retries;
            }
        }

        if(res > 0)   // Something have been written successfully
        {
            bufpos += res;
            continue; // Move to the next unwritten position
        }

        if(res <= 0) // Write failed even after retries
        {
            sprintf(errmesg, "\rError: cannot write block starting at %llu, skipping %i bytes ",
                                                       (unsigned long long) curr_pos, skipbytes);
            print(ERROR, errmesg);
            printerr("\nFile/device write error:"); //Print error message

            // Skip next skipbytes and move forward

            iowriteerrcnt -= bufpos; // count i/o errors properly

            bufpos += skipbytes;
            if(bufpos > bufsize) bufpos = bufsize;

            iowriteerrcnt += bufpos;

            skipbytes = skipbytes + skipbytes / SKIP_DIV;
            if(skipbytes > MAX_SKIP_BYTES ) skipbytes = MAX_SKIP_BYTES; // To prevent overflow
        }
    }
    while (bufpos != bufsize); // Nothing left to write
    print(ERROR, "\n");
    return (uint32_t) iowriteerrcnt;
}
