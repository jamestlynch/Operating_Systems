/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

int test;
int indexcheck1, indexcheck2, indexcheck3, indexcheck4, indexcheck5, indexcheck6, indexcheck7, indexcheck8, indexcheck9, indexcheck10, indexcheck11, indexcheck12;
int LockIndex1, LockIndex2, LockIndex3, LockIndex4;
int CVIndex1, CVIndex2, CVIndex3, CVIndex4;
void t1_t1(){
    
    /*checks destroy and create invalid input is handled properly*/
    indexcheck1= CreateLock("abc", -1);
    indexcheck2=DestroyLock(indexcheck1+100);
    indexcheck3=DestroyLock(-100);
    if (indexcheck1==-1 && indexcheck2==-1 && indexcheck3==-1){
        Write("passed: create/destroyLOCK validates input\n", sizeof("passed: create/destroyLOCK validates input\n"), 1);
    }
    else{
        Write("failed: create/destroyLOCK validates input\n", sizeof("failed: create/destroyLOCK validates input\n"), 1);
    }
}
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

void t3_t1(){
    indexcheck1=CreateCV("def", 1);
    indexcheck2=DestroyCV(indexcheck1+100);
    indexcheck3= CreateCV("abc", -1);
    indexcheck4=DestroyCV(-100);
    if (indexcheck1==-1 && indexcheck2==-1 && indexcheck3==-1 && indexcheck4==-1){
        Write("passed: create/destroyCV validates input\n", sizeof("passed: create/destroyCV validates input\n"), 1);
    }
    else{
        Write("failed: create/destroyCV validates input\n", sizeof("failed: create/destroyCV validates input\n"), 1);
    }
}
void t4_t1(){
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
        Write("passed: create/destroyCV validates input\n", sizeof("passed: create/destroyCV validates input\n"), 1);
    }
    else{
        Write("failed: create/destroyCV validates input\n", sizeof("failed: create/destroyCV validates input\n"), 1);
    }
}
void t5_t1(){
    test=AcquireLock(LockIndex1);
    Write("1 acquired\n", sizeof("1 acquired\n"), 1);
    test=ReleaseLock(LockIndex1);
    Write("1 released\n", sizeof("1 released\n"), 1);
}
void t5_t2(){
    test=AcquireLock(LockIndex1);
    Write("2 acquired\n", sizeof("2 acquired\n"), 1);
    test=ReleaseLock(LockIndex1);
    Write("2 released\n", sizeof("2 released\n"), 1);
}
void t5_t3(){
    test=AcquireLock(LockIndex1);
    Write("3 acquired\n", sizeof("3 acquired\n"), 1);
    test=ReleaseLock(LockIndex1);
    Write("3 released\n", sizeof("3 released\n"), 1);
}
/*
void t5_t1() {
    test=AcquireLock(Lockindex1);
    test=Wait(Lockindex1, CVIndex1);
    Write("2\n", sizeof("2\n"), 1);
    test=DestroyLock(Lockindex1);
    test=ReleaseLock(Lockindex1);
    Exit(0);
}
void t5_t2() {
    test=AcquireLock(Lockindex1);
    test=Wait(Lockindex1, CVIndex1);
    Write("3\n", sizeof("3\n"), 1);
    test=ReleasLock(Lockindex1);
    this function is called the last among the functions that are used for the same test. print out the result and FORK next text function
    Write("Passed destroy test if printed numbers are in increasing order. \n End of test 1\n", sizeof("Passed if numbers are in increasing order. \n End of test 1\n"), 1);
    Exit(0);
}
void t5_t3() {
    test=Acquire(Lockindex1);
    Write("1\n", sizeof("1\n"), 1);
    test=Broadcast(Lockindex1, CVIndex1);
    test=Release(Lockindex1);
    Exit(0);
}*/
void t6_t1() {
    /*2nd to acquire lock*/
    test=AcquireLock(indexcheck1);
    test=Wait(LockIndex1, CVIndex1);
    Write("2\n", sizeof("2\n"), 1);
    test=ReleaseLock(indexcheck1);
    test=DestroyLock(indexcheck1);
    test=DestroyCV(CVIndex1);
    Exit(0);
}
void t6_t2() {
    /*3rd to acquire lock*/
    test=AcquireLock(indexcheck1);
    test=Wait(LockIndex1, CVIndex1);
    Write("3\n", sizeof("3\n"), 1);
    test=ReleaseLock(indexcheck1);
    Write("Passed broadcast test if printed numbers are in increasing order. \n End of test 1\n", sizeof("Passed if numbers are in increasing order. \n End of test 1\n"), 1);
    Exit(0);
}
void t6_t3() {
    /*1st to acquire lock*/
    test=AcquireLock(indexcheck1);
    Write("1\n", sizeof("1\n"), 1);
    test=Broadcast(indexcheck1, CVIndex1);
    test=ReleaseLock(indexcheck1);
    Exit(0);
}

