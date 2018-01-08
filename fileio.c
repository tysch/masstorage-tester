#define _XOPEN_SOURCE 500

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "print.h"
#include "errmesg.h"
#include "constants.h"
#include "nofailio.h"

// TODO: separate per-run and global error cap

extern int stop_all;

// Create and write buf to file; returns number of i/o errors
uint32_t nofail_writefile(char * path, char * buf, uint32_t bufsize)
{
    uint32_t ret;
    char errstr[PATH_LENGTH];

    int fd = nofail_open(path);

    if(fd == -1)
    {
        sprintf(errstr, "\n%s access error!\n", path);
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
uint32_t nofail_readfile(char * path, char * buf, uint32_t bufsize, int notdeletefile)
{
    uint32_t ret;
    char errstr[PATH_LENGTH];

    int fd = nofail_open(path);

    if(fd == -1)
    {
        sprintf(errstr, "\n%s access error!\n", path);
        print(ERROR, errstr);
        return bufsize;
    }
    else
    {
        ret = nofail_pread(fd, buf, bufsize, 0);
        nofail_close(fd);
        if(!notdeletefile) nofail_unlink(path);
    }
    return ret;
}

int nofail_stat(const char *path, struct stat *buf)
{
    int ret = stat(path, buf);
    if(ret == -1) printerr("\nFile stat reading error:");
    return ret;
}

int isdir(const char *path)
{
    struct stat buf;
    nofail_stat(path, &buf);

    if(buf.st_mode & S_IFDIR) return 1;
    return 0;
}

// Wrapper for mkdir with error message logging
int nofail_mkdir(char * path)
{
    int ret = mkdir(path, 0666);
    //int ret = 0;
    if(ret == -1) printerr("\nDirectory creation error:");
    return ret;
}

// Wrapper for mkdir with error message logging
int nofail_rmdir(char * path)
{
    int ret = rmdir(path);
    if(ret == -1) printerr("\nDirectory removing error:");
    return ret;
}

// TODO: Dirent have d type optimization
// FIXME: make unused functions static

// Recursively create directories up to depth level while counting total nfiles they can contain
void rec_mkdir(char * path, uint64_t * nfiles, uint32_t files_per_folder, int depth, int is_topdir)
{
    char current_foldername[DIGITS_MAX];
    char current_path[PATH_LENGTH];

    int mkd = 0;

    if((*nfiles) == 0) return; // No directories need to be added

    if(!is_topdir)  mkd = nofail_mkdir(path);  // Create a directory at current depth

    if(mkd == -1) return;      // Failed to create directory, skip all subdirectories creation

    if(depth == 0) // Maximum recursion depth reached, subtract the file count and move to another branch
    {
        if(*nfiles > files_per_folder) (*nfiles) -= files_per_folder;
        else (*nfiles) = 0;
        return;
    }

    // For non-zero depth, populate current directory with subdirectories
    if(depth > 0)
    {
        for(int i = 0; i < files_per_folder; i++)
        {
            sprintf(current_foldername, "/%i", i);
            strcpy(current_path, path);
            strcat(current_path, current_foldername);
            rec_mkdir(current_path, nfiles, files_per_folder, depth - 1, 0);
        }
    }
}

// Initialize directory tree for nfiles
void create_dirs(char * path, uint64_t nfiles, uint32_t files_per_folder)
{
    int depth = 0;
    uint64_t tmp = nfiles;

    // Maximum recursion depth
    while(tmp > files_per_folder)
    {
        depth++;
        tmp /= files_per_folder;
    }
    tmp = nfiles;
    rec_mkdir(path, &tmp, files_per_folder, depth, 1);
}

// Generate filename for n-th file in a directory tree
void path_append(char * path, char * fullpath, uint64_t n, uint64_t totnfiles, uint32_t files_per_folder)
{
    char filename[DIGITS_MAX];

    uint64_t tmp = totnfiles;

    uint64_t name = 0;

    uint64_t div = 1;
    filename[0] = '\0';

    if(n > totnfiles)
    {
        printf("\nPath_append error: file out of range\n");
        stop_all = 1;
        return;
    }

    strcpy(fullpath, path);

    if(tmp < files_per_folder) 
    {
   //     sprintf(filename, "/%lli", (long long) n);
  //      strcat(fullpath, filename);
   //     return;
    }

    // Find total depth of nested folders
    while(tmp > files_per_folder)
    {
        tmp /= files_per_folder ;
        div *= files_per_folder;
    }

    while(div >= 1 /*files_per_folder*/  /*files_per_folder*/)
    {
        name = n / div;

        n -= name * div;

        div /= files_per_folder;

        // Append last digits of file
        sprintf(filename, "/%lli", (long long) name);
        strcat(fullpath, filename);
    }

 //   sprintf(filename, "/%lli", (long long) n);
 //   strcat(fullpath, filename);
}

// Checks if file in .jnk to delete only those files we've had added
int isjnkfile(char * name)
{
    // Skip digits
    while(*name != '\0')
    {
        if(*name == '.') break;
        if(!(*name >= '0' && *name <= '9')) return 0; // Foreign chracters
        name++;
    }

    if(strcmp(name, ".jnk") == 0) return 1;
    return 0;
}

// rm -rf-like routine; warns about files that was not deleted properly 
// FIXME: Check if tested enough
void delall(char * path, int preserve_topdir)
{
    char newpath[PATH_LENGTH];
    struct dirent * entry;
    char * d_name;

    DIR * d = opendir(path);
    if(d != NULL)
    {
        for(uint64_t i = 0 ; i < FILES_PER_FOLDER_MAX; i++)  /// Cap files_per_folder to this constant
        {
            entry = readdir(d);
            if (! entry) break; // End of file list

            d_name = entry->d_name;

            strcpy(newpath, path); // delete current file
            strcat(newpath, "/");
            strcat(newpath, d_name);

                if(strcmp(d_name, "..") == 0) continue; // skip special entries
                if(strcmp(d_name, "." ) == 0) continue;

            if(isdir(newpath)) // is a directory
            {

           //     strcpy(newpath, path); // delete current subdirectory
            //    strcat(newpath, "/");
             //   strcat(newpath, d_name);
                delall(newpath, 0);
            }
            else
            {
            //    strcpy(newpath, path); // delete current file
             //   strcat(newpath, "/");
            //    strcat(newpath, d_name);
                if(isjnkfile(d_name))
                {
                    nofail_unlink(newpath);
                }
                else 
                {
                    print(ERROR, "\nWarning! Foreign file in a directory tree");
                    printf("\nPath to error non-empty subdir = %s\n", newpath);
                }
            }
        }

        // No entries left in directory
        closedir(d);
        if(!preserve_topdir) nofail_rmdir(path);
    }
    else
    {
        printerr("\nCannot remove files: failed to open directory");
    }
}
