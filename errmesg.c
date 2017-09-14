/*
 * errmesg.c
 *
 */

#include <stdio.h>
#include <errno.h>

void printopenerr(FILE * logfile, int islogging)
{
	char * errstr;
    printf("\nFile/device open error:", errstr);
    if(islogging)  fprintf(logfile, "%s", errstr);
	switch (errno)
	{
        case EACCES:
            errstr = "\nSearch permission is denied on a component of the path prefix,\n \
            		  or the file exists and the permissions specified by oflag are denied,\n \
            		  or the file does not exist and write permission is denied for the parent directory of the file to be created,\n \
            		  or O_TRUNC is specified and write permission is denied.\n";
            break;
        case EEXIST:
            errstr = "\nO_CREAT and O_EXCL are set, and the named file exists.\n";
            break;
        case EINTR:
            errstr = "\nA signal was caught during open().\n";
            break;
        case EINVAL:
            errstr = "\nThe implementation does not support synchronized I/O for this file.\n";
            break;
        case EIO:
            errstr = "\nThe path argument names a STREAMS file and a hangup or error occurred during the open().\n";
            break;
        case EISDIR:
            errstr = "\nThe named file is a directory and oflag includes O_WRONLY or O_RDWR, or includes O_CREAT without O_DIRECTORY.\n";
            break;
        case ELOOP:
            errstr = "\nA loop exists in symbolic links encountered during resolution of the path argument, or O_NOFOLLOW was specified and the path argument names a symbolic link.\n";
            break;
        case EMFILE:
            errstr = "\nAll file descriptors available to the process are currently open.\n";
            break;
        case ENAMETOOLONG:
            errstr = "\nThe length of a component of a pathname is longer than {NAME_MAX}.\n";
            break;
        case ENFILE:
            errstr = "\nThe maximum allowable number of files is currently open in the system.\n";
            break;
        case ENOENT:
            errstr = "\nO_CREAT is not set and a component of path does not name an existing file, or O_CREAT is set and a component of the path prefix of path does not name an existing file, or path points to an empty string.\n";
            break;
        case ENOTDIR:
            errstr = "\nO_CREAT is set, and the path argument contains at least one non- <slash> character and ends with one or more trailing <slash> characters. If path without the trailing <slash> characters would name an existing file, an [ENOENT] error shall not occur.\n";
            break;
        case ENOSR:
            errstr = "\nThe path argument names a STREAMS-based file and the system is unable to allocate a STREAM.\n";
            break;
        case ENOSPC:
            errstr = "\nThe directory or file system that would contain the new file cannot be expanded, the file does not exist, and O_CREAT is specified.\n";
            break;
        case ENOTDIR:
            errstr = "\nA component of the path prefix names an existing file that is neither a directory nor a symbolic link to a directory; or O_CREAT and O_EXCL are not specified, the path argument contains at least one non- <slash> character and ends with one or more trailing <slash> characters, and the last pathname component names an existing file that is neither a directory nor a symbolic link to a directory; or O_DIRECTORY was specified and the path argument resolves to a non-directory file.\n";
            break;
        case ENXIO:
            errstr = "\nO_NONBLOCK is set, the named file is a FIFO, O_WRONLY is set, and no process has the file open for reading.\n";
            break;
        case ENXIO:
            errstr = "\nThe named file is a character special or block special file, and the device associated with this special file does not exist.\n";
            break;
        case EOVERFLOW:
            errstr = "\nThe named file is a regular file and the size of the file cannot be represented correctly in an object of type off_t.\n";
            break;
        case EROFS:
            errstr = "\nThe named file resides on a read-only file system and either O_WRONLY, O_RDWR, O_CREAT (if the file does not exist), or O_TRUNC is set in the oflag argument.\n";
            break;
        case EAGAIN:
            errstr = "\nThe path argument names the slave side of a pseudo-terminal device that is locked.\n";
            break;
        case EINVAL:
            errstr = "\nThe value of the oflag argument is not valid.\n";
            break;
        case ELOOP:
            errstr = "\nMore than {SYMLOOP_MAX} symbolic links were encountered during resolution of the path argument.\n";
            break;
        case ENAMETOOLONG:
            errstr = "\nThe length of a pathname exceeds {PATH_MAX}, or pathname resolution of a symbolic link produced an intermediate result with a length that exceeds {PATH_MAX}.\n";
            break;
        case ENOMEM:
            errstr = "\nThe path argument names a STREAMS file and the system is unable to allocate resources.\n";
            break;
        case EOPNOTSUPP:
            errstr = "\nThe path argument names a socket.\n";
            break;
        case ETXTBSY:
            errstr = "\nThe file is a pure procedure (shared text) file that is being executed and oflag is O_WRONLY or O_RDWR.\n";
            break;
	}
    printf("%s", errstr);
    if(islogging)  fprintf(logfile, "%s", errstr);
}

