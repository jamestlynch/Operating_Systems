#include "syscall.h"


int res1, res2, res3, res4, res5, acq1;

void ClientTest1(){
	res1= CreateLock("abc", 3);
	res2= CreateLock("def", 3);
	res3= CreateLock("ghi", 3);
	res4= CreateLock("jkl", 3);
	res5= CreateLock("mno", 3);
}

int main() 
{
	ClientTest1();
}