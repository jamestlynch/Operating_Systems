#include "syscall.h"


int indexcheck1, indexcheck2;


void ClientTest3(){
	indexcheck1= CreateLock("def", 3);
	indexcheck2= AcquireLock(indexcheck1);
}

int main() 
{
    ClientTest3();
}