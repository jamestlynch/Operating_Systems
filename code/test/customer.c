#include "syscall.h"

int main()
{
	Write("Inside application clerk", sizeof("Inside application clerk"), 1);
	Exit(0);
}