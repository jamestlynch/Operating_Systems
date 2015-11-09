#include "syscall.h"


int indexcheck1;

void ClientTest(){
	indexcheck1= CreateLock("abc", 3);
	Write("Client test index: %d \n",indexcheck1, 1);
}
int main() 
{
    Write("Client test start\n", sizeof("Client test start\n"), 1);
    ClientTest();

}