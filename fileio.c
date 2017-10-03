#include <stdint.h>
#include <stdio.h>
#include "print.h"
#include "constants.h"
#include "nofailio.h"

// Create and write buf to file; returns number of i/o errors
uint32_t nofail_writefile(char * path, char * buf, uint32_t bufsize)
{
	uint32_t ret;
	char errstr[PATH_MAX + 20];

	int fd = nofail_open(path);

    if(fd == -1)
    {
        sprintf(errstr, "%s access error!\n", path);
    	print(ERROR, errstr);
    	return bufsize;
    }
    else
    {
        ret = nofail_pwrite(fd, buf, bufsize, 0);
        nofail_fsync(fd);
        nofail_close(fd);
    }
    return ret;
}

// Read and delete file; returns number of i/o errors
uint32_t nofail_readfile(char * path, char * buf, uint32_t bufsize)
{
    int fd;
    uint32_t ret;
    char errstr[PATH_MAX + 20];

    fd = nofail_open(path);

    if(fd == -1)
    {
        sprintf(errstr, "%s access error!\n", path);
    	print(ERROR, errstr);
    	return bufsize;
    }
    else
    {
        ret = nofail_pread(fd, buf, bufsize, 0);
        nofail_close(fd);
        nofail_unlink(path);
    }
    return ret;
}
