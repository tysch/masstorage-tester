/*
 * devwrite.h
 *
 */

#ifndef DEVWRITE_H_
#define DEVWRITE_H_


// Opens file and reserve space for size and seed information
int device_init_write(char * path, FILE * logfile,  int islogging, uint32_t bufsize);

// Fills device with a random data
int32_t write_rand_block(char *buf, int fd, FILE * logfile , int islogging, uint32_t bufsize);

// Embeds seed and size information in buffer with (DISK_BUFFER/16)-modular redundancy
void writeseedandsize (char * buf, uint32_t seed, uint64_t size, uint32_t bufsize);

#endif /* DEVWRITE_H_ */
