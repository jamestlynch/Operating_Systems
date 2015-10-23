#include "syscall.h"

int test;
int lock1;
int cv1;

void t8_t1(){

}
void t8_t2(){

}
void t8_t3(){

}
void t8_t4(){

}

int main(){
    Write("Test 8 start. Multiple threads try to acquire the same lock. Should not overlap. \n", sizeof("Test 8 start\n"), 1);
    lock1=CreateLock("abc", 3);
    Fork("thread1", 7, t8_t1);
    Fork("thread2", 7, t8_t2);
    Fork("thread3", 7, t8_t3);
    Fork("thread4", 7, t8_t4);
    Exit(0);
}
/*

PASSED

*/