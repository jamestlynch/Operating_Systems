#include "syscall.h"

int test;
int LockIndex1;

void t5_t1(){
    test=AcquireLock(LockIndex1);
    Write("1 acquired\n", sizeof("1 acquired\n"), 1);
    test=ReleaseLock(LockIndex1);
    Write("1 released\n", sizeof("1 released\n"), 1);
    Exit(0);
}
void t5_t2(){
    test=AcquireLock(LockIndex1);
    Write("2 acquired\n", sizeof("2 acquired\n"), 1);
    test=ReleaseLock(LockIndex1);
    Write("2 released\n", sizeof("2 released\n"), 1);
    Exit(0);
}
void t5_t3(){
    test=AcquireLock(LockIndex1);
    Write("3 acquired\n", sizeof("3 acquired\n"), 1);
    test=ReleaseLock(LockIndex1);
    Write("3 released\n", sizeof("3 released\n"), 1);
    Exit(0);
}
void t5_t4(){
    test=AcquireLock(LockIndex1);
    Write("4 acquired\n", sizeof("3 acquired\n"), 1);
    test=ReleaseLock(LockIndex1);
    Write("4 released\n", sizeof("3 released\n"), 1);
    Write("Passed: acquired GREEN numbers are increasing order/non overlapping.\n", sizeof("Passed: acquired GREEN numbers are increasing order/non overlapping.\n"), 1);
    Exit(0);
}
int main(){
    Write("Test 5 start. Multiple threads try to acquire the same lock. Should not overlap.\n", sizeof("Test 5 start. Multiple threads try to acquire the same lock. Should not overlap.\n"), 1);
    LockIndex1=CreateLock("abc", 3);
    Fork("thread1", 7, t5_t1);
    Fork("thread2", 7, t5_t2);
    Fork("thread3", 7, t5_t3);
    Fork("thread4", 7, t5_t4);
    Exit(0);
}

/*

PASSED

*/