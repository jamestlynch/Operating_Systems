#include "syscall.h"


int indexlock, indexcv;



void MVTest()
{
	int indexmv, mvvalue;

	indexmv = CreateMV("MVTest", sizeof("MVTest"), 4); /* Create a MV with ID "MVTest" and size 4 */
	SetMV(indexmv, 0, 4); /* Set MV[0] = 4 */
	mvvalue = GetMV(indexmv, 0); /* Should == 4 */

	if (mvvalue != 4)
	{
		PrintfOne("Failed [CreateAndUpdateMV] MV[0] should equal 4. Instead, MV[0] = %d\n", 
			sizeof("Failed [CreateAndUpdateMV] MV[0] should equal 4. Instead, MV[0] = %d\n"), 
			mvvalue);
		Halt();
	}
	else
	{
		Write("Passed [CreateAndUpdateMV]\n", sizeof("Passed [CreateAndUpdateMV]\n"), 1);
	}
}

void ClientTest3()
{
	indexlock = CreateLock("def", 3);
	indexlock = AcquireLock(indexlock);
	indexcv = CreateCV("testcv", sizeof("testcv"));
	Wait(indexcv, indexlock);
	Signal(indexcv, indexlock);
	ReleaseLock(indexlock);
}

int main() 
{
    ClientTest3();
}