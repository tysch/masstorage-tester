CFLAGS= -Wall -O1 -Wno-parentheses -std=c99 -D_POSIX_C_SOURCE

massstoragetester : main.o printprogress.o init.o tests.o strconv.o rng.o devtest.o fec.o devwrite.o devread.o read.o
	cc $(CFLAGS) -o massstoragetester main.o printprogress.o init.o rng.o tests.o strconv.o devtest.o fec.o devwrite.o devread.o read.o

main.o : main.c init.h printprogress.h constants.h tests.h
	cc $(CFLAGS) -c main.c

tests.o : tests.c printprogress.h rng.h devtest.h filetest.h
	cc $(CFLAGS) -c tests.c

devtest.o : devtest.c devwrite.h devread.h
	cc $(CFLAGS) -c devtest.c

filetest.o : filetest.c 
	cc $(CFLAGS) -c filetest.c

devwrite.o : devwrite.c rng.h
	cc $(CFLAGS) -c devwrite.c

devread.o : devread.c constants.h rng.h
	cc $(CFLAGS) -c devread.c 

fec.o : fec.h
	cc $(CFLAGS) -c fec.c

rng.o : rng.c constants.h
	cc $(CFLAGS) -c rng.c

read.o : read.c
	cc $(CFLAGS) -c read.c

printprogress.o : printprogress.h strconv.h
	cc $(CFLAGS) -c printprogress.c

init.o : init.h strconv.h
	cc $(CFLAGS) -c init.c

strconv.o : strconv.c
	cc $(CFLAGS) -c strconv.c

clean :
	rm -f *.o
