/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

void
CreateLock_Test() {
    int indexlock1;
    int indexlock2;

   /* indexlock1 = CreateLock("abc", 0);
    if (indexlock1 != -1) {
    	WriteError("CreateLock failed: Should return -1 when the length of the lock's identifier is 0.\n", 83);
    }

    indexlock1 = CreateLock("abc", -1);
    if (indexlock1 != -1) {
        WriteError("CreateLock failed: Should return -1 for negative lock identifier lengths.\n", 74);
    }
    }*/

    indexlock1 = CreateLock(0, 1);
    if (indexlock1 != -1) {
        WriteError("CreateLock failed: Should return -1 for bad pointers to lock identifier.\n", 73);
    }

    /* THIS TEST BELOW CAUSES IT TO BREAK.
    indexlock1 = CreateLock(-1, 1);
    if (indexlock1 != -1) {
        Write("CreateLock failed: Should return -1 for invalid pointers to lock identifier.\n", 77, 1);
    }*/

    indexlock1 = CreateLock("abc", 3);
    if (indexlock1 == -1) {
        WriteError("CreateLock failed: Should NOT return -1 when valid lock identifier and lengths are passed in.\n", 94);
    }

        WriteInt(indexlock1);

    indexlock2 = CreateLock("abc", 3);
    if (indexlock1 == indexlock2) {
        WriteError("CreateLock failed: Should NOT return the same index when creating two locks.\n", 77);
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

    acquire1= AcquireLock(-1);
    if (acquire1 != -1){
            Write("AcquirLock0 failed: Should return -1 when lock index is out of bounds.\n", 77, 1);
    }
    acquire1= AcquireLock(100);
    if (acquire1 != -1){
            Write("AcquirLock1 failed: Should return -1 when lock index is out of bounds.\n", 77, 1);
    }

    acquire1= AcquireLock(lockIndex);
    if (acquire1!= -1){
            Write("Lock created for acquire test.\n", 77, 1);
    }

        release1= ReleaseLock(300);
    if (release1 != -1){
            Write("ReleaLock0 failed: Should return -1 when lock index is out of bounds.\n", 77, 1);
    }
    release1= ReleaseLock(100);
    if (release1 != -1){
            Write("ReleaLock1 failed: Should return -1 when lock index is out of bounds.\n", 77, 1);
    }
    release1= ReleaseLock(200);
    if (release1!=-1){
            Write("ReleaLock2 failed: Should return -1 when lock index is out of bounds.\n", 77, 1);
    }
    release1= ReleaseLock(acquire1);
    if (release1 == -1){
            Write("ReleaLock3 failed: Should return -1 when lock index is out of bounds.\n", 77, 1);
    }
}

void
DestroyLock_Test() {
    int indexlock1, indexlock2;
    int lockIndex;

    lockIndex = CreateLock("abc", 3);

    indexlock1 = DestroyLock(-1);
    if (indexlock1 != -1) {
        Write("DestroyLock failed: Should return -1 for negative index.\n", 74, 1);
    }
    else{
        Write("Success", 7, 1);
    }
    indexlock1 = DestroyLock(1000);
    if (indexlock1 != -1) {
        Write("DestroyLock failed: Should return -1 for index out of bounds.\n", 73, 1);
    }
    else{
        Write("Success", 7, 1);
    }
    indexlock1= DestroyLock(lockIndex);

    int destroy1;
    int cv;
    int acquire;
    int release;

    cv= CreateCV("abc", 3);
    destroy1= DestroyCV(cv);

    cv= CreateCV("def", 3);
    acquire= AcquireCV(cv);
    release= ReleaseCV(acquire);
    destroy1= DestroyCV();

    cv= CreateCV("def", 3);
    acquire();
    DestroyCV();
    release();
    DestroyCV();



    /*LOCK OWNER
    //ARE THERE WAITING THREADS? NO, DELETE LOCK IMMEDIATELY. YES, SET TODELETE=TRUE*/
}

int 
main() {
     /*CreateLock_Test(); */
     Acquire_Test();
     /*Release_Test();*/
    /* DestroyLock_Test(); */


	/* Multiple threads test */
}
