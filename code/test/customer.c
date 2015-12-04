#include "syscall.h"
#include "create.h"

enum outputstatement { 
	Clerk_SignalledCustomer, Clerk_ReceivedSSN, Clerk_SystemJobComplete,
	Clerk_ReceivedBribe, Clerk_GoingOnBreak, Clerk_ComingOffBreak,
	Clerk_TookPicture, Clerk_ToldByCustomerDoesNotLikePicture, Clerk_ToldByCustomerDoesLikePicture,
	Clerk_DeterminedAppAndPicNotCompleted, Clerk_DeterminedAppAndPicCompleted, Clerk_VerifiedPassportCertified, 
	Clerk_ReceivedPayment, Clerk_ReceivedPaymentGoBackInLine, Clerk_ProvidedPassport, 
	Clerk_RecordedCustomerGivenPassport,
	
	Manager_WokeUpClerk, Manager_CountedMoneyForClerk, Manager_CountedTotalMoney,
	
	Customer_GotInRegularLine, Customer_GotInBribeLine, Customer_GaveSSN,
	Customer_DoesNotLikePicture, Customer_DoesLikePicture, Customer_WentTooSoon,
	Customer_PaidForPassport, Customer_GoingOutsideForSenator, Customer_LeavingPassportOffice,
};

