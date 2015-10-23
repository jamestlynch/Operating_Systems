
#include "syscall.h"
int cvindex, lockindex, test;


void t6_t1() 
{
    /*2nd to acquire lock*/
    test=AcquireLock(lockindex);
    test=Wait(lockindex, cvindex);  
    Write("SECOND\n", sizeof("SECOND\n"), 1);
    test=ReleaseLock(lockindex);
    Exit(0);
}
void t6_t2() 
{
    /*3rd to acquire lock*/
    test=AcquireLock(lockindex);
    test=Wait(lockindex, cvindex);
    Write("THIRD\n", sizeof("THIRD\n"), 1);
    test=ReleaseLock(lockindex);
    Exit(0);
}

void t6_t3() 
{
    /*3rd to acquire lock*/
    test=AcquireLock(lockindex);
    test=Wait(lockindex, cvindex);
    Write("FORTH\n", sizeof("FORTH\n"), 1);
    Write("Passed broadcast test if FINISHED numbers are in increasing order.\n", sizeof("Passed broadcast test if FINISHED numbers are in increasing order.\n"), 1);
    test=ReleaseLock(lockindex);
    Exit(0);
}

void t6_t4() {
    /*1st to acquire lock*/
    test=AcquireLock(lockindex);
    Write("thread calls broadcast\n", sizeof("thread calls broadcast\n"), 1);
    test=Broadcast(lockindex, cvindex);
    test=ReleaseLock(lockindex);
    Exit(0);
}

int main()
{
	Write("Test 6 start\n", sizeof("Test 6 start\n"), 1);
	lockindex = CreateLock("lock1", 5);
    cvindex = CreateCV("cv1", 3);

    Fork("thread1", 7, t6_t1);
    Fork("thread2", 7, t6_t2);
    Fork("thread3", 7, t6_t3);
    Fork("thread4", 7, t6_t4);

}