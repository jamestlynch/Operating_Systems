
#include "syscall.h"
 int printf(const char* format, ...);

void CreateCondition_Test(){
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
    if (conditionlock1 != -1) {
        Write("CreateCondition failed: Should return -1 for bad pointers to lock identifier.\n", 73, 1);
    }
    /*
    conditionlock1 = CreateCV(-1, 1);
    if (conditionlock1 != -1) {
        Write("CreateCondition failed: Should return -1 for invalid pointers to lock identifier.\n", 77, 1);
    }

    conditionlock1 = CreateCV("abc", 3);
    if (conditionlock1 == -1) {
        Write("CreateCondition failed: Should NOT return -1 when valid lock identifier and lengths are passed in.\n", 94, 1);
    }

    conditionlock2 = CreateCV("abc", 3);
    if (conditionlock1 == conditionlock2) {
        Write("CreateCondition failed: Should NOT return the same index when creating two locks.\n", 77, 1);
    }*/
}
void Wait_Test(){

	int conditionIndex;

	conditionIndex= Wait(1, 1);
	return;

}
void Signal_Test(){

}
void Broadcast_Test(){

}
void DestroyCV_Test(){
	/*int test= DestroyCV(-1);*/

}
int 
main() {
    /*CreateCondition_Test();*/

    /*Wait_Test();*/

    /*Signal_Test();*/

    /*Broadcast_Test();*/

    CreateCondition_Test();

	/*Multiple threads test*/
}