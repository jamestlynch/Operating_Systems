#include "syscall.h"

int main()
{
	Exec("../test/appclerk", sizeof("../test/appclerk"));
	Exec("../test/picclerk", sizeof("../test/picclerk"));
}