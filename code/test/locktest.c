/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

void
CreateLock_Test() {
    // int lockIndex;

    // // If length is 0
    // lockIndex = CreateLock("abc", 0);
    // ASSERT(lockIndex == -1);

    // // If length is negative
    // lockIndex = CreateLock("abc", -1);
    // ASSERT(lockIndex == -1);

    // // Bad vaddr: invalid
    // lockIndex = CreateLock(-1, 1);
    // ASSERT(lockIndex == -1);
    
    // Bad vaddr: different address space


    // If vector has no more room ??

    // Memory running out ??
}

void
AcquireLock_Test() {
    // CreateLock

	// Invalid indeces: negative, out of bounds

	// Lock is set to delete

	// Process is not lock owner

}

void 
ReleaseLock_Test() {
	// Invalid indeces: negative, out of bounds

	// Lock is set to delete

	// Process is not lock owner
}

void
DestroyLock_Test() {
	// Invalid indeces: negative, out of bounds

	// Process is not lock owner

	// No waiting threads: delete

	// Waiting threads: don't delete but set to be deleted
}

int 
main() {
    //CreateLock_Test();
    //AcquireLock_Test();
    //ReleaseLock_Test();
    //DestroyLock_Test();


	// Multiple threads test
}
