//manager.cc

#include "copyright.h"
#include "synch.h"
#include "system.h"

Manager::Manager(char* debugName)	{
	overalltotal=0;
	appclerktotal=0;
	picclerktotal=0;
	passportclerktotal=0;
	cashiertotal=0;
	name=debugName;

}	
// initialize condition to "no one waiting"
Manager::~Manager(){

}
void Manager::ClerkInteraction(enum clerkType, ){
	//for (int i=0; i<clerkState.size; i++)
		//if (clerkstateline[i].size >=3){
		//	WAKE UP CLERK
		//}
}
void Manager::PrintMoney(){
	Printf("Overall Current Total: %s\n", overalltotal);
	Printf("Application Clerk Current Total: %s\n", appclerktotal);
	Printf("Picture Clerk Current Total: %s\n", picclerktotal);
	Printf("Passport Clerk Current Total: %s\n", passportclerkltotal);
	Printf("Cashier Clerk Current Total: %s\n", cashiertotal);
}

// deallocate the condition