
// Verbose comments for open(), close(), pread(), pwrite(), fsync() and unlink() failures

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

// Verbose comments for fsync() errors
void printfsyncerr(void)
{
    char * errstr;
    if(errnochanged())
    {
        switch (errno)
        {
            case EBADF:
                errstr = "\nNot a file descriptor opened for writing.\n";
                break;

            case EINVAL:
                errstr = "\nThe implementation does not support synchronized I/O for this file.\n";
                break;

            case EIO:
                errstr = "\nA low-level I/O error occurred.\n";
                break;
            default:
                errstr = "\nUnknown error";
                printf(": %i\n\n", errno);
                break;
        }

        print(ERROR, "\nFile/device sync error:");
        print(ERROR, errstr);
    }
}

// Verbose comments for open() errors
void printopenerr(void)
{
    char * errstr;
    if(errnochanged())
    {
        switch (errno)
        {
            case EACCES:
                errstr = "\nSearch permission is denied on a component of the path prefix,\n\
or the file exists and the permissions specified by oflag are denied,\n\
or the file does not exist and write permission is denied for the parent directory\
of the file to be created,\n\
or O_TRUNC is specified and write permission is denied.\n";
            break;

            case EEXIST:
                errstr = "\nO_CREAT and O_EXCL are set, and the named file exists.\n";
                break;

            case EINTR:
                errstr = "\nA signal was caught during open().\n";
                break;

            case EINVAL:
                errstr = "\nThe implementation does not support synchronized I/O for this file, \
or the value of the oflag argument is not valid.\n";
                break;

            case EIO:
                errstr = "\nA low-level I/O error occured.\n";
                break;

            case EISDIR:
                errstr = "\nThe named file is a directory and oflag includes O_WRONLY or O_RDWR,\
or includes O_CREAT without O_DIRECTORY.\n";
                break;

            case ELOOP:
                errstr = "\nMore than {SYMLOOP_MAX} symbolic links were encountered during resolution\
of the path argument,\na loop exists in symbolic links encountered during\
resolution of the path argument,\nor O_NOFOLLOW was specified and the path argument\
names a symbolic link.\n";
                break;

            case EMFILE:
                errstr = "\nAll file descriptors available to the process are currently open.\n";
                break;

            case ENAMETOOLONG:
                errstr = "\nThe length of a pathname exceeds {PATH_MAX}, or pathname resolution of a symbolic link\n\
produced an intermediate result with a length that exceeds {PATH_MAX}.\n";
                break;

            case ENFILE:
                errstr = "\nThe maximum allowable number of files is currently open in the system.\n";
                break;

            case ENOENT:
                errstr = "\nO_CREAT is not set and a component of path does not name an existing file, \n\
O_CREAT is set and a component of the path prefix of path does not name an existing \
file,\nor path points to an empty string.\n";
                break;

            case ENOTDIR:
                errstr = "\nA component of the path prefix names an existing file that is neither a directory \
nor a symbolic link to a directory,\nor O_CREAT and O_EXCL are not specified,\
the path argument contains at least one non-/ character and ends with\n\
one or more trailing / characters, and the last pathname component names an\
existing file that is neither a directory\nnor a symbolic link to a directory, nor \
O_DIRECTORY was specified and the path argument resolves to a non-directory file.\n";
                break;

            case ENOSR:
                errstr = "\nThe path argument names a STREAMS-based file and the system is unable\
to allocate a STREAM.\n";
                break;

            case ENOSPC:
                errstr = "\nThe directory or file system that would contain the new file cannot be expanded,\n\
the file does not exist, and O_CREAT is specified.\n";
                break;

            case ENXIO:
                errstr = "\nThe named file is a character special or block special file,\n\
and the device associated with this special file does not exist.\n";
                break;

            case EOVERFLOW:
                errstr = "\nThe named file is a regular file and the size of the file cannot be represented\
correctly in an object of type off_t.\n";
                break;

            case EROFS:
                errstr = "\nThe named file resides on a read-only file system and either O_WRONLY, O_RDWR,\
O_CREAT (if the file does not exist),\nor O_TRUNC is set in the oflag argument.\n";
                break;

            case EAGAIN:
                errstr = "\nThe path argument names the slave side of a pseudoterminal device that is locked.\n";
                break;

            case ENOMEM:
                errstr = "\nThe path argument names a STREAMS file and the system is unable\
to allocate resources.\n";
                break;

            case EOPNOTSUPP:
                errstr = "\nThe path argument names a socket.\n";
                break;

            case ETXTBSY:
                errstr = "\nThe file is a pure procedure (shared text) file that is being executed\
and oflag is O_WRONLY or O_RDWR.\n";
                break;
            default:
                errstr = "\nUnknown error";
                printf(": %i\n\n", errno);
                break;
        }

        print(ERROR, "\nFile/device open error:");
        print(ERROR, errstr);
    }
}

// Verbose comments for close() errors
void printcloseerr(void)
{
    char * errstr;
    if(errnochanged())
    {
        switch (errno)
        {
            case EBADF:
                errstr = "\nThe fildes argument is not a open file descriptor.\n";
                break;

            case EINTR:
                errstr = "\nThe close() function was interrupted by a signal.\n";
                break;

            case EIO:
                errstr = "\nAn I/O error occurred while reading from or writing to the file system.\n";
                break;
            default:
                errstr = "\nUnknown error";
                printf(": %i\n\n", errno);
                break;
        }

        print(ERROR, "\nFile/device close error:");
        print(ERROR, errstr);
    }
}

