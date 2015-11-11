#include "syscall.h"


int indexcheck1, indexcheck2;

void ClientTest(){
	indexcheck1= CreateLock("abc", 3);
	indexcheck2= AcquireLock(indexcheck1);

}
int main() 
{
    ClientTest();
}