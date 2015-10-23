#include "syscall.h"
int indexcheck1, indexcheck2, indexcheck3, indexcheck4;

void t3_t1(){
    int cv;
    cv=CreateCV("def", 1);
    indexcheck1=DestroyCV(cv+100);
    indexcheck2= CreateCV("abc", -1);
    indexcheck3=DestroyCV(-100);
    if (indexcheck1!=-1 && indexcheck2!=-1 && indexcheck3!=-1){
        Write("failed: create/destroyCV validates input\n", sizeof("failed: create/destroyCV validates input\n"), 1);    
    }
    else{
         Write("passed: create/destroyCV validates input\n", sizeof("passed: create/destroyCV validates input\n"), 1);
    }
}
int main(){
    Write("Test 3 start.\n", sizeof("Test 3 start\n"), 1);
    Fork("thread1", 7, t3_t1);
}

/*

PASSED

*/