/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"
int CVIndex1, CVIndex2, CVIndex3, CVIndex4;
int indexlock1, indexlock2;
int destroy1, acquire1, release1;
int lockIndex;

int testing1 = 1;
int testing2 = 0;
int testing3 = 0;
int testing5 = 0;


void BoundsErrorCheck_Test(){
    


    /*Write("Test2: Two different lock TEST\n", sizeof("Test2: Two different lock TEST\n"), ConsoleOutput);
    LockIndex2 = CreateLock("SecondLOCK", 9);
    CVIndex2 = CreateCV("SecondCV", 7);
    LockIndex3 = CreateLock("ThirdLOCK", 9);
    CVIndex3 = CreateCV("ThirdCV", 7);
    /*Fork three functions that are used for the test!*/
    /*Fork(function4, "changeme", sizeof("changeme"));
    Fork(function5, "changeme", sizeof("changeme"));
    Fork(function6, "changeme", sizeof("changeme"));
    Exit(0);*/
 
    lockIndex = CreateLock("def", 3);

    indexlock1 = CreateLock("abc", 0);
    if (indexlock1 != -1) {
    	Write("CreateLock failed: Should return -1 when the length of the lock's identifier is 0.\n", sizeof("CreateLock failed: Should return -1 when the length of the lock's identifier is 0.\n"), 1);
    }

    indexlock1 = CreateLock("abc", -1);
    if (indexlock1 != -1) {
        Write("CreateLock failed: Should return -1 for negative lock identifier lengths.\n", 74, 1);
    }

    indexlock1 = CreateLock(0, 1);
    if (indexlock1 != -1) {
        Write("CreateLock failed: Should return -1 for bad pointers to lock identifier.\n", 73, 1);
    }

    /* THIS TEST BELOW CAUSES IT TO BREAK.
    indexlock1 = CreateLock(-1, 1);
    if (indexlock1 != -1) {
        Write("CreateLock failed: Should return -1 for invalid pointers to lock identifier.\n", 77, 1);
    }*/

    indexlock1 = CreateLock("abc", 3);
    if (indexlock1 == -1) {
        Write("CreateLock failed: Should NOT return -1 when valid lock identifier and lengths are passed in.\n", 94, 1);
    }

    indexlock2 = CreateLock("abc", 3);
    if (indexlock1 == indexlock2) {
        Write("CreateLock failed: Should NOT return the same index when creating two locks.\n", 77, 1);
    }

    indexlock1= AcquireLock(-1);
    if (indexlock1 != -1){
            Write("AcquireLock0 failed: Should return -1 when lock index is out of bounds.\n", 78, 1);
    }
    indexlock1= AcquireLock(100);
    if (indexlock1 != -1){
            Write("AcquireLock1 failed: Should return -1 when lock index is out of bounds.\n", 78, 1);
    }
    indexlock1= AcquireLock(lockIndex);
    if (indexlock1!= -1){
            Write("Lock created for acquire test.\n", sizeof("Lock created for acquire test.\n"), 1);
    }
    indexlock1= ReleaseLock(300);
    if (indexlock1 != -1){
            Write("ReleaseLock0 failed: Should return -1 when lock index is out of bounds.\n", 79, 1);
    }
    indexlock1= ReleaseLock(-1);
    if (indexlock1 == -1){
            Write("ReleaseLock3 failed: Should return -1 when lock index is out of bounds.\n", 79, 1);
    }
    indexlock1 = DestroyLock(-1);
    if (indexlock1 != -1) {
        Write("DestroyLock0 failed: Should return -1 for negative index.\n", 75, 1);
    }
    indexlock1 = DestroyLock(1000);
    if (indexlock1 != -1) {
        Write("DestroyLock1 failed: Should return -1 for out of bounds.\n", 74, 1);
    }

    /* Possible other tests: */
    /*  - Bad vaddr: different address space */
    /*  - If vector has no more room ?? */
    /*  - Memory running out ?? */
}
void
passingVars_Test(){
    lockIndex = CreateLock("def", 3);
    acquire1=AcquireLock(lockIndex);
    destroy1=DestroyLock(acquire1);
    release1=ReleaseLock(lockIndex);
    if(lockIndex!= -1 && acquire1!=-1 && destroy1 != -1 && release1 != -1){
        Write("Passed test passing variables\n", sizeof("Success\n"), 1);
    }
}

