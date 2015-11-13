#include "syscall.h"


int indexlock, indexcv;


void ClientTest3()
{
	indexlock = CreateLock("def", 3);
	indexlock = AcquireLock(indexlock);
	indexcv = CreateCV("testcv", sizeof("testcv"));
	Wait(indexcv, indexlock);
	Signal(indexcv, indexlock);
	ReleaseLock(indexlock);
}

int main() 
{
    ClientTest3();
}