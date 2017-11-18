// Verbose prints for system call errors

#include <stdio.h>
#include <errno.h>
#include "print.h"

// Skips messages in case of repeated errors
int errnochanged(void)
{
    static int preverrno = 0;
    if(preverrno != errno)
    {
        preverrno = errno;
        return 1;
    }
    return 0;
}

void printerr(char * descript)
{
	static char descr_prev[1024] = "";
	//
	if(
	    !(strcmp(descript, descr_prev) == 0) // Error happened in the same function
	    || errnochanged())                   // Type of error changed
	{
    	strcpy(descr_prev, descript);
		print(ERROR, descript);
    	print(ERROR, strerror(errno));
	}
}
