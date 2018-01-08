/*
 * strconv.h
 */

#ifndef STRCONV_H_
#define STRCONV_H_

// Converts bytes count to human readable string
void bytestostr(uint64_t bytes, char * str);

// converts seconds interval to date
void todate(uint64_t s, char * date);

// Converts human-readable size string to bytes
uint64_t tobytes(char * x);

#endif /* STRCONV_H_ */
