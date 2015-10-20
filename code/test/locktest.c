/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

void
CreateLock_Test() {
    int indexlock1, indexlock2;

    indexlock1 = CreateLock("abc", 0);
    if (indexlock1 != -1) {
    	WriteError("CreateLock failed: Should return -1 when the length of the lock's identifier is 0.\n", 83);
    }

    indexlock1 = CreateLock("abc", -1);
    if (indexlock1 != -1) {
        WriteError("CreateLock failed: Should return -1 for negative lock identifier lengths.\n", 74);
    }

    indexlock1 = CreateLock(0, 1);
    if (indexlock1 != -1) {
        WriteError("CreateLock failed: Should return -1 for bad pointers to lock identifier.\n", 73);
    }
    indexlock1 = CreateLock(-1, 1);
    if (indexlock1 != -1) {
        WriteError("CreateLock failed: Should return -1 for invalid pointers to lock identifier.\n", 77);
    }

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
AcquireLock_Test() {
    int lockIndex;

    lockIndex = CreateLock("abc", 3);

	/* Invalid indeces: negative, out of bounds */

	/* Lock is set to delete */

	/* Process is not lock owner */

}

void 
ReleaseLock_Test() {
	/* Invalid indeces: negative, out of bounds */

	/* Lock is set to delete */

	/* Process is not lock owner */
}

void
DestroyLock_Test() {
	/* Invalid indeces: negative, out of bounds */

	/* Process is not lock owner */

	/* No waiting threads: delete */

	/* Waiting threads: don't delete but set to be deleted */
}

int 
main() {
    CreateLock_Test();
    /* AcquireLock_Test(); */
    /* ReleaseLock_Test(); */
    /* DestroyLock_Test(); */


	/* Multiple threads test */
}
