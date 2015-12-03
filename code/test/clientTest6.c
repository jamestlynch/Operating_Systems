#include "syscall.h"


int indexlock, indexcv;


void ClientTest6()
{
	indexlock = 0; /* Another machine created lock at index 0 */
	indexcv = 0; /* Another machine created cv at index 0 */
	AcquireLock(indexlock);
	Signal(indexcv, indexlock);
	DestroyLock(indexlock);
	DestroyCV(indexcv);
}

int main() 
{
    ClientTest6();
}