CFLAGS= -Wall -O1 -Wno-parentheses -std=c99 -D_POSIX_C_SOURCE

massstoragetester : main.o main.o tests.o devtest.o filetest.o fileread.o filewrite.o devwrite.o devread.o fec.o rng.o read.o printprogress.o init.o strconv.o errmesg.o optrw.o
	cc $(CFLAGS) -o massstoragetester main.o tests.o devtest.o filetest.o fileread.o filewrite.o devwrite.o devread.o fec.o rng.o read.o printprogress.o init.o strconv.o errmesg.o optrw.o

main.o : main.c init.h printprogress.h constants.h tests.h
	cc $(CFLAGS) -c main.c

tests.o : tests.c printprogress.h rng.h devtest.h filetest.h
	cc $(CFLAGS) -c tests.c

devtest.o : devtest.c devwrite.h devread.h read.h printprogress.h strconv.h
	cc $(CFLAGS) -c devtest.c

filetest.o : filetest.c filetest.h printprogress.h strconv.h constants.h filewrite.h fileread.h optrw.h devread.h devwrite.h rng.h
	cc $(CFLAGS) -c filetest.c
	
fileread.o : fileread.c fileread.h constants.h read.h
	cc $(CFLAGS) -c fileread.c
	
filewrite.o : filewrite.c filewrite.h constants.h
	cc $(CFLAGS) -c filewrite.c

devwrite.o : devwrite.c rng.h
	cc $(CFLAGS) -c devwrite.c

devread.o : devread.c constants.h rng.h fec.h
	cc $(CFLAGS) -c devread.c 

fec.o : fec.c fec.h strconv.h constants.h
	cc $(CFLAGS) -c fec.c

rng.o : rng.c constants.h
	cc $(CFLAGS) -c rng.c

read.o : read.c
	cc $(CFLAGS) -c read.c

printprogress.o : printprogress.c printprogress.h strconv.h
	cc $(CFLAGS) -c printprogress.c

init.o : init.c init.h strconv.h constants.h
	cc $(CFLAGS) -c init.c

strconv.o : strconv.c strconv.h
	cc $(CFLAGS) -c strconv.c

errmesg.o : errmesg.c errmesg.h
	cc $(CFLAGS) -c errmesg.c

optrw.o : optrw.c constants.h rng.h errmesg.h
	cc $(CFLAGS) -c optrw.c

clean :
	rm -f *.o massstoragetester
