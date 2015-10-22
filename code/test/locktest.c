/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"
// int CVIndex1, CVIndex2, CVIndex3, CVIndex4;
// int indexlock1, indexlock2;
// int destroy, acquire, release;
// int lockIndex;
// int testing2 = 0;
// int testing3 = 0;
// int testing5 = 0;


// void BoundsErrorCheck_Test(){

//     /*Write("Test2: Two different lock TEST\n", sizeof("Test2: Two different lock TEST\n"), ConsoleOutput);
//     LockIndex2 = CreateLock("SecondLOCK", 9);
//     CVIndex2 = CreateCV("SecondCV", 7);
//     LockIndex3 = CreateLock("ThirdLOCK", 9);
//     CVIndex3 = CreateCV("ThirdCV", 7);
//     /*Fork three functions that are used for the test!*/
//     /*Fork(function4, "changeme", sizeof("changeme"));
//     Fork(function5, "changeme", sizeof("changeme"));
//     Fork(function6, "changeme", sizeof("changeme"));
//     Exit(0);*/
 
//     lockIndex = CreateLock("def", 3);

//     indexlock1 = CreateLock("abc", 0);
//     if (indexlock1 != -1) {
//     	Write("CreateLock failed: Should return -1 when the length of the lock's identifier is 0.\n", sizeof("CreateLock failed: Should return -1 when the length of the lock's identifier is 0.\n"), 1);
//     }

//     indexlock1 = CreateLock("abc", -1);
//     if (indexlock1 != -1) {
//         Write("CreateLock failed: Should return -1 for negative lock identifier lengths.\n", 74, 1);
//     }
//     indexlock1 = CreateLock(0, 1);
//     if (indexlock1 != -1) {
//         Write("CreateLock failed: Should return -1 for bad pointers to lock identifier.\n", 73, 1);
//     }

//     /* THIS TEST BELOW CAUSES IT TO BREAK.
//     indexlock1 = CreateLock(-1, 1);
//     if (indexlock1 != -1) {
//         Write("CreateLock failed: Should return -1 for invalid pointers to lock identifier.\n", 77, 1);
//     }*/

//     indexlock1 = CreateLock("abc", 3);
//     if (indexlock1 == -1) {
//         Write("CreateLock failed: Should NOT return -1 when valid lock identifier and lengths are passed in.\n", 94, 1);
//     }

//     indexlock2 = CreateLock("abc", 3);
//     if (indexlock1 == indexlock2) {
//         Write("CreateLock failed: Should NOT return the same index when creating two locks.\n", 77, 1);
//     }

//     indexlock1= AcquireLock(-1);
//     if (indexlock1 != -1){
//             Write("AcquireLock0 failed: Should return -1 when lock index is out of bounds.\n", 78, 1);
//     }
//     indexlock1= AcquireLock(100);
//     if (indexlock1 != -1){
//             Write("AcquireLock1 failed: Should return -1 when lock index is out of bounds.\n", 78, 1);
//     }
//     indexlock1= AcquireLock(lockIndex);
//     if (indexlock1!= -1){
//             Write("Lock created for acquire test.\n", sizeof("Lock created for acquire test.\n"), 1);
//     }
//     indexlock1= ReleaseLock(300);
//     if (indexlock1 != -1){
//             Write("ReleaseLock0 failed: Should return -1 when lock index is out of bounds.\n", 79, 1);
//     }
//     indexlock1= ReleaseLock(-1);
//     if (indexlock1 == -1){
//             Write("ReleaseLock3 failed: Should return -1 when lock index is out of bounds.\n", 79, 1);
//     }
//     indexlock1 = DestroyLock(-1);
//     if (indexlock1 != -1) {
//         Write("DestroyLock0 failed: Should return -1 for negative index.\n", 75, 1);
//     }
//     indexlock1 = DestroyLock(1000);
//     if (indexlock1 != -1) {
//         Write("DestroyLock1 failed: Should return -1 for out of bounds.\n", 74, 1);
//     }

