#include "syscall.h"


int test1, test2, test3, test4, test5, test6;


void ClientTest2(){
	test1= AcquireLock(1);
	

	Exit(0);
}

int main() 
{
	ClientTest2();
}