void printcloseerr(FILE * logfile, int islogging)
{
	char * errstr;
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
	}
	printf("%s", errstr);
    if(islogging)  fprintf(logfile, "%s", errstr);
}

void printlseekerr(FILE * logfile, int islogging)
{
	char * errstr;
	switch (errno)
	{
        case EBADF:
            errstr = "\nThe fildes argument is not an open file descriptor.\n";
            break;
        case EINVAL:
            errstr = "\nThe whence argument is not a proper value, or the resulting file offset would be negative for a regular file, block special file, or directory.\n";
            break;
        case EOVERFLOW:
            errstr = "\nThe resulting file offset would be a value which cannot be represented correctly in an object of type off_t.\n";
            break;
        case ESPIPE:
            errstr = "\nThe fildes argument is associated with a pipe, FIFO, or socket.\n";
            break;
	}
	printf("%s", errstr);
    if(islogging)  fprintf(logfile, "%s", errstr);
}

void printreaderr(FILE * logfile, int islogging)
{
	char * errstr;
	switch (errno)
	{
        case EAGAIN:
            errstr = "\nThe file is neither a pipe, nor a FIFO, nor a socket, the O_NONBLOCK flag is set for the file descriptor, and the thread would be delayed in the read operation.\n";
            break;
        case EBADF:
            errstr = "\nThe fildes argument is not a valid file descriptor open for reading.\n";
            break;
        case EBADMSG:
            errstr = "\nThe file is a STREAM file that is set to control-normal mode and the message waiting to be read includes a control part.\n";
            break;
        case EINTR:
            errstr = "\nThe read operation was terminated due to the receipt of a signal, and no data was transferred.\n";
            break;
        case EINVAL:
            errstr = "\nThe STREAM or multiplexer referenced by fildes is linked (directly or indirectly) downstream from a multiplexer.\n";
            break;
        case EIO:
            errstr = "\nThe process is a member of a background process group attempting to read from its controlling terminal, and either the calling thread is blocking SIGTTIN or the process is ignoring SIGTTIN or the process group of the process is orphaned. This error may also be generated for implementation-defined reasons.\n";
            break;
        case EISDIR:
            errstr = "\nThe fildes argument refers to a directory and the implementation does not allow the directory to be read using read() or pread(). The readdir() function should be used instead.\n";
            break;
        case EOVERFLOW:
            errstr = "\nThe file is a regular file, nbyte is greater than 0, the starting position is before the end-of-file, and the starting position is greater than or equal to the offset maximum established in the open file description associated with fildes.\n";
            break;
        case EINVAL:
            errstr = "\nThe file is a regular file or block special file, and the offset argument is negative. The file offset shall remain unchanged.\n";
            break;
        case ESPIPE:
            errstr = "\nThe file is incapable of seeking.\n";
            break;
        case EAGAIN:
            errstr = "\nThe file is a pipe or FIFO, the O_NONBLOCK flag is set for the file descriptor, and the thread would be delayed in the read operation.\n";
            break;
        case EWOULDBLOCK:
            errstr = "\nThe file is a socket, the O_NONBLOCK flag is set for the file descriptor, and the thread would be delayed in the read operation.\n";
            break;
        case ECONNRESET:
            errstr = "\nA read was attempted on a socket and the connection was forcibly closed by its peer.\n";
            break;
        case ENOTCONN:
            errstr = "\nA read was attempted on a socket that is not connected.\n";
            break;
        case ETIMEDOUT:
            errstr = "\nA read was attempted on a socket and a transmission timeout occurred.\n";
            break;
        case EIO:
            errstr = "\nA physical I/O error has occurred.\n";
            break;
        case ENOBUFS:
            errstr = "\nInsufficient resources were available in the system to perform the operation.\n";
            break;
        case ENOMEM:
            errstr = "\nInsufficient memory was available to fulfill the request.\n";
            break;
        case ENXIO:
            errstr = "\nA request was made of a nonexistent device, or the request was outside the capabilities of the device.\n";
            break;
	}
	printf("%s", errstr);
    if(islogging)  fprintf(logfile, "%s", errstr);
}