//     /* Possible other tests: */
//     /*  - Bad vaddr: different address space */
//     /*  - If vector has no more room ?? */
//     /*  - Memory running out ?? */
// }
// // void
// // passingVars_Test(){
// //     lockIndex = CreateLock("def", 3);
// //     acquire=AcquireLock(lockIndex);
// //     destroy=DestroyLock(acquire);
// //     release=ReleaseLock(lockIndex);
// //     if(lockIndex!= -1 && acquire!=-1 && destroy != -1 && release != -1){
// //         Write("Passed test passing variables\n", sizeof("Success\n"), 1);
// //     }
// // }

// // void
// // Acquire_Test() {

// //     indexlock1 = CreateLock("abc", 3);
// //     indexlock2 = CreateLock("def", 3);

// //     acquire= AcquireLock(-1);
// //     if (acquire != -1){
// //             Write("AcquirLock1 failed: Should return -1 when lock index is out of bounds.\n", sizeof("AcquirLock1 failed: Should return -1 when lock index is out of bounds.\n"), 1);
// //     }
// //     acquire= AcquireLock(100);
// //     if (acquire != -1){
// //             Write("AcquireLock1 failed: Should return -1 when lock index is out of bounds.\n", sizeof("AcquireLock1 failed: Should return -1 when lock index is out of bounds.\n"), 1);
// //     }
// //     acquire= AcquireLock(indexlock1);
// //     if (acquire!= -1){
// //             Write("Lock created for acquire test.\n", 77, 1);
// //     }

// //     /*
// //     set lock index to be acquired by someone else. go on wait queue.

// //     AFTER SUCCESSFULL CREATING A LOCK, THE NEXT RELEASE DOESNT WORK AS EXPECTED.
// //     */
// //     /*destroyIndex= DestroyLock(acquire1);


// // 	/* Lock is set to delete */

// // 	/* Process is not lock owner */
// // }
// // void Release_Test(){
// //     int acquire1;
// //     int release1;
// //     int lockIndex;
// //     int lockIndex2;

// //     lockIndex = CreateLock("abc", 3);
// //     lockIndex2 = CreateLock("def", 3);

// //     /*confirm in the correct process, confirm is the lock owner currently- cannot release lock*/


// // }

// void
// DestroyLock_Test() {

//     int indexlock1, indexlock2;
//     int lockIndex;
//     int destroy1;
//     int lock;
//     int acquire;
//     int release;

//     lockIndex = CreateLock("abc", 3);
//     lockIndex = CreateLock("def", 3);
//     lockIndex = CreateLock("ghi", 3);
//     lockIndex = CreateLock("jkl", 3);

//     indexlock1 = DestroyLock(-1);
//     if (indexlock1 != -1) {
//         Write("DestroyLock failed: Should return -1 for negative index.\n", 74, 1);
//     }
//     else{
//         Write("Success\n", 8, 1);
//     }
//     indexlock1 = DestroyLock(1000);
//     if (indexlock1 != -1) {
//         Write("DestroyLock failed: Should return -1 for index out of bounds.\n", 73, 1);
//     }
//     else{
//         Write("Success\n", 8, 1);
//     }
//     indexlock1= DestroyLock(lockIndex);

//     lock= CreateLock("abc", 3);
//     destroy1= DestroyLock(lock);

//     lock= CreateLock("def", 3);
//     acquire= AcquireLock(lock);
//     release= ReleaseLock(acquire);
//     destroy1= DestroyLock(release);

//     lock= CreateLock("def", 3);
//    /* acquire();
//     DestroyCV();
//     release();
//     DestroyCV();*/