void t7_t1(){
    test=AcquireLock(indexcheck1);
}
void t7_t2(){
    test=AcquireLock(indexcheck1);
}
void t7_t3(){
    test=AcquireLock(indexcheck1);
}

void test1(){
    Write("Test 1 start\n", sizeof("Test 1 start\n"), 1);
    Fork("thread1", 7, t1_t1);
    Exit(0);
}
void test2(){
    Write("Test 2 start\n", sizeof("Test 2 start\n"), 1);
    Fork("thread1", 7, t2_t1);

}
void test3(){
    Write("Test 3 start\n", sizeof("Test 3 start\n"), 1);
    Fork("thread1", 7, t3_t1);

}
void test4(){
    Write("Test 4 start\n", sizeof("Test 4 start\n"), 1);
    Fork("thread1", 7, t4_t1);
}
void test5(){
    Write("Test 5 start\n", sizeof("Test 5 start\n"), 1);
    Fork("thread1", 7, t5_t1);
    Fork("thread2", 7, t5_t2);
    Fork("thread3", 7, t5_t3);

}
void test6(){
    Write("Test 6 start\n", sizeof("Test 6 start\n"), 1);
    Fork("thread1", 7, t6_t1);
    Fork("thread2", 7, t6_t2);
    Fork("thread3", 7, t6_t3);
}

int 
main() {
    LockIndex1= CreateLock("Lock1", 5);
    Write("Lock 1 created\n", sizeof("Lock 1 created\n"), 1);
    LockIndex2 = CreateLock("Lock2", 5);
    Write("Lock 2 created\n", sizeof("Lock 2 created\n"), 1);
    LockIndex3 = CreateLock("Lock3", 5);
    Write("Lock 3 created\n", sizeof("Lock 3 created\n"), 1);
    LockIndex4 = CreateLock("Lock4", 5);
    Write("Lock 4 created\n", sizeof("Lock 4 created\n"), 1);
    CVIndex1 = CreateCV("CV1", 3);
    Write("CV 1 created\n", sizeof("CV 1 created\n"), 1);
    CVIndex2 = CreateCV("CV2", 3);
    Write("CV 2 created\n", sizeof("CV 2 created\n"), 1);
    CVIndex3 = CreateCV("CV3", 3);
    Write("CV 3 created\n", sizeof("CV 3 created\n"), 1);
    CVIndex4 = CreateCV("CV4", 3);
    Write("CV 4 created\n", sizeof("CV 4 created\n"), 1);

    /*BoundsErrorCheck_Test();
     passingVars_Test();*/

    test2();
    /*test2();



    /*test2();
    test3();
    test4();
    test5();*/
    /*test6();*/

/*
    int lock1;

    /*BoundsErrorCheck_Test();
    int lock1, lock2, lock3, lock4;
    lock1= CreateLock("abc", 3);
    /*lock2= CreateLock("def", 3);
    lock3= CreateLock("ghi", 3);
    lock4= CreateLock("jkl", 3);*/

    /*Fork("thread1", 7, unsigned int vFuncAddr);
    Fork("thread2", 7, unsigned int vFuncAddr);
    Fork("thread3", 7, unsigned int vFuncAddr);


     /*CreateLock_Test(); */
     /*Acquire_Test();*/
     /*Release_Test();*/
     /*DestroyLock_Test();*/


	/* Multiple threads test */
}
