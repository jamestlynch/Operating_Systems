#include "syscall.h"


int indexlock, indexcv;


void ClientTest4()
{
	indexlock = 0; /* Another machine created lock at index 0 */
	AcquireLock(indexlock);
	Signal(indexcv, indexlock);
	Wait(indexcv, indexlock);
}

int main() 
{
    ClientTest4();
}