//     /*LOCK OWNER
//     //ARE THERE WAITING THREADS? NO, DELETE LOCK IMMEDIATELY. YES, SET TODELETE=TRUE*/
// }
int test;
int indexCheck1, indexCheck2, indexCheck3, indexCheck4;
void t1_t1()
{
    /* checks acquire and release invalid input is handled properly.*/
    

    indexCheck1 = AcquireLock(5000);
    indexCheck2 = ReleaseLock(5000);
    indexCheck3 = AcquireLock(-1);
    indexCheck4= ReleaseLock(-1);

    /*this function is called the last among the functions that are used for the same test. print out the result and FORK next text function*/
    if(indexCheck1 == - 1 && indexCheck2 == -1 && indexCheck3 == -1 && indexCheck4 == -1) {
        Write("passed: acquire/release validates input\n", sizeof("passed: acquire/release validates input\n"), 1);
    }else{
        Write("failed: acquire/release validates input\n", sizeof("failed: acquire/release validates input\n"), 1);
    }
    Exit(0);
}
void t2_t1(){
    
    /*checks destroy and create invalid input is handled properly*/
    indexCheck1=CreateLock_Syscall("def", 1);
    indexCheck2=DestroyLock(test1+100);
    indexCheck3= CreateLock_Syscall("abc", -1);
    indexcheck4=DestroyLock(-100);
    if (indexcheck1==-1 && indexcheck2==-1 && indexCheck3==-1 && indexcheck4==-1){
        Write("passed: create/destroyLOCK validates input\n", sizeof("passed: create/destroyLOCK validates input\n"), 1);
    }
    else{
        Write("failed: create/destroyLOCK validates input\n", sizeof("failed: create/destroyLOCK validates input\n"), 1);
    }
}
void t3_t1(){
    indexcheck1=CreateCV("def", 1);
    indexcheck2=DestroyCV(test1+100);
    indexCheck= CreateCV("abc", -1);
    indexcheck4=DestroyCV(-100);
    if (indexcheck1==-1 && indexcheck2==-1 && indexCheck3==-1 && indexcheck4==-1){
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
    test=AcquireLock(indexlock1);
    test=Wait(acquire, CVIndex1);
    Write("2\n", sizeof("2\n"), 1);
    test=ReleaseLock(indexlock1);
    test=DestroyLock(indexlock1);
    test=DestroyCV(CVIndex1);
    Exit(0);
}
void t6_t2() {
    /*3rd to acquire lock*/
    test=AcquireLock(indexlock1);
    test=Wait(indexlock1, CVIndex1);
    Write("3\n", sizeof("3\n"), 1);
    test=ReleaseLock(indexlock1);
    Write("Passed broadcast test if printed numbers are in increasing order. \n End of test 1\n", sizeof("Passed if numbers are in increasing order. \n End of test 1\n"), 1);
    Exit(0);
}
void t6_t3() {
    /*1st to acquire lock*/
    test=AcquireLock(indexlock1);
    Write("1\n", sizeof("1\n"), 1);
    test=Broadcast(indexlock1, CVIndex1);
    test=ReleaseLock(indexlock1);
    Exit(0);
}

void t7_t1(){
    test=AcquireLock(indexlock1);
}
void t7_t2(){
    test=AcquireLock(indexlock1);
}
void t7_t3(){
    test=AcquireLock(indexlock1);
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
    LockIndex1= CreateLock("Lock1", 5)
    Write("Lock 1 created\n", sizeof("Lock 1 created\n"), 1);
    LockIndex2 = CreateLock("Lock2", 5);
    Write("Lock 2 created\n", sizeof("Lock 2 created\n"), 1);
    LockIndex3 = CreateLock("Lock3", 5);
    Write("Lock 3 created\n", sizeof("Lock 3 created\n"), 1);
    LockIndex4 = CreateLock("Lock4", 5);
    Write("Lock 4 created\n", sizeof("Lock 4 created\n"), 1);
    LockIndex5 = CreateLock("Lock5", 5);
    Write("Lock 5 created\n", sizeof("Lock 5 created\n"), 1);
    LockIndex6 = CreateLock("Lock6", 5);
    Write("Lock 6 created\n", sizeof("Lock 5 created\n"), 1);
    CVIndex1 = CreateCV("CV1", 3)
    Write("CV 1 created\n", sizeof("CV 1 created\n"), 1);
    CVIndex2 = CreateCV("CV2", 3);
    Write("CV 2 created\n", sizeof("CV 2 created\n"), 1);
    CVIndex3 = CreateCV("CV3", 3);
    Write("CV 3 created\n", sizeof("CV 3 created\n"), 1);
    CVIndex4 = CreateCV("CV4", 3);
    Write("CV 4 created\n", sizeof("CV 4 created\n"), 1);
    CVIndex5 = CreateCV("CV5", 3);
    Write("CV 5 created\n", sizeof("CV 5 created\n"), 1);
    CVIndex6 = CreateCV("CV6", 3);
    Write("CV 6 created\n", sizeof("CV 6 created\n"), 1);

    /*BoundsErrorCheck_Test();
     passingVars_Test();*/

    /*test1();
    test2();
    test3();
    test4();*/
    test5();
    /*test6();*/

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
