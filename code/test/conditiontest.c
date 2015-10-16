
#include "syscall.h"
 int printf(const char* format, ...);

void CreateCondition_Test(){
	  

	int conditionIndex;
    /*If length is 0
     conditionIndex = CreateCV("abc", 0);
     if (conditionIndex== -1 ){
     	
     }
     /*ASSERT(conditionIndex == -1);*/

    /*If length is negative
     conditionIndex = CreateCV("abc", -1);
     /*ASSERT(conditionIndex == -1);
     if (conditionIndex== -1 ){
     	
     }

    /*Bad vaddr: invalid
     conditionIndex = CreateCV(-1, 1);
     /*ASSERT(conditionIndex == -1);*/
     if (conditionIndex== -1 ){
     	
     }
    
    conditionIndex = CreateCV("abcdef", 6);

    /* Bad vaddr: different address space


    // If vector has no more room ??

    // Memory running out ??*/
}
void Wait_Test(){
	
	int conditionIndex;

	conditionIndex= Wait(-1);
	return;

}
void Signal_Test(){

}
void Broadcast_Test(){

}
int 
main() {
    /*CreateCondition_Test();*/

    Wait_Test();

    /*Signal_Test();

    //Broadcast_Test();

	// Multiple threads test*/
}