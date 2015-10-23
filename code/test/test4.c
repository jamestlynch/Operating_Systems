#include "syscall.h"

int indexcheck1, indexcheck2, indexcheck3, indexcheck4, indexcheck5, indexcheck6, indexcheck7, indexcheck8, indexcheck9, indexcheck10, indexcheck11, indexcheck12;
int i;

void t4_t1(){
    i = CreateLock("abc", 3);
    indexcheck1= Signal(1, -1);
    indexcheck2= Signal(-1, 1);
    indexcheck3= Signal(1000, 1);
    indexcheck4= Signal(1, 1000);
    indexcheck5= Broadcast(1, -1);
    indexcheck6= Broadcast(-1, 1);
    indexcheck7= Broadcast(1000, 1);
    indexcheck8= Broadcast(1, 1000);
    indexcheck9= Wait(1, -1);
    indexcheck10= Wait(-1, 1);
    indexcheck11= Wait(1000, 1);
    indexcheck12= Wait(1, 1000);
    if (indexcheck1==-1 && indexcheck2==-1 && indexcheck3==-1 && indexcheck4==-1 && indexcheck5==-1 && indexcheck6==-1 && indexcheck7==-1 && indexcheck8==-1 && indexcheck9==-1 && indexcheck10==-1 && indexcheck11==-1 && indexcheck12==-1 ){
        Write("passed: signal/broadc/wait valid input\n", sizeof("passed: signal/broadc/wait valid input\n"), 1);
    }
    else{
        Write("failed: create/destroyCV valid input\n", sizeof("failed: create/destroyCV valid input\n"), 1);
    }
}
int 
main() {
    Write("Test 4 start\n", sizeof("Test 4 start\n"), 1);
    Fork("thread1", 7, t4_t1); 
}

/*

PASSED

*/