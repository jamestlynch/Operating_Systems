#include "syscall.h"


int indexcheck1, indexcheck2;

void ClientTest(){
	indexcheck1= CreateLock("abc", 3);
	indexcheck2= AcquireLock(indexcheck1);

	Write("Client test index: %d \n",indexcheck1, 1);
}

void MVTest()
{
	int indexmv, mvvalue;

	indexmv = CreateMV("MVTest", sizeof("MVTest"), 4); /* Create a MV with ID "MVTest" and size 4 */
	SetMV(indexmv, 0, 4); /* Set MV[0] = 4 */
	mvvalue = GetMV(indexmv, 0); /* Should == 4 */

	if (mvvalue != 4)
	{
		PrintfOne("Failed [CreateAndUpdateMV] MV[0] should equal 4. Instead, equals %d",
			sizeof("Failed [CreateAndUpdateMV] MV[0] should equal 4. Instead, equals %d"),
			mvvalue);
		Halt();
	}
	else
	{
		PrintfOne("Passed [CreateAndUpdateMV]");
	}
}

int main() 
{
    /*ClientTest();*/
    MVTest();
}