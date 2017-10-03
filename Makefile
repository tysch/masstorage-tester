CFLAGS= -Wall -O1 -Wno-parentheses -std=c99 -D_POSIX_C_SOURCE
CC = gcc

massstoragetester : main.o main.o tests.o devtest.o filetest.o fec.o rng.o print.o init.o strconv.o errmesg.o fileio.o nofailio.o saveload.o
	$(CC) $(CFLAGS) -o massstoragetester main.o tests.o devtest.o filetest.o fec.o rng.o print.o init.o strconv.o errmesg.o fileio.o nofailio.o saveload.o

main.o : main.c init.h print.h constants.h tests.h
	$(CC) $(CFLAGS) -c main.c

tests.o : tests.c print.h devtest.h filetest.h saveload.h
	$(CC) $(CFLAGS) -c tests.c

filetest.o : filetest.c filetest.h fileio.h strconv.h constants.h print.h rng.h
	$(CC) $(CFLAGS) -c filetest.c

devtest.o : devtest.c print.h nofailio.h fec.h rng.h
	$(CC) $(CFLAGS) -c devtest.c

fec.o : fec.c fec.h print.h strconv.h constants.h
	$(CC) $(CFLAGS) -c fec.c

fileio.o : fileio.c fileio.h print.h constants.h nofailio.h
	$(CC) $(CFLAGS) -c fileio.c

nofailio.o : nofailio.c nofailio.h errmesg.h constants.h
	$(CC) $(CFLAGS) -c nofailio.c

init.o : init.c init.h strconv.h constants.h print.h
	$(CC) $(CFLAGS) -c init.c

saveload.o : saveload.c saveload.h
	$(CC) $(CFLAGS) -c saveload.c

rng.o : rng.c constants.h
	$(CC) $(CFLAGS) -c rng.c

errmesg.o : errmesg.c errmesg.h print.h
	$(CC) $(CFLAGS) -c errmesg.c

print.o : print.c print.h strconv.h
	$(CC) $(CFLAGS) -c print.c

strconv.o : strconv.c strconv.h
	$(CC) $(CFLAGS) -c strconv.c	

clean :
	rm -f *.o massstoragetester
