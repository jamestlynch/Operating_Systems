#ifndef TEST2_H
#define TEST2_H

#include "copyright.h"
#include "synch.h"
#include "system.h"



Test2(char* debugName)	
{

	Lock* EntranceLineLock = new Lock("EntranceLineLock");
	Lock* PassportLineLock= new Lock("PassportLineLock");
	Lock* CashierLineLock = new Lock("CashierLineLock");
	invalid=true;
}

void Test2::UserInput(){			
	char buffer[10];
	/*
	getting user input for application clerks
	*/
	while(invalid)
	{
		invalid=false;
		memset(buffer, NULL, 10);
		printf("\nEnter number of ApplicationClerks = ");		
		scanf("%s", buffer);
		for(int x = 0; x < sizeof(buffer)/sizeof(buffer[0]); x++) 
		{
			if(!isdigit(buffer[x]) && buffer[x] != NULL)
			{
				invalid = true;
				printf("\nInvalid character. :: [%c]",buffer[x]);								
			}
			
		}
		else		
		{
			invalid=false;
			numAppClerks = atoi(buffer);	//converts to string
			if(numAppClerks < 6 && numAppClerks >0)								
				break;		
			else			
				printf("\nNot within bounds");				
		}		
	}
	
}					
~Test2()
{

}
#endif