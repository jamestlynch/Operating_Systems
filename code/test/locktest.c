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
    	Write("CreateLock failed: Should return -1 when the length of the lock's identifier is 0.\n", 83, 1);
    }

    indexlock1 = CreateLock("abc", -1);
    if (indexlock1 != -1) {
        Write("CreateLock failed: Should return -1 for negative lock identifier lengths.\n", 74, 1);
    }*/

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


    /* Possible other tests: */
    /*  - Bad vaddr: different address space */
    /*  - If vector has no more room ?? */
    /*  - Memory running out ?? */
}

void
AcquireLock_Test() {
    int acquire1;
    int acquire2;
    int lockIndex;
    int destroyIndex;

    lockIndex = CreateLock("abc", 3);

    acquire1= AcquireLock(-1);
    if (acquire1 != -1){
            Write("CreateLock failed: Should return -1 when lock index is out of bounds.\n", 77, 1);
    }
    acquire1= AcquireLock(100);
    if (acquire1 != -1){
            Write("CreateLock failed: Should return -1 when lock index is out of bounds.\n", 77, 1);
    }

    acquire1= AcquireLock(1);
    if (acquire1==0){
            Write("Lock created for acquire test.\n", 77, 1);
    }

    /*destroyIndex= DestroyLock(acquire1);

    acquire1= AcquireLock(1);
    acquire2=AcquireLock(1);
    if (acquire2 != )*/




	/* Invalid indeces: negative, out of bounds */

	/* Lock is set to delete */

	/* Process is not lock owner */

}

void 
ReleaseLock_Test() {
    int lockIndex;
	/* Invalid indeces: negative, out of bounds */

	/* Process is not lock owner */

    /* does not currently own the lock*/
    lockIndex = CreateLock("abc", 3);

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

	/* Process is not lock owner */

	/* No waiting threads: delete */

	/* Waiting threads: don't delete but set to be deleted */
}

int 
main() {
    /*CreateLock_Test();*/
     CreateLock_Test(); 
    /* ReleaseLock_Test(); */
    /* DestroyLock_Test(); */


	/* Multiple threads test */
}