void WriteOutput (enum outputstatement statement, enum persontype clerkType, enum persontype customerType, int ssn, int clerkID)
{
	int money; /* Only Manager print statements need money */
	money = ssn;

	switch (statement)
	{
		case Clerk_SignalledCustomer:
			switch(clerkType)
			{
				case APPLICATION:
					WriteOne("ApplicationClerk %d has signalled a Customer to come to their counter.", 
						sizeof("ApplicationClerk %d has signalled a Customer to come to their counter."), 
						clerkID);
					break;
				case PICTURE:
					WriteOne("PictureClerk %d has signalled a Customer to come to their counter.", 
						sizeof("PictureClerk %d has signalled a Customer to come to their counter."),
						clerkID);
					break;
				case PASSPORT:
					WriteOne("PassportClerk %d has signalled a Customer to come to their counter.",
						sizeof("PassportClerk %d has signalled a Customer to come to their counter."),
						clerkID);
					break;
				case CASHIER:
					WriteOne("Cashier %d has signalled a Customer to come to their counter.",
						sizeof("Cashier %d has signalled a Customer to come to their counter."),
						clerkID);
					break;
			}
			break;
		case Clerk_ReceivedSSN:
			switch(clerkType)
			{
				case APPLICATION:
					WriteTwo("ApplicationClerk %d has received SSN from Customer %d",
						sizeof("ApplicationClerk %d has received SSN from Customer %d"),
						clerkID,
						ssn);
					break;
				case PICTURE:
					WriteTwo("PictureClerk %d has received SSN from Customer %d", 
						sizeof("PictureClerk %d has received SSN from Customer %d"),
						clerkID,
						ssn);
					break;
				case PASSPORT:
					WriteTwo("PassportClerk %d has received SSN from Customer %d",
						sizeof("PassportClerk %d has received SSN from Customer %d"),
						clerkID,
						ssn);
					break;
				case CASHIER:
					WriteTwo("Cashier %d has received SSN from Customer %d",
						sizeof("Cashier %d has received SSN from Customer %d"),
						clerkID,
						ssn);
					break;
			}
			break;
		case Clerk_GoingOnBreak:
			switch(clerkType)
			{
				case APPLICATION:
					WriteOne("ApplicationClerk %d is going on break",
						sizeof("ApplicationClerk %d is going on break"),
						clerkID);
					break;
				case PICTURE:
					WriteOne("PictureClerk %d is going on break",
						sizeof("PictureClerk %d is going on break"),
						clerkID);
					break;
				case PASSPORT:
					WriteOne("PassportClerk %d is going on break",
						sizeof("PassportClerk %d is going on break"),
						clerkID);
					break;
				case CASHIER:
					WriteOne("Cashier %d is going on break",
						sizeof("Cashier %d is going on break"),
						clerkID);
					break;
			}
			break;
		case Clerk_ComingOffBreak:
			switch(clerkType)
			{
				case APPLICATION:
					WriteOne("ApplicationClerk %d is coming off break",
						sizeof("ApplicationClerk %d is coming off break"),
						clerkID);
					break;
				case PICTURE:
					WriteOne("PictureClerk %d is coming off break",
						sizeof("PictureClerk %d is coming off break"),
						clerkID);
					break;
				case PASSPORT:
					WriteOne("PassportClerk %d is coming off break",
						sizeof("PassportClerk %d is coming off break"),
						clerkID);
					break;
				case CASHIER:
					WriteOne("Cashier %d is coming off break",
						sizeof("Cashier %d is coming off break"),
						clerkID);
					break;
			}
			break;
		case Clerk_SystemJobComplete:
			switch(clerkType)
			{
				case APPLICATION:	
					switch(customerType)
					{
						case CUSTOMER:
							WriteTwo("ApplicationClerk %d has recorded a completed application for Customer %d",
								sizeof("ApplicationClerk %d has recorded a completed application for Customer %d"),
								clerkID,
								ssn);
							break;
						case SENATOR:
							WriteTwo("ApplicationClerk %d has recorded a completed application for Senator %d",
								sizeof("ApplicationClerk %d has recorded a completed application for Senator %d"),
								clerkID,
								ssn);
							break;
					}
					break;
				case PICTURE:	
					switch(customerType)
					{
						case CUSTOMER:
							WriteTwo("PictureClerk %d has filed a picture for Customer %d",
								sizeof("PictureClerk %d has filed a picture for Customer %d"),
								clerkID,
								ssn);
							break;
						case SENATOR:
							WriteTwo("PictureClerk %d has filed a picture for Senator %d",
								sizeof("PictureClerk %d has filed a picture for Senator %d"),
								clerkID,
								ssn);
							break;
					}
					break;
				case PASSPORT:	
					switch(customerType)
					{
						case CUSTOMER:
							WriteTwo("PassportClerk %d has recorded Customer %d passport documentation",
								sizeof("PassportClerk %d has recorded Customer %d passport documentation"),
								clerkID,
								ssn);
							break;
						case SENATOR:
							WriteTwo("PassportClerk %d has recorded Senator %d passport documentation",
								sizeof("PassportClerk %d has recorded Senator %d passport documentation"),
								clerkID,
								ssn);
							break;
					}
					break;
			}
			break;
		case Clerk_ReceivedBribe:
			WriteTwo("ApplicationClerk %d has received $500 from Customer %d",
				sizeof("ApplicationClerk %d has received $500 from Customer %d"),
				clerkID,
				ssn);
			break;
		case Clerk_TookPicture:
			WriteTwo("PictureClerk %d has taken a picture of Customer %d",
				sizeof("PictureClerk %d has taken a picture of Customer %d"),
				clerkID,
				ssn);
			break;
		case Clerk_ToldByCustomerDoesNotLikePicture:
			WriteTwo("PictureClerk %d has been told that Customer %d does not like their picture",
				sizeof("PictureClerk %d has been told that Customer %d does not like their picture"),
				clerkID,
				ssn);
			break;
		case Clerk_ToldByCustomerDoesLikePicture:
			WriteTwo("PictureClerk %d has been told that Customer %d does like their picture",
				sizeof("PictureClerk %d has been told that Customer %d does like their picture"),
				clerkID,
				ssn);
			break;
		case Clerk_DeterminedAppAndPicNotCompleted:
			WriteTwo("PassportClerk %d has determined that Customer %d does not have both their application and picture completed",
				sizeof("PassportClerk %d has determined that Customer %d does not have both their application and picture completed"),
				clerkID,
				ssn);
			break;
		case Clerk_DeterminedAppAndPicCompleted:
			WriteTwo("PassportClerk %d has determined that Customer %d has both their application and picture completed",
				sizeof("PassportClerk %d has determined that Customer %d has both their application and picture completed"),
				clerkID,
				ssn);
			break;
		case Clerk_VerifiedPassportCertified:
			WriteTwo("Cashier %d has verified that Customer %d has been certified by a PassportClerk",
				sizeof("Cashier %d has verified that Customer %d has been certified by a PassportClerk"),
				clerkID,
				ssn);
			break;
		case Clerk_ReceivedPayment:
			WriteTwo("Cashier %d has received the $100 from Customer %d after certification",
				sizeof("Cashier %d has received the $100 from Customer %d after certification"),
				clerkID,
				ssn);
			break;
		case Clerk_ReceivedPaymentGoBackInLine:
			WriteTwo("Cashier %d has received the $100 from Customer %d before certification. They are to go to the back of my line.",
				sizeof("Cashier %d has received the $100 from Customer %d before certification. They are to go to the back of my line."),
				clerkID,
				ssn);
			break;
		case Clerk_ProvidedPassport:
			WriteTwo("Cashier %d has provided Customer %d their completed passport",
				sizeof("Cashier %d has provided Customer %d their completed passport"),
				clerkID,
				ssn);
			break;
		case Clerk_RecordedCustomerGivenPassport:
			WriteTwo("Cashier %d has recorded that Customer %d has been given their completed passport",
				sizeof("Cashier %d has recorded that Customer %d has been given their completed passport"),
				clerkID,
				ssn);
			break;
	
		
		case Manager_WokeUpClerk:
			switch(clerkType)
			{
				case APPLICATION:
					Write("Manager has woken up an ApplicationClerk",
						sizeof("Manager has woken up an ApplicationClerk"),
						1);
					break;
				case PICTURE:
					Write("Manager has woken up an PictureClerk",
						sizeof("Manager has woken up an PictureClerk"),
						1);
					break;
				case PASSPORT:
					Write("Manager has woken up an PassportClerk",
						sizeof("Manager has woken up an PassportClerk"),
						1);
					break;
				case CASHIER:
					Write("Manager has woken up an Cashier",
						sizeof("Manager has woken up an Cashier"),
						1);
					break;
			}
			break;
		case Manager_CountedMoneyForClerk:
			switch(clerkType)
			{
				case APPLICATION:
					WriteOne("Manager has counted a total of $%d for ApplicationClerks",
						sizeof("Manager has counted a total of $%d for ApplicationClerks"),
						money);
					break;
				case PICTURE:
					WriteOne("Manager has counted a total of $%d for PictureClerks",
						sizeof("Manager has counted a total of $%d for PictureClerks"),
						money);
					break;
				case PASSPORT:
					WriteOne("Manager has counted a total of $%d for PassportClerks",
						sizeof("Manager has counted a total of $%d for PassportClerks"),
						money);
					break;
				case CASHIER:
					WriteOne("Manager has counted a total of $%d for Cashiers",
						sizeof("Manager has counted a total of $%d for Cashiers"),
						money);
					break;
			}
			break;
		case Manager_CountedTotalMoney:
			WriteOne("Manager has counted a total of $%d for the passport office",
				sizeof("Manager has counted a total of $%d for the passport office"),
				money);
			break;
	
	
		case Customer_GotInRegularLine:
			switch(clerkType)
			{
				case APPLICATION:
					switch(customerType)
					{
						case CUSTOMER:
							WriteTwo("Customer %d has gotten in regular line for ApplicationClerk %d.",
								sizeof("Customer %d has gotten in regular line for ApplicationClerk %d."),
								ssn,
								clerkID);
							break;
						case SENATOR:
							WriteTwo("Senator %d has gotten in regular line for ApplicationClerk %d.",
								sizeof("Senator %d has gotten in regular line for ApplicationClerk %d."),
								ssn,
								clerkID);
							break;
					}
					break;
				case PICTURE:
					switch(customerType)
					{
						case CUSTOMER:
							WriteTwo("Customer %d has gotten in regular line for PictureClerk %d.",
								sizeof("Customer %d has gotten in regular line for PictureClerk %d."),
								ssn,
								clerkID);
							break;
						case SENATOR:
							WriteTwo("Senator %d has gotten in regular line for PictureClerk %d.",
								sizeof("Senator %d has gotten in regular line for PictureClerk %d."),
								ssn,
								clerkID);
							break;
					}
					break;
				case PASSPORT:
					switch(customerType)
					{
						case CUSTOMER:
							WriteTwo("Customer %d has gotten in regular line for PassportClerk %d.",
								sizeof("Customer %d has gotten in regular line for PassportClerk %d."),
								ssn,
								clerkID);
							break;
						case SENATOR:
							WriteTwo("Senator %d has gotten in regular line for PassportClerk %d.",
								sizeof("Senator %d has gotten in regular line for PassportClerk %d."),
								ssn,
								clerkID);
							break;
					}
					break;
				case CASHIER:
					switch(customerType)
					{
						case CUSTOMER:
							WriteTwo("Customer %d has gotten in regular line for Cashier %d.",
								sizeof("Customer %d has gotten in regular line for Cashier %d."),
								ssn,
								clerkID);
							break;
						case SENATOR:
							WriteTwo("Senator %d has gotten in regular line for Cashier %d.",
								sizeof("Senator %d has gotten in regular line for Cashier %d."),
								ssn,
								clerkID);
							break;
					}
					break;
			}
			break;
		case Customer_GotInBribeLine:
			switch(clerkType)
			{
				case APPLICATION:
					WriteTwo("Customer %d has gotten in bribe line for ApplicationClerk %d.",
						sizeof("Customer %d has gotten in bribe line for ApplicationClerk %d."),
						ssn,
						clerkID);
					break;
				case PICTURE:
					WriteTwo("Customer %d has gotten in bribe line for PictureClerk %d.",
						sizeof("Customer %d has gotten in bribe line for PictureClerk %d."),
						ssn,
						clerkID);
					break;
				case PASSPORT:
					WriteTwo("Customer %d has gotten in bribe line for PassportClerk %d.",
						sizeof("Customer %d has gotten in bribe line for PassportClerk %d."),
						ssn,
						clerkID);
					break;
				case CASHIER:
					WriteTwo("Customer %d has gotten in bribe line for Cashier %d.",
						sizeof("Customer %d has gotten in bribe line for Cashier %d."),
						ssn,
						clerkID);
					break;
			}
			break;
		case Customer_GaveSSN:
			switch(clerkType)
			{
				case APPLICATION:
					switch(customerType)
					{
						case CUSTOMER:
							WriteTwo("Customer %d has given SSN to ApplicationClerk %d.", 
								sizeof("Customer %d has given SSN to ApplicationClerk %d."),
								ssn,
								clerkID);
							break;
						case SENATOR:
							WriteTwo("Senator %d has given SSN to ApplicationClerk %d.",
								sizeof("Senator %d has given SSN to ApplicationClerk %d."),
								ssn,
								clerkID);
							break;
					}
					break;
				case PICTURE:
					switch(customerType)
					{
						case CUSTOMER:
							WriteTwo("Customer %d has given SSN to PictureClerk %d.",
								sizeof("Customer %d has given SSN to PictureClerk %d."),
								ssn,
								clerkID);
							break;
						case SENATOR:
							WriteTwo("Senator %d has given SSN to PictureClerk %d.",
								sizeof("Senator %d has given SSN to PictureClerk %d."),
								ssn, 
								clerkID);
							break;
					}
					break;
				case PASSPORT:
					switch(customerType)
					{
						case CUSTOMER:
							WriteTwo("Customer %d has given SSN to PassportClerk %d.",
								sizeof("Customer %d has given SSN to PassportClerk %d."),
								ssn,
								clerkID);
							break;
						case SENATOR:
							WriteTwo("Senator %d has given SSN to PassportClerk %d.",
								sizeof("Senator %d has given SSN to PassportClerk %d."),
								ssn,
								clerkID);
							break;
					}
					break;
				case CASHIER:
					switch(customerType)
					{
						case CUSTOMER:
							WriteTwo("Customer %d has given SSN to Cashier %d.",
								sizeof("Customer %d has given SSN to Cashier %d."),
								ssn,
								clerkID);
							break;
						case SENATOR:
							WriteTwo("Senator %d has given SSN to Cashier %d.",
								sizeof("Senator %d has given SSN to Cashier %d."),
								ssn,
								clerkID);
							break;
					}
					break;
			}
			break;
		case Customer_DoesNotLikePicture:
			switch(customerType)
			{
				case CUSTOMER:
					WriteTwo("Customer %d does not like their picture from PictureClerk %d.",
						sizeof("Customer %d does not like their picture from PictureClerk %d."),
						ssn,
						clerkID);
					break;
				case SENATOR:
					WriteTwo("Senator %d does not like their picture from PictureClerk %d.",
						sizeof("Senator %d does not like their picture from PictureClerk %d."),
						ssn,
						clerkID);
					break;
			}
			break;
		case Customer_DoesLikePicture:
			switch(customerType)
			{
				case CUSTOMER:
					WriteTwo("Customer %d does like their picture from PictureClerk %d.",
						sizeof("Customer %d does like their picture from PictureClerk %d."),
						ssn,
						clerkID);
					break;
				case SENATOR:
					WriteTwo("Senator %d does like their picture from PictureClerk %d.",
						sizeof("Senator %d does like their picture from PictureClerk %d."),
						ssn,
						clerkID);
					break;
			}
			break;
		case Customer_WentTooSoon:
			switch(clerkType)
			{
				case PASSPORT:
					switch(customerType)
					{
						case CUSTOMER:
							WriteTwo("Customer %d has gone to PassportClerk %d too soon. They are going to the back of the line.",
								sizeof("Customer %d has gone to PassportClerk %d too soon. They are going to the back of the line."),
								ssn,
								clerkID);
							break;
						case SENATOR:
							WriteTwo("Senator %d has gone to PassportClerk %d too soon. They are going to the back of the line.",
								sizeof("Senator %d has gone to PassportClerk %d too soon. They are going to the back of the line."),
								ssn,
								clerkID);
							break;
					}
					break;
				case CASHIER:
					switch(customerType)
					{
						case CUSTOMER:
							WriteTwo("Customer %d has gone to Cashier %d too soon. They are going to the back of the line.",
								sizeof("Customer %d has gone to Cashier %d too soon. They are going to the back of the line."),
								ssn,
								clerkID);
							break;
						case SENATOR:
							WriteTwo("Senator %d has gone to Cashier %d too soon. They are going to the back of the line.",
								sizeof("Senator %d has gone to Cashier %d too soon. They are going to the back of the line."),
								ssn,
								clerkID);
							break;
					}
					break;
			break;
		case Customer_PaidForPassport:
			switch(customerType)
			{
				case CUSTOMER:
					WriteTwo("Customer %d has given Cashier %d $100.",
						sizeof("Customer %d has given Cashier %d $100."),
						ssn,
						clerkID);
					break;
				case SENATOR:
					WriteTwo("Senator %d has given Cashier %d $100.",
						sizeof("Senator %d has given Cashier %d $100."),
						ssn,
						clerkID);
					break;
			}
			break;
		case Customer_GoingOutsideForSenator:
			WriteOne("Customer %d is going outside the Passport Office because their is a Senator present.",
				sizeof("Customer %d is going outside the Passport Office because their is a Senator present."),
				ssn);
			break;
		case Customer_LeavingPassportOffice:
			switch(customerType)
			{
				case CUSTOMER:
					WriteOne("Customer %d is leaving the Passport Office.",
						sizeof("Customer %d is leaving the Passport Office."),
						ssn);
					break;
				case SENATOR:
					WriteOne("Senator %d is leaving the Passport Office.",
						sizeof("Senator %d is leaving the Passport Office."),
						ssn);
					break;
			}
			break;
		}
	}
}

