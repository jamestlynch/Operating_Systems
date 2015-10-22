/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"
void BoundsErrorCheck_Test(){
    int indexlock1;
    int indexlock2;
    int lockIndex;
    int acquire1;

    lockIndex = CreateLock("def", 3);

    indexlock1 = CreateLock("abc", 0);
    if (indexlock1 != -1) {
    	Write("CreateLock failed: Should return -1 when the length of the lock's identifier is 0.\n", 83, 1);
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
            Write("Lock created for acquire test.\n", 77, 1);
    }
    indexlock1= ReleaseLock(300);
    if (indexlock1 != -1){
            Write("ReleaseLock0 failed: Should return -1 when lock index is out of bounds.\n", 79, 1);
    }
    indexlock1= ReleaseLock(100);
    if (indexlock1 != -1){
            Write("ReleaseLock1 failed: Should return -1 when lock index is out of bounds.\n", 79, 1);
    }
    indexlock1= ReleaseLock(200);
    if (indexlock1!=-1){
            Write("ReleaseLock2 failed: Should return -1 when lock index is out of bounds.\n", 79, 1);
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
Acquire_Test() {
    int acquire1;
    int release1;
    int lockIndex;
    int lockIndex2;

    lockIndex = CreateLock("abc", 3);
    lockIndex2 = CreateLock("def", 3);

    acquire1= AcquireLock(-1);
    if (acquire1 != -1){
            Write("AcquirLock1 failed: Should return -1 when lock index is out of bounds.\n", 77, 1);
    }
    acquire1= AcquireLock(100);
    if (acquire1 != -1){
            Write("AcquirLock1 failed: Should return -1 when lock index is out of bounds.\n", 77, 1);
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

void t5_t1(int lock) {
    DestroyLock(lock); /*should say sorry you are not owner*/
    ReleaseLock(lock); /*should say sorry you cannot release lock you do not own*/

}

void t5_t2(int lock){
    AcquireLock(lock); 
    ReleaseLock(lock);
}

int 
main() {
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
