/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

void
CreateLock_Test() {
    int lockIndex;

    /* If length is 0
     *	Error Message: Length for lock's identifier name must be nonzero and positive */
    lockIndex = CreateLock("abc", 0);
    if (lockIndex == -1) {
    	Write("Test failed: CreateLock should return -1 when the length of the lock's identifier is 0.", 87, 1);
    }

    /* If length is negative
     *	Error Message: Length for lock's identifier name must be nonzero and positive */
    lockIndex = CreateLock("abc", -1);
    /* assert(lockIndex == -1); */

    /* Bad vaddr: null
     *	Error Message: Bad pointer passed to create new lock */
    lockIndex = CreateLock(0, 1);
    /* assert(lockIndex == -1); */

    /* Bad vaddr: invalid
     *	Error Message: Bad pointer passed to create new lock */
    lockIndex = CreateLock(-1, 1);
    /* assert(lockIndex == -1); */
    
    /* Bad vaddr: different address space */


    /* If vector has no more room ?? */

    /* Memory running out ?? */
}

void
AcquireLock_Test() {
    /* CreateLock */

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
