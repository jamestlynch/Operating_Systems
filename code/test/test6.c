
#include "syscall.h"
int cvindex, lockindex, test;


void t6_t1() {
    /*2nd to acquire lock*/
    test=AcquireLock(lockindex);
    test=Wait(test, cvindex);  
    Write("SECOND\n", sizeof("SECOND\n"), 1);
    Exit(0);
}
void t6_t2() {
    /*3rd to acquire lock*/
    test=AcquireLock(lockindex);
    test=Wait(test, cvindex);
    Write("THIRD\n", sizeof("THIRD\n"), 1);
    Exit(0);
}
void t6_t4() {
    /*3rd to acquire lock*/
    test=AcquireLock(lockindex);
    test=Wait(test, cvindex);
    Write("FORTH\n", sizeof("FORTH\n"), 1);
    Write("Passed broadcast test if FINISHED numbers are in increasing order.\n", sizeof("Passed broadcast test if FINISHED numbers are in increasing order.\n"), 1);
    test=ReleaseLock(test);
    Exit(0);
}
void t6_t3() {
    /*1st to acquire lock*/
    test=AcquireLock(lockindex);
    Write("thread calls broadcast\n", sizeof("thread calls broadcast\n"), 1);
    test=Broadcast(test, cvindex);
    test=ReleaseLock(lockindex);
    Exit(0);
}
int
main(){
	Write("Test 6 start\n", sizeof("Test 6 start\n"), 1);
	lockindex= CreateLock("abc", 3);
    cvindex= CreateCV("def", 3);
    Fork("thread1", 7, t6_t1);
    Fork("thread2", 7, t6_t2);
    Fork("thread3", 8, t6_t3);
    Fork("thread4", 8, t6_t4);
}