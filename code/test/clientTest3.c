#include "syscall.h"


int indexcheck1, indexcheck2;


void ClientTest3(){
	indexcheck1= CreateLock("def", 3);
	indexcheck2= AcquireLock(indexcheck1);
}

<<<<<<< HEAD:code/test/clientTest3.c
int main() 
{
    ClientTest3();
=======
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

int main() 
{
    /*ClientTest();*/
    MVTest();
>>>>>>> b410c0212518da70c98a49e2f7b0022ea9e795fe:code/test/clientTest.c
}