int DecideClerk (int ssn, enum persontype clerkType)
{
	int numClerks;
	int lineLock;

	int clerkID; /* Going to iterate over all clerks to make line decision */
	int currentClerk = -1;
	int currentLineLength = 1000;
	int shortestLine = -1;
	int shortestLineLength = 1000;
	int clerkLineLength; /* Keeps track of either normal line count or senator line count depending on if isSenator */

	/*numClerks = clerkGroups[clerkType].numClerks;

	clerkGroups[clerkType].clerks[i].state
	int appclerkdata = GetMV(clerkGroups, clerkType);
	int appclerks = GetMV(appclerkdata, clerks);
	int clerk = GetMV(clerks, i);
	SetMV(clerk, state, BUSY);


	clerkGroups[clerkType].clerks[i]


	clerkGroups[clerkType].clerks


	lineLock = clerkGroups[clerkType].lineLock;*/

	numClerks= getMV(clerkType, numClerks);
	lineLock= getMV(clerkType, lineLock);

	/*  TODO: Arrays were cast as unsigned ints, how to convert/cast back to array; or just how to access data inside array, otherwise? */

	/* Check if the senator is present, and if so, "go outside" by waiting on the CV. */
	/* 	By placing this here, we ensure line order remains consistent, conveniently. */

	/*AcquireLock(senatorIndoorLock);
	if (isSenatorPresent && people[ssn].type != SENATOR)
	{
		WriteOutput(Customer_GoingOutsideForSenator, clerkType, CUSTOMER, ssn, clerkID);
		Wait(senatorIndoorCV, senatorIndoorLock);
	}
	ReleaseLock(senatorIndoorLock);*/

	AcquireLock(lineLock);
	for (clerkID = 0; clerkID < numClerks; clerkID++)
	{
		/* Different lines depending on whether customer or senator. */
		int person= GetMV(people, ssn);
		int t = GETMV(person, type);
		
		if (t == SENATOR)
		{
			int appclerkdata = GetMV(clerkGroups, clerkType);
			int appclerks = GetMV(clerkdata, clerks);
			int clerk = GetMV(clerks, i);
			SetMV(clerk, senatorLineLength, BUSY);

			clerkLineLength = clerkGroups[clerkType].clerks[clerkID].senatorLineLength;
		}
		else
		{
			clerkLineLength = clerkGroups[clerkType].clerks[clerkID].lineLength;
		}

		/* If clerk is available, go there. */
		if (clerkLineLength == 0 && clerkGroups[clerkType].clerks[clerkID].state == AVAILABLE)
		{
			currentClerk = clerkID;
			currentLineLength = clerkLineLength;
			break;
		}

		/* Pick the shortest line of clerks not on break. */
		if (clerkLineLength < shortestLineLength && clerkGroups[clerkType].clerks[clerkID].state != ONBREAK)
		{
			currentClerk = clerkID;
			currentLineLength = clerkLineLength;
		}

		/* If everyone is on break, need to know which line is shortest so we join it. Managers will eventually wake clerks up. */
		if (clerkLineLength < shortestLineLength)
		{
			shortestLine = clerkID;
			shortestLineLength = clerkLineLength;
		}
	}

	/* If everyone was on break, set your line to be the shortest line. */
	if (currentClerk == -1)
	{
		currentClerk = shortestLine;
		currentLineLength = shortestLineLength;
	}

	return currentClerk;
}

