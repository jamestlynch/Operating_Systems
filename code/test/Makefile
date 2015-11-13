# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# If you are running on SunOS 5.x machines, uncomment the next line
GCCDIR = ../gnu/
LDFLAGS = -T newscript -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld
CPP = $(GCCDIR)cpp

INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

all: halt matmult sort testfiles locktest conditiontest runtests test1 test2 test3 test4 test5 test6 test7 test8 execTest forkTest clientTest1 clientTest2 clientTest3 clientTest4 mvtest

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

testfiles.o: testfiles.c
	$(CC) $(CFLAGS) -c testfiles.c
testfiles: testfiles.o start.o
	$(LD) $(LDFLAGS) start.o testfiles.o -o testfiles.coff
	../bin/coff2noff testfiles.coff testfiles

locktest.o: locktest.c
	$(CC) $(CFLAGS) -c locktest.c
locktest: locktest.o start.o
	$(LD) $(LDFLAGS) start.o locktest.o -o locktest.coff
	../bin/coff2noff locktest.coff locktest

conditiontest.o: conditiontest.c
	$(CC) $(CFLAGS) -c conditiontest.c
conditiontest: conditiontest.o start.o
	$(LD) $(LDFLAGS) start.o conditiontest.o -o conditiontest.coff
	../bin/coff2noff conditiontest.coff conditiontest

runtests.o: runtests.c
	$(CC) $(CFLAGS) -c runtests.c
runtests: runtests.o start.o
	$(LD) $(LDFLAGS) start.o runtests.o -o runtests.coff
	../bin/coff2noff runtests.coff runtests

test1.o: test1.c
	$(CC) $(CFLAGS) -c test1.c
test1: test1.o start.o
	$(LD) $(LDFLAGS) start.o test1.o -o test1.coff
	../bin/coff2noff test1.coff test1

test2.o: test2.c
	$(CC) $(CFLAGS) -c test2.c
test2: test2.o start.o
	$(LD) $(LDFLAGS) start.o test2.o -o test2.coff
	../bin/coff2noff test2.coff test2

test3.o: test3.c
	$(CC) $(CFLAGS) -c test3.c
test3: test3.o start.o
	$(LD) $(LDFLAGS) start.o test3.o -o test3.coff
	../bin/coff2noff test3.coff test3

test4.o: test4.c
	$(CC) $(CFLAGS) -c test4.c
test4: test4.o start.o
	$(LD) $(LDFLAGS) start.o test4.o -o test4.coff
	../bin/coff2noff test4.coff test4

test5.o: test5.c
	$(CC) $(CFLAGS) -c test5.c
test5: test5.o start.o
	$(LD) $(LDFLAGS) start.o test5.o -o test5.coff
	../bin/coff2noff test5.coff test5

test6.o: test6.c
	$(CC) $(CFLAGS) -c test6.c
test6: test6.o start.o
	$(LD) $(LDFLAGS) start.o test6.o -o test6.coff
	../bin/coff2noff test6.coff test6

test7.o: test7.c
	$(CC) $(CFLAGS) -c test7.c
test7: test7.o start.o
	$(LD) $(LDFLAGS) start.o test7.o -o test7.coff
	../bin/coff2noff test7.coff test7

test8.o: test8.c
	$(CC) $(CFLAGS) -c test8.c
test8: test8.o start.o
	$(LD) $(LDFLAGS) start.o test8.o -o test8.coff
	../bin/coff2noff test8.coff test8

execTest.o: execTest.c
	$(CC) $(CFLAGS) -c execTest.c
execTest: execTest.o start.o
	$(LD) $(LDFLAGS) start.o execTest.o -o execTest.coff
	../bin/coff2noff execTest.coff execTest

forkTest.o: forkTest.c
	$(CC) $(CFLAGS) -c forkTest.c
forkTest: forkTest.o start.o
	$(LD) $(LDFLAGS) start.o forkTest.o -o forkTest.coff
	../bin/coff2noff forkTest.coff forkTest

mvtest.o: mvtest.c
	$(CC) $(CFLAGS) -c mvtest.c
mvtest: mvtest.o start.o
	$(LD) $(LDFLAGS) start.o mvtest.o -o mvtest.coff
	../bin/coff2noff mvtest.coff mvtest

clientTest1.o: clientTest1.c
	$(CC) $(CFLAGS) -c clientTest1.c
clientTest1: clientTest1.o start.o
	$(LD) $(LDFLAGS) start.o clientTest1.o -o clientTest1.coff
	../bin/coff2noff clientTest1.coff clientTest1

clientTest2.o: clientTest2.c
	$(CC) $(CFLAGS) -c clientTest2.c
clientTest2: clientTest2.o start.o
	$(LD) $(LDFLAGS) start.o clientTest2.o -o clientTest2.coff
	../bin/coff2noff clientTest2.coff clientTest2

clientTest3.o: clientTest3.c
	$(CC) $(CFLAGS) -c clientTest3.c
clientTest3: clientTest3.o start.o
	$(LD) $(LDFLAGS) start.o clientTest3.o -o clientTest3.coff
	../bin/coff2noff clientTest3.coff clientTest3

clientTest4.o: clientTest4.c
	$(CC) $(CFLAGS) -c clientTest4.c
clientTest4: clientTest4.o start.o
	$(LD) $(LDFLAGS) start.o clientTest4.o -o clientTest4.coff
	../bin/coff2noff clientTest4.coff clientTest4

clean:
	rm -f *.o *.coff