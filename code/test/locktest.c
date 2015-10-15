/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

void
CreateLock_Test() {
    // If length is 0

    // Bad vaddr

    

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

int main() {
    CreateLock_Test();
    AcquireLock_Test();
    ReleaseLock_Test();
    DestroyLock_Test();


	// Multiple threads test
}