void WaitInLine (int ssn, int clerkID, enum persontype clerkType)
{	
	int lineLock;
	int lineCV;
	int bribeLineCV;
	int senatorLineCV;
	int clerkLock;
	int workCV;
	int bribeCV;

	linelock= getMV(clerkType, lineLock);
	lineCV= getMV(clerkType, lineCV);

	lineLock = clerkGroups[clerkType].lineLock;
	lineCV = clerkGroups[clerkType].lineCVs[clerkID];
	bribeLineCV = clerkGroups[clerkType].bribeLineCVs[clerkID];
	senatorLineCV = clerkGroups[clerkType].senatorLineCVs[clerkID];
	clerkLock = clerkGroups[clerkType].clerkLocks[clerkID];
	workCV = clerkGroups[clerkType].workCVs[clerkID];
	bribeCV = clerkGroups[clerkType].bribeCVs[clerkID];

	/* Now that customer has selected a clerk's line, figure out whether to go: */
	/* 	- Straight to the counter */
	/* 	- In line */
	/*	- Bribe */
	/*	- Senator line (if isSenator) */

	if (clerkGroups[clerkType].clerks[clerkID].state != AVAILABLE)
	{ 
		/* Clerk is unavailable; Rule out going straight to counter, so now decide which line to wait in. */
		if (people[ssn].type == SENATOR)
		{
			/* TODO: Can we do direct incrementation on array value? */
			clerkGroups[clerkType].clerks[clerkID].senatorLineLength++;
			WriteOutput(Customer_GotInRegularLine, clerkType, SENATOR, ssn, clerkID);
			Wait(senatorLineCV, lineLock);
			/* TODO: See above. */
			clerkGroups[clerkType].clerks[clerkID].senatorLineLength--;
		}
		else
		{ 
			/* Ruled out straight to counter and senator line. Bribe or no bribe? */
			if (clerkGroups[clerkType].clerks[clerkID].state != ONBREAK && people[ssn].money >= 600 && clerkGroups[clerkType].clerks[clerkID].lineLength >= 1)
			{ /* If customer has enough money and she's not in a line for a clerk that is on break, always bribe. */
				/* Let clerk know you are trying to bribe her. */
				/* TODO: See above. */
				clerkGroups[clerkType].clerks[clerkID].numCustomersBribing++;
				Wait(bribeCV, lineLock);
				/* TODO: See above. */
				clerkGroups[clerkType].clerks[clerkID].numCustomersBribing--;
				ReleaseLock(lineLock);

				/* Do customer side of bribe. */
				AcquireLock(clerkLock);

				clerkGroups[clerkType].clerks[clerkID].currentCustomer = ssn;
				people[ssn].money -= 500; /* Pay $500 for bribe */
				Signal(workCV, lineLock);
				Wait(workCV, lineLock);
				WriteOutput(Customer_GotInBribeLine, clerkType, CUSTOMER, ssn, clerkID);
				
				ReleaseLock(clerkLock);

				/* Now get into bribe line. */
				AcquireLock(lineLock);

				/* TODO: See above. (increment operator) */
				clerkGroups[clerkType].clerks[clerkID].bribeLineLength++;
				Wait(bribeLineCV, lineLock);

				/* Woken up. Make sure no senators have entered so that I can do my business with clerk. */
				AcquireLock(senatorIndoorLock);
				if (isSenatorPresent && people[ssn].type != SENATOR)
				{
					WriteOutput(Customer_GoingOutsideForSenator, clerkType, CUSTOMER, ssn, clerkID);
					Wait(senatorIndoorCV, senatorIndoorLock);
					ReleaseLock(senatorIndoorLock); /* Lock gets reacquired inside of Wait, release it and re-decide line. */
					return WaitInLine(ssn, clerkID, clerkType);
				}
				else
				{
					ReleaseLock(senatorIndoorLock);
				}

				/* Made it out of line. */
				/* TODO: See above. (Decrement operator) */
				clerkGroups[clerkType].clerks[clerkID].bribeLineLength--;
			}
			else
			{ /* No other options. Get in regular line. */
				/* TODO: See above. (Increment operator) */
				clerkGroups[clerkType].clerks[clerkID].lineLength++;
				WriteOutput(Customer_GotInRegularLine, clerkType, CUSTOMER, ssn, clerkID);
				Wait(lineCV, lineLock);

				/* Woken up. Make sure no senators have entered so that I can do my business with clerk. */
				AcquireLock(senatorIndoorLock);
				if (isSenatorPresent && people[ssn].type != SENATOR)
				{
					WriteOutput(Customer_GoingOutsideForSenator, clerkType, CUSTOMER, ssn, clerkID);
					Wait(senatorIndoorCV, senatorIndoorLock);
					ReleaseLock(senatorIndoorLock); /* Lock gets reacquired inside of Wait, release it and re-decide line. */
					return WaitInLine(ssn, clerkID, clerkType);
				}
				else
				{
					ReleaseLock(senatorIndoorLock);
				}

				/* Made it out of line. */
				/* TODO: See above. (Decrement operator) */
				clerkGroups[clerkType].clerks[clerkID].lineLength--;
			}
		}
	}
	else
	{ 
		/* Line was empty when customer joined, go straight to the counter. */
		clerkGroups[clerkType].clerks[clerkID].state = BUSY;
	}
	ReleaseLock(lineLock);

	return;
}

void GetBackInLine (int ssn, enum persontype clerkType);

void MakePhotoDecision (int ssn, int clerkID, enum persontype clerkType)
{
	int clerkLock;
	int workCV;

	int amountLiked;

	clerkLock = clerkGroups[clerkType].clerkLocks[clerkID];
	workCV = clerkGroups[clerkType].workCVs[clerkID];

	/* Picture Clerk already took my picture, decide if I like it. */

	/* TODO: Random syscall. See above. */
	/*	amountLiked = Random(1, 100); */
	amountLiked = 100;
	if (amountLiked > 50) 
	{
		/* I liked the photo */
		WriteOutput(Customer_DoesLikePicture, PICTURE, people[ssn].type, ssn, clerkID);
		clerkGroups[PICTURE].clerks[clerkID].customerLikedPhoto = true;
	} 
	else
	{
	    /* I did not like the photo, going to get back in line. */
	   	WriteOutput(Customer_DoesNotLikePicture, PICTURE, people[ssn].type, ssn, clerkID);
	    clerkGroups[PICTURE].clerks[clerkID].customerLikedPhoto = false;

	    Signal(workCV, clerkLock);
	    ReleaseLock(clerkLock);

	    GetBackInLine(ssn, clerkType);
	}
}

void PayForPassport (int ssn, int clerkID)
{
	int clerkLock;
	int workCV;

	clerkLock = clerkGroups[CASHIER].clerkLocks[clerkID];
	workCV = clerkGroups[CASHIER].workCVs[clerkID];

	people[ssn].money -= 100;

	WriteOutput(Customer_PaidForPassport, PASSPORT, people[ssn].type, ssn, clerkID);
	Signal(workCV, clerkLock);
	Wait(workCV, clerkLock);
}

void PunishTooSoon (int ssn, int clerkID, enum persontype clerkType)
{
	/* TODO: Moved the customer leaving the clerk BEFORE punishing, verify this is correct. */
	int clerkLock;
	int workCV;

	int punishmentTime;
	int i;

	clerkLock = clerkGroups[clerkType].clerkLocks[clerkID];
	workCV = clerkGroups[clerkType].workCVs[clerkID];

	Signal(workCV, clerkLock);
	ReleaseLock(clerkLock);

	/* TODO: Random syscall. See above. */
	/*	punishmentTime = Random(100, 1000); */
	punishmentTime = 100;

	for (i = 0; i < punishmentTime; i++)
	{
		Yield();
	}

	GetBackInLine(ssn, clerkType);
}

void CustomerInteraction (int ssn, int clerkID, enum persontype clerkType)
{
	int clerkLock;
	int workCV;

	clerkLock = clerkGroups[clerkType].clerkLocks[clerkID];
	workCV = clerkGroups[clerkType].workCVs[clerkID];

	AcquireLock(clerkLock);
	clerkGroups[clerkType].clerks[clerkID].currentCustomer = ssn;
	WriteOutput(Customer_GaveSSN, clerkType, people[ssn].type, ssn, clerkID);
	Signal(workCV, clerkLock);
	Wait(workCV, clerkLock);

	switch (clerkType) 
	{
		case APPLICATION:
			break; /* No extra work for Filing Application (for customer) */
		case PICTURE:
			MakePhotoDecision(ssn, clerkID, PICTURE);
			break;
		case PASSPORT:
			if(clerkGroups[clerkType].clerks[clerkID].customerAppReadyToCertify == false)
			{
				PunishTooSoon(ssn, clerkID, PASSPORT);
			}
			break;
		case CASHIER:
			if(clerkGroups[clerkType].clerks[clerkID].customerAppReadyForPayment == false)
			{
				PunishTooSoon(ssn, clerkID, CASHIER);
			}
			else
			{
				PayForPassport(ssn, clerkID);
			}
			break;
	}

	Signal(workCV, clerkLock);
	ReleaseLock(clerkLock);
}

