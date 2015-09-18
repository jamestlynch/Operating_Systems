#include "copyright.h"
#include "synch.h"
#include "system.h"

Clerk(char* debugName)	
{
		clerkState=AVAILABLE;
		name=debugName;
		waitqueue=NULL;
}
~Clerk()
{
	delete waitqueue;
}