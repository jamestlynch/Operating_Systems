#include "syscall.h"

int indexcheck1, indexcheck2, indexcheck3, indexcheck4;

void t2_t1()
{

    /* checks acquire and release invalid input is handled properly.*/
    indexcheck1 = AcquireLock(5000);
    indexcheck2 = ReleaseLock(5000);
    indexcheck3 = AcquireLock(-1);
    indexcheck4= ReleaseLock(-1);

    /*this function is called the last among the functions that are used for the same test. print out the result and FORK next text function*/
    if(indexcheck1 == - 1 && indexcheck2 == -1 && indexcheck3 == -1 && indexcheck4 == -1) {
        Write("passed: acquire/releaselock validates input\n", sizeof("passed: acquire/releaselock validates input\n"), 1);
    }else{
        Write("failed: acquire/releaselock validates input\n", sizeof("failed: acquire/releaselock validates input\n"), 1);
    }
    Exit(0);
}
int main(){
    Write("Test 2 start\n", sizeof("Test 2 start\n"), 1);
    Fork("thread1", 7, t2_t1);
}
