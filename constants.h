/*
 * constants.h
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#define GF                         256          // Galouis field size for Reed-Solomon algorithm
#define MIN_RS_BLOCKSIZE           256          // Minimal Reed-Solomon block being tested
                                                // Must be multiplier of 4
                                                // Small blocks significantly slows down readbacks

#define DISK_BUFFER_DEFAULT        1024*1024    // Buffer and file size for reading and writing to raw devices
                                                // Must be multiplier of 4
#define FILE_SIZE_MAX              0x8000000ULL // Maximum buffer/file size
#define FILE_SIZE_MIN              16ULL        // Minimum buffer/file size

#define PATH_LENGTH                4096         // # chars in a path name including nul
#define DIGITS_MAX                 24           // chars for string representation of uint64_t
#define SAVESTR_LEN                120          // string length for save file and alike

#define FILES_PER_FOLDER_DEFAULT   1000         // max number of files per folder on tested path
#define FILES_PER_FOLDER_MAX       1000000      // max number of files per folder on tested path
#define MAX_FILE_COUNT             (1LL << 60)  // Maximum total number of files to work with

#define MAX_RETRIES                16           // Maximum number of repeats to read or write to file/device
#define SKIP_BYTES                 8            // Bytes to be skipped in case of unrecoverable read/write errors
#define SKIP_DIV                   4            // Skip bytes growth rate in case of subsequent unrecoverable read/write errors
                                                // skip = skip * (1 + 1/SKIP_DIV)
                                                // should be not less than 1
#define MAX_SKIP_BYTES             (1 << 30)

#define SYSFS_SIZE_BLOCKSIZE       512          // Size units derived from /sys/class/block/sd*/size

#define BUF_ALGNMT                 4096         // Alignment of buffer for O_DIRECT I/O

#define ONESHOT                    1            // First run filesystem overhead measurement feature
#define REPEATED                   2            // Each run  filesystem overhead measurement feature

#endif /* CONSTANTS_H_ */