void printwriteerr(FILE * logfile, int islogging)
{
	char * errstr;
	switch (errno)
	{
        case EAGAIN:
            errstr = "\nThe file is neither a pipe, nor a FIFO, nor a socket, the O_NONBLOCK flag is set for the file descriptor, and the thread would be delayed in the write() operation.\n";
            break;
        case EBADF:
            errstr = "\nThe fildes argument is not a valid file descriptor open for writing.\n";
            break;
        case EFBIG:
            errstr = "\nAn attempt was made to write a file that exceeds the implementation-defined maximum file size [XSI] [Option Start]  or the file size limit of the process, [Option End]  and there was no room for any bytes to be written.\n";
            break;
        case EFBIG:
            errstr = "\nThe file is a regular file, nbyte is greater than 0, and the starting position is greater than or equal to the offset maximum established in the open file description associated with fildes.\n";
            break;
        case EINTR:
            errstr = "\nThe write operation was terminated due to the receipt of a signal, and no data was transferred.\n";
            break;
        case EIO:
            errstr = "\nThe process is a member of a background process group attempting to write to its controlling terminal, TOSTOP is set, the calling thread is not blocking SIGTTOU, the process is not ignoring SIGTTOU, and the process group of the process is orphaned. This error may also be returned under implementation-defined conditions.\n";
            break;
        case ENOSPC:
            errstr = "\nThere was no free space remaining on the device containing the file.\n";
            break;
        case ERANGE:
            errstr = "\nThe transfer request size was outside the range supported by the STREAMS file associated with fildes.\n";
            break;
        case EAGAIN:
            errstr = "\nThe file is a pipe or FIFO, the O_NONBLOCK flag is set for the file descriptor, and the thread would be delayed in the write operation.\n";
            break;
        case EAGAIN:
            errstr = "\nThe file is a socket, the O_NONBLOCK flag is set for the file descriptor, and the thread would be delayed in the write operation.\n";
            break;
        case EWOULDBLOCK:
            errstr = "\nThe file is a socket, the O_NONBLOCK flag is set for the file descriptor, and the thread would be delayed in the write operation.\n";
            break;
        case ECONNRESET:
            errstr = "\nA write was attempted on a socket that is not connected.\n";
            break;
        case EPIPE:
            errstr = "\nAn attempt is made to write to a pipe or FIFO that is not open for reading by any process, or that only has one end open. A SIGPIPE signal shall also be sent to the thread.\n";
            break;
        case EPIPE:
            errstr = "\nA write was attempted on a socket that is shut down for writing, or is no longer connected. In the latter case, if the socket is of type SOCK_STREAM, a SIGPIPE signal shall also be sent to the thread.\n";
            break;
        case EINVAL:
            errstr = "\nThe STREAM or multiplexer referenced by fildes is linked (directly or indirectly) downstream from a multiplexer.\n";
            break;
        case EIO:
            errstr = "\nA physical I/O error has occurred.\n";
            break;
        case ENOBUFS:
            errstr = "\nInsufficient resources were available in the system to perform the operation.\n";
            break;
        case ENXIO:
            errstr = "\nA request was made of a nonexistent device, or the request was outside the capabilities of the device.\n";
            break;
        case ENXIO:
            errstr = "\nA hangup occurred on the STREAM being written t\n";
            break;
        case ENXIO:
            errstr = "\nA write to a STREAMS file may fail if an error message has been received at the STREAM head. In this case, errno is set to the value included in the error message.\n";
            break;
        case EACCES:
            errstr = "\nA write was attempted on a socket and the calling process does not have appropriate privileges.\n";
            break;
        case ENETDOWN:
            errstr = "\nA write was attempted on a socket and the local network interface used to reach the destination is down.\n";
            break;
        case ENETUNREACH:
            errstr = "\n\A write was attempted on a socket and no route to the network is present.n";
            break;
            // pwrite() errors, in additions to the above
        case EINVAL:
            errstr = "\nThe file is a regular file or block special file, and the offset argument is negative. The file offset shall remain unchanged.\n";
            break;
        case ESPIPE:
            errstr = "\nThe file is incapable of seeking.\n";
	}
	printf("%s", errstr);
    if(islogging)  fprintf(logfile, "%s", errstr);
}
