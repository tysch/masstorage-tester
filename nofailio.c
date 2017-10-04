/*
 * nofailio.c
 *
 */
#define _XOPEN_SOURCE 500
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include "errmesg.h"
#include "constants.h"
#include "print.h"
#include <errno.h>


static char errmesg[512];

void nofail_fsync(int fd)
{
	int err = fsync(fd);
	if(err == -1)
	{
		printfsyncerr();
	}
}

int nofail_open(char * path)
{
	int fd = open(path, O_RDWR | O_SYNC | O_CREAT, 0666);

	if(fd == -1) printopenerr();

	return fd;
}

void nofail_close(int fd)
{
	int ret = close(fd);

	if(ret == -1) printcloseerr();

}

void nofail_unlink(char * path)
{
	int ret = unlink(path);
	if(ret == -1) printunlinkerr();
}


// Error-tolerant pread() ; returns bytes lost by i/o errors and pads them with zeroes
uint32_t nofail_pread(int fd, char * buf, uint32_t bufsize, off_t offset)
{
	char * offtbuf;
	uint32_t bufpos;
	uint32_t remsize;

	int32_t res;
	off_t curr_pos;

	uint32_t padzero_start = 0;
	uint32_t padzero_end = 0;
	uint32_t skipbytes = SKIP_BYTES;

	uint32_t retries;

	uint32_t ioreaderrcnt = 0;
	int firsterr = 0;

	bufpos = 0;

	do
	{
		retries = MAX_RETRIES;
		curr_pos = offset + bufpos; // Absolute file position
		offtbuf = buf + bufpos;     // Pointer to the beginning of the unread buffer section
		remsize = bufsize - bufpos;	// Remaining bytes to read

		res = pread(fd, offtbuf, remsize, curr_pos);

		if(res == remsize) return ioreaderrcnt; // Nothing left to read

		if(res > 0)    // Something have been read successfully
		{
			bufpos += res;
			continue;  // Move to the next unread position
		}

		if(res <= 0)// Try to read more times if nothing have been read out
		{
			if(!firsterr) print(ERROR, "\n");
			firsterr = 1;
			sprintf(errmesg, "\rWarning: nothing have been read at position %llu, retrying...", (unsigned long long) curr_pos);
			print(ERROR, errmesg);

			while(retries) // Try to read again if error is recoverable
			{
				retries--;
				if(
			        (res == 0) || (                  // No data read while no errors specified
					                 (res == -1) &&  // Read was interrupted or may be recoverable
							         ((errno == EINTR) || (errno == EAGAIN) || (errno == EIO))
					              )
				  )
			    {
				    if(errno == EIO) printpreaderr(); // Warn about hardware error
				    res = pread(fd, offtbuf, remsize, curr_pos);
			    }

				if(res > 0) break; // Something have been read out by retries;
			}
		}

		if(res > 0)   // Something have been read successfully
		{
			bufpos += res;
			continue; // Move to the next unread position
		}

		if(res == -1) // Read failed even after retries
		{
			sprintf(errmesg, "\rError: unreadable block starting at %llu, skipping %i bytes", (unsigned long long) curr_pos, skipbytes);
			print(ERROR, errmesg);

			printpreaderr(); //Print error message

			// Pad next SKIP_BYTES with zeroes and move forward
			padzero_start = bufpos;
			padzero_end = bufpos + skipbytes;
			if(padzero_end > bufsize) padzero_end = bufsize;
			// Count padded bytes as a read errors
			ioreaderrcnt += - padzero_end - padzero_start;

			for(int i = padzero_start; i < padzero_end; i++) buf[i] = 0;

			bufpos = padzero_end;

			skipbytes = skipbytes + skipbytes / SKIP_DIV;
			if(skipbytes > (1 << 30)) skipbytes = 1 << 30; // To prevent overflow
		}
	}
	while (bufpos != bufsize); // Buffer is full
	print(ERROR, "\n");
	return ioreaderrcnt;
}

// Error-tolerant pwrite() ; returns bytes that was failed to be written
uint32_t nofail_pwrite(int fd, char * buf, uint32_t bufsize, off_t offset)
{
	char * offtbuf;
	uint32_t bufpos;
	uint32_t remsize;

	int32_t res;
	off_t curr_pos;

	uint32_t skipbytes = SKIP_BYTES; //initial count of bytes to skip

	uint32_t retries;

	int32_t iowriteerrcnt = 0;
	int firsterr = 0;

	bufpos = 0;

	do
	{
		retries = MAX_RETRIES;
		curr_pos = offset + bufpos; // Absolute file position
		offtbuf = buf + bufpos;     // Pointer to the beginning of the unwritten buffer section
		remsize = bufsize - bufpos;	// Remaining bytes to write

		res = pwrite(fd, offtbuf, remsize, curr_pos);
		if(res == remsize) return (uint32_t) iowriteerrcnt; // Nothing left to be written

		if(res > 0)    // Something have been written successfully
		{
			bufpos += res;
			continue;  // Move to the next unwritten position
		}

		if(res <= 0)   // Try to write more times if nothing have been written
		{
			if(!firsterr) print(ERROR, "\n");
			firsterr = 1;
			sprintf(errmesg, "\rWarning: nothing have been written at position %llu, retrying...", (unsigned long long) curr_pos);
			print(ERROR, errmesg);

			while(retries) // Try to write again if error is recoverable
			{
				retries--;
				if(
			        (res == 0) || (                  // No data written while no errors specified
					                 (res == -1) &&  // Write was interrupted or may be recoverable
							         ((errno == EINTR) || (errno == EAGAIN) || (errno == EIO))
					              )
				  )
			    {
				    if(errno == EIO) printpwriteerr(); // Warn about hardware error
				    res = pwrite(fd, offtbuf, remsize, curr_pos);
			    }
				if(res > 0) break; // Something have been written by retries;
			}
		}

		if(res > 0)   // Something have been written successfully
		{
			bufpos += res;
			continue; // Move to the next unwritten position
		}

		if(res == -1) // Write failed even after retries
		{
			sprintf(errmesg, "\rError: cannot write block starting at %llu, skipping %i bytes ", (unsigned long long) curr_pos, skipbytes);
			print(ERROR, errmesg);

			printpwriteerr(); //Print error message

			// Skip next skipbytes and move forward

			iowriteerrcnt -= bufpos; // count i/o errors properly

			bufpos += skipbytes;
			if(bufpos > bufsize) bufpos = bufsize;

			iowriteerrcnt += bufpos;

			skipbytes = skipbytes + skipbytes / SKIP_DIV;
			if(skipbytes > (1 << 30)) skipbytes = 1 << 30; // To prevent overflow
		}
	}
	while (bufpos != bufsize); // Nothing left to write
	print(ERROR, "\n");
	return (uint32_t) iowriteerrcnt;
}
