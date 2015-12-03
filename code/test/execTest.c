#include "syscall.h"

int main()
{
	/*Exec("", -1);
	Exec("test", 4);*/
	Exec("../test/matmult", sizeof("../test/matmult"));
	Exec("../test/matmult", sizeof("../test/matmult"));
	
    /* not reached */
}
