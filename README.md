# masstorage-tester
Universal tool for speed measurement, long term data retention and endurance testing.
Fills target folder with a pseudorandom data, reads it back and check for read errors and silent data corruption.

To compile, use 
$ gcc main.c -o masstoragetester -std=c99 -D_POSIX_C_SOURCE
