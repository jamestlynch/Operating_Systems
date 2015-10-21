#include "syscall.h"
#include "syscall.h"

void CreateCondition_Test()
{
	int conditionlock1, conditionlock2;

    /*conditionlock1 = CreateCV("abc", -1);
    if (conditionlock1 != -1) {
        Write("CreateCondition failed: Should return -1 for negative lock identifier lengths.\n", 74, 1);
    }

    conditionlock1 = CreateCV("abc", 0);
    if (conditionlock1 != -1) {
        Write("CreateCondition failed: Should return -1 when the length of the lock's identifier is 0.\n", 83, 1);
    }*/

    conditionlock1 = CreateCV(0, 1);
    if (conditionlock1 != -1) 
    {
        Write("CreateCondition failed: Should return -1 for bad pointers to lock identifier.\n", 73, 1);
    }


    /* THIS TEST BELOW IS PROBLEMATIC, MAKES NACHOS EXIT WITH ERRORs

    conditionlock1 = CreateCV(-1, 1);
    if (conditionlock1 != -1) {
        Write("CreateCondition failed: Should return -1 for invalid pointers to lock identifier.\n", 77, 1);
    }
    */

    conditionlock1 = CreateCV("abc", 3);
    if (conditionlock1 == -1) {
        Write("CreateCondition failed: Should NOT return -1 when valid lock identifier and lengths are passed in.\n", 94, 1);
    }

    conditionlock2 = CreateCV("abc", 3);
    if (conditionlock1 == conditionlock2) {
        Write("CreateCondition failed: Should NOT return the same index when creating two locks.\n", 77, 1);
    }
}

void Wait_Test() 
{

	int conditionIndex;

	conditionIndex = Wait(-1, -1);
    if (conditionIndex != -1){
        Write("Wait_Locks failed: Should return -1 for bad pointers to lock identifier.\n", 73, 1);
    }

    conditionIndex= Wait(-1, 1);
     if (conditionIndex != -1){
        Write("Wait_Locks failed: Should return -1 for bad pointers to lock identifier.\n", 73, 1);
    }

    conditionIndex= Wait(1, -1);
     if (conditionIndex != -1){
        Write("Wait_Locks failed: Should return -1 for bad pointers to lock identifier.\n", 73, 1);
    }
	return;

}
void Signal_Test(){
    int signal;

}
void Broadcast_Test(){
    int cv;
    int acq;

    cv= CreateCV("abc", 3);
    /*cv= CreateCV*/

}
void DestroyCV_Test()
{
	int destroy1;
    int cv;
    int acquire;
    int release;

    cv= CreateCV("abc", 3);
    destroy1= DestroyCV(cv);

    cv= CreateCV("def", 3);
    acquire();
    release();

    cv= CreateCV("def", 3);
    acquire();
    DestroyCV();
    release();
}

int main() {
    /*CreateCondition_Test();*/

    Wait_Test();

    /*Signal_Test();*/

    /*Broadcast_Test();*/

	/*Multiple threads test*/
}