void GetBackInLine (int ssn, enum persontype clerkType)
{
	int clerkID;

	clerkID = DecideClerk(ssn, clerkType);
	WaitInLine(ssn, clerkID, clerkType);
	CustomerInteraction(ssn, clerkID, clerkType);
}

void Leave (int ssn)
{
	numCustomersFinished++;

	if (people[ssn].type == SENATOR)
	{
		WriteOutput(Customer_LeavingPassportOffice, SENATOR, SENATOR, ssn, ssn);
		AcquireLock(senatorIndoorLock);
		isSenatorPresent = false;
		Broadcast(senatorIndoorCV, senatorIndoorLock);
		ReleaseLock(senatorIndoorLock);
	}

	else
	{
		WriteOutput(Customer_LeavingPassportOffice, CUSTOMER, CUSTOMER, ssn, ssn);
	}

	Exit(0);
}

void Customer ()
{
	int ssn;
	struct Person customer;
	int applicationFirst;
	int clerkID;

	/*AcquireLock(paramLock);*/

	ssn = threadParam;
	ReleaseLock(paramLock);
	WriteOne("ssn inside customer: %d", sizeof("ssn inside customer: %d"), ssn);

	customer = people[ssn];

	/* Customer (randomly) decides whether she wants to file application */
	/* 	or take picture first. */
	/* TODO: Random syscall. */
	applicationFirst = 1;

	AcquireLock(senatorIndoorLock);
	if (isSenatorPresent)
	{
		Wait(senatorIndoorCV, senatorIndoorLock);
	}
	else if (customer.type == SENATOR)
	{
		isSenatorPresent = true;
	}
	ReleaseLock(senatorIndoorLock);

	if (applicationFirst > 50)
	{
		/* Go to Application Clerk */
		clerkID = DecideClerk(ssn, APPLICATION);
		WaitInLine(ssn, clerkID, APPLICATION);
		CustomerInteraction(ssn, clerkID, APPLICATION);
		
		/* Go to Picture Clerk */
		clerkID = DecideClerk(ssn, PICTURE);
		WaitInLine(ssn, clerkID, PICTURE);
		CustomerInteraction(ssn, clerkID, PICTURE);
	}
	else
	{
		/* Go to Picture Clerk */
		clerkID = DecideClerk(ssn, PICTURE);
		WaitInLine(ssn, clerkID, PICTURE);
		CustomerInteraction(ssn, clerkID, PICTURE);
		
		/* Go to Application Clerk */
		clerkID = DecideClerk(ssn, APPLICATION);
		WaitInLine(ssn, clerkID, APPLICATION);
		CustomerInteraction(ssn, clerkID, APPLICATION);
	}

	/* Go to Passport Clerk */
	clerkID = DecideClerk(ssn, PASSPORT);
	WaitInLine(ssn, clerkID, PASSPORT);
	CustomerInteraction(ssn, clerkID, PASSPORT);

	/* Go to Cashier */
	clerkID = DecideClerk(ssn, CASHIER);
	WaitInLine(ssn, clerkID, CASHIER);
	CustomerInteraction(ssn, clerkID, CASHIER);
}

int main()
{
	InitializeApplicationClerkData();
	InitializePictureClerkData();
	InitializeCustomerData();

	/*while (){ customer has not left passport office
		Yield();
	}*/

	Exit(0);
}