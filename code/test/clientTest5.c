#include "syscall.h"


int indexlock, indexcv;


void ClientTest5()
{
	indexlock = CreateLock("testlock", sizeof("testlock"));
	indexlock = AcquireLock(indexlock);
	indexcv = CreateCV("testcv", sizeof("testcv"));
	Wait(indexcv, indexlock);
	Signal(indexcv, indexlock);
	ReleaseLock(indexlock);
	AcquireLock(indexlock); /* Should send a 400 response. Lock has been destroyed. */
}

int main() 
{
    ClientTest5();
}