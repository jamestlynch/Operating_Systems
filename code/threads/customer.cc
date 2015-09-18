//customer.cc

#include "copyright.h"
#include "synch.h"
#include "system.h"

Customer::Customer(char* debugName)	{
	completedApplication=false;
	char* name= debugName;
	//int SSN= generate # sequentially increasing
	int pictureid= NULL;
	//int money= set to 100, 600, 1000, 1600
}	
Customer::~Customer(){

}			// deallocate the condition
void Customer::ClerkInteraction(){
	clerkLineLock->Acquire();//do i need to wait in line or can i walk up to counter
	if counter person is not counted as in line..check avail or busy in clerkState!
	//pick shortest line with clerk not on break
    int myLine= -1;
    int lineSize=1000;//bigger than # customers created
    for (int i=0; i<5; i++){
        if(clerkLineCount[i] < lineSize && clerkState[i] != onBreak)
               myLine=i;
               lineSize = clerkLineCount[i];
               //what if everyones on break?
               //even if line size = 0, the clerk could still be busy since being at the counter is not                                                             ‘in line'
        if(clerkState[myLine]==BUSY)
               //I must wait in line
                clerkLineCount[myLine]++;
               clerkLineCV[myLine]->wait(clerkLineLock); //critical section, clerk needs to release the lock
                clerkLineCount[myLine]--;

        else{
               //clerk is avail
               clerkState[myLine]=BUSY;
        }
     clerkLineLock->Release();
    }
//each clerk needs a regular line and a bribe line
if doBribe() ///check if the person did a bribe, function is same vars are different, same idea
}