// Verbose comments for pread() errors
void printpreaderr(void)
{
    char * errstr;
    if(errnochanged())
    {
    	switch (errno)
        {
            case EAGAIN:
                errstr = "\nThe O_NONBLOCK flag is set for the file descriptor, and the thread would be delayed \
in the read operation.\n";
                break;

            case EBADF:
                errstr = "\nThe fildes argument is not a valid file descriptor open for reading.\n";
                break;

            case EINTR:
                errstr = "\nThe read operation was terminated due to the receipt of a signal,\
and no data was transferred.\n";
                break;

            case EINVAL:
                errstr = "\nThe file is a regular file or block special file, and the offset argument\
is negative.\n";
                break;

            case EIO:
                errstr = "\nA physical I/O error has occurred.\nThis error may also be generated\
for implementation-defined reasons.\n";
                break;

            case EISDIR:
                errstr = "\nThe fildes argument refers to a directory and the implementation does not allow the\
directory to be read using pread().\nThe readdir() function should be used instead.\n";
                break;

            case EOVERFLOW:
                errstr = "\nThe file is a regular file, nbyte is greater than 0, the starting position is before\
the end-of-file, and the starting position\n is greater than or equal to the offset maximum established in the open file \
description\nassociated with fildes.\n";
                break;

            case ESPIPE:
                errstr = "\nThe file is incapable of seeking.\n";
                break;

            case ETIMEDOUT:
                errstr = "\nA read was attempted on a socket and a transmission timeout occurred.\n";
                break;

            case ENOBUFS:
                errstr = "\nInsufficient resources were available in the system to perform the operation.\n";
                break;

            case ENOMEM:
                errstr = "\nInsufficient memory was available to fulfill the request.\n";
                break;

            case ENXIO:
                errstr = "\nA request was made of a nonexistent device,\n\
or the request was outside the capabilities of the device.\n";
               break;
            default:
                errstr = "\nUnknown error";
                printf(": %i\n\n", errno);
                break;
        }

    	print(ERROR, "\nFile/device read error:");
    	print(ERROR, errstr);
    }
}

void printpwriteerr(void)
{
    char * errstr;
    if(errnochanged())
    {
    	switch (errno)
        {
            case EAGAIN:
                errstr = "\nThe O_NONBLOCK flag is set for the file descriptor, and the thread\
would be delayed in the write() operation.\n";
                break;

            case EBADF:
                errstr = "\nThe fildes argument is not a valid file descriptor open for writing.\n";
                break;

            case EFBIG:
                errstr = "\nAn attempt was made to write a file that exceeds the implementation-defined maximum \
file size,\nor the file size limit of the process, and there was no room for any bytes to be written,\n\
or the file is a regular file, nbyte is greater than 0, and the starting position is greater than or equal \n\
to the offset maximum established in the open file descriptor associated with fildes.\n";
                break;

            case EINTR:
                errstr = "\nThe write operation was terminated due to the receipt of a signal,\
and no data was transferred.\n";
                break;

            case EIO:
                errstr = "\nA physical I/O error has occurred.\nThis error may also be returned \
under implementation-defined conditions.\n";
                break;

            case ENOSPC:
                errstr = "\nThere was no free space remaining on the device containing the file.\n";
                break;

            case ERANGE:
                errstr = "\nThe transfer request size was outside the range supported by the STREAMS file \
associated with fildes.\n";
                break;

            case EINVAL:
                errstr = "\nThe file is a regular file or block special file, and the offset argument is negative.\n";
                break;

            case ENOBUFS:
                errstr = "\nInsufficient resources were available in the system to perform the operation.\n";
                break;

            case ENXIO:
                errstr = "\nA request was made of a nonexistent device, or the request was outside the capabilities of the device.\n";
                break;

            case ESPIPE:
                errstr = "\nThe file is incapable of seeking.\n";
                break;
            default:
                errstr = "\nUnknown error";
                printf(": %i\n\n", errno);
                break;
        }

    	print(ERROR, "\nFile/device write error:");
    	print(ERROR, errstr);
    }
}

void printunlinkerr(void)
{
    char * errstr;
    if(errnochanged())
    {
        switch (errno)
        {
            case EACCES:
                errstr = "\nSearch permission is denied for a component of the path prefix, or write permission is denied \
on the directory containing the directory entry to be removed.\n";
                break;

            case EFAULT:
                errstr = "\nPathname is an invalid pointer.\n";
                break;

            case EIO:
                errstr = "\nA physical I/O error has occurred.\n";
                break;

            case EROFS:
                errstr = "\nPathname resides on a read-only filesystem.\n";
                break;

            case EPERM:
                errstr = "\nThe system does not allow the unlinking of files, or the file named by path is a directory,\n\
and either the calling process does not have appropriate privileges, or the implementation prohibits using unlink() on directories.\n";
                break;

            case ENOTDIR:
                errstr = "\nA component in pathname is not a directory.\n";
                break;

            case ENOMEM:
                errstr = "\nThere is insufficient memory available to complete the request.\n";
                break;

            case ENOENT:
                errstr = "\nA component in pathname does not exist.\n";
                break;

            case ENAMETOOLONG:
                errstr = "\nPathname is too long.\n";
                break;

            case ELOOP:
                errstr = "\nToo many symbolic links or a loop were encountered in traversing pathname.\n";
                break;

            case EISDIR:
                errstr = "\nPathname refers to a directory.\n";
                break;

            case EBUSY:
                errstr = "\nThe file named by the path argument cannot be unlinked because it is being used by the system or another \
process and the implementation considers this an error.\n";
                break;
            default:
                errstr = "\nUnknown error";
                printf(": %i\n\n", errno);
                break;
        }

        print(ERROR, "\nFile removing error:");
        print(ERROR, errstr);
    }
}