void
Acquire_Test() {

    lockIndex = CreateLock("abc", 3);
    lockIndex2 = CreateLock("def", 3);

    acquire1= AcquireLock(-1);
    if (acquire1 != -1){
            Write("AcquirLock1 failed: Should return -1 when lock index is out of bounds.\n", 77, 1);
    }
    acquire1= AcquireLock(100);
    if (acquire1 != -1){
            Write("AcquireLock1 failed: Should return -1 when lock index is out of bounds.\n", sizeof("AcquireLock1 failed: Should return -1 when lock index is out of bounds.\n"), 1);
    }
    acquire1= AcquireLock(lockIndex);
    if (acquire1!= -1){
            Write("Lock created for acquire test.\n", 77, 1);
    }

    /*
    set lock index to be acquired by someone else. go on wait queue.

    AFTER SUCCESSFULL CREATING A LOCK, THE NEXT RELEASE DOESNT WORK AS EXPECTED.
    */
    /*destroyIndex= DestroyLock(acquire1);


	/* Lock is set to delete */

	/* Process is not lock owner */
}
void Release_Test(){
    int acquire1;
    int release1;
    int lockIndex;
    int lockIndex2;

    lockIndex = CreateLock("abc", 3);
    lockIndex2 = CreateLock("def", 3);

    /*confirm in the correct process, confirm is the lock owner currently- cannot release lock*/


}

void
DestroyLock_Test() {

    int indexlock1, indexlock2;
    int lockIndex;
     int destroy1;
    int lock;
    int acquire;
    int release;

    lockIndex = CreateLock("abc", 3);
    lockIndex = CreateLock("abc", 3);
    lockIndex = CreateLock("abc", 3);
    lockIndex = CreateLock("abc", 3);

    indexlock1 = DestroyLock(-1);
    if (indexlock1 != -1) {
        Write("DestroyLock failed: Should return -1 for negative index.\n", 74, 1);
    }
    else{
        Write("Success\n", 8, 1);
    }
    indexlock1 = DestroyLock(1000);
    if (indexlock1 != -1) {
        Write("DestroyLock failed: Should return -1 for index out of bounds.\n", 73, 1);
    }
    else{
        Write("Success\n", 8, 1);
    }

    indexlock1= DestroyLock(lockIndex);

    lock= CreateLock("abc", 3);
    destroy1= DestroyLock(lock);

    lock= CreateLock("def", 3);
    acquire= AcquireLock(lock);
    release= ReleaseLock(acquire);
    destroy1= DestroyLock(release);

    lock= CreateLock("def", 3);
   /* acquire();
    DestroyCV();
    release();
    DestroyCV();*/

    /*LOCK OWNER
    //ARE THERE WAITING THREADS? NO, DELETE LOCK IMMEDIATELY. YES, SET TODELETE=TRUE*/
}


void function1() {
    /*In this function, it increments integer(testing1) by 5 after function 3 wake it up*/
    AcquireLock(indexlock1);
    Wait(indexlock1, CVIndex1);
    testing1 = testing1 + 5;
    ReleaseLock(indexlock1);
    DestroyLock(indexlock1);
    DestroyCV(CVIndex1);
    Exit(0);
}
void function2() {
    /*In this function, it increments integer(testing1) by 3 after function 3 wake it up*/
    AcquireLock(indexlock1);
    Wait(indexlock1, CVIndex1);
    testing1 = testing1 + 3;
    ReleaseLock(indexlock1);
    /*Sinec this function is waken up the last so integer value all calculation 
    Therefore, we need to check the result and if we can get the expected result, then the test passes*/
    if(testing1 == 10) {
        Write("Broadcast/signal test passed\n", sizeof("Broadcast/signal test passed\n"), ConsoleOutput);
    }else {
        Write("Broadcast/signal test failed\n", sizeof("Broadcast/signal test failed\n"), ConsoleOutput);
    }
    /*At this point, we are done with the test so going to the next function that we can start the next test!*/
    /*Fork(testStart2, "changeme", sizeof("changeme"));*/
    Exit(0);
}
void function3() {
    /*In this function, it mutiply the integer(testing1) before waking other condition variable in other two function.
    in order to see if this broadcast syscall actually wake others condition variable up.
    Otherwise, integer value is going to be different at the end. */
    AcquireLock(indexlock1);
    testing1 = testing1 * 2;
    Broadcast(indexlock1, CVIndex1);
    ReleaseLock(indexlock1);
    Exit(0);
}


void test1(){
    Fork("thread1", 7, function1);
    Fork("thread2", 7, function2);
    Fork("thread3", 7, function3);
}

int 
main() {

    BoundsErrorCheck_Test();
    passingVars_Test();

    /*test1();*/

/*
    int lock1;

    lock1 = CreateLock("abc", 3);
    /*lock2= CreateLock("def", 3);
    lock3= CreateLock("ghi", 3);
    lock4= CreateLock("jkl", 3);*/

    


     /*CreateLock_Test(); */
     /*Acquire_Test();*/
     /*Release_Test();*/
     /*DestroyLock_Test();*/


	/* Multiple threads test */
}
