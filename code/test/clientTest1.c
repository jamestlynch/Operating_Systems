#include "syscall.h"


int res1, res2, res3, res4, res5, rel1, rel2;

void ClientTest1(){
	res1= CreateLock("abc", 3);

}

int main() 
{
	ClientTest1();
}