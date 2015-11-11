#include "syscall.h"


int test1, test2, test3, test4, test5, test6;


void ClientTest2(){
	test1= AcquireLock(0);
	test2= AcquireLock(1);
	test3= AcquireLock(2);
	test4= AcquireLock(3);
	test5= AcquireLock(4);
	test6= AcquireLock(5);
	Exit(0);

}

int main() 
{
	ClientTest2();
}