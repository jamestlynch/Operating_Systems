#include "syscall.h"


int res1, res2, res3, res4, res5, rel1, rel2;

void ClientTest1(){
	res1= CreateLock("abc", 3);
	res2= CreateLock("def", 3);
	res3= CreateLock("ghi", 3);
	res4= CreateLock("jkl", 3);
	res5= CreateLock("mno", 3);
	rel1= AcquireLock(1);
	rel2= ReleaseLock(1);
}

int main() 
{
	ClientTest1();
}