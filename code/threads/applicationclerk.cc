//clerk.cc

#include "copyright.h"
#include "synch.h"
#include "system.h"

applicationclerk(char* debugName)	
{
	state=0;
	line=NULL:
	lineSize=-1;
	AppClerkState=FREE;
}	// initialize condition to 
					// "no one waiting"
~applicationclerk()
{

}			// deallocate the condition