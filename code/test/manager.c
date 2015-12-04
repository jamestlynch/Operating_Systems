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

int CollectMoney (enum persontype clerkType)
{
	int moneyLock;
	int groupMoney;

	moneyLock = clerkGroups[clerkType].moneyLock;
	groupMoney = 0;

	AcquireLock(moneyLock);
	groupMoney = clerkGroups[clerkType].groupMoney;
	ReleaseLock(moneyLock);	

	return groupMoney;
}

void TakeClerksOffBreak (enum persontype clerkType)
{
	int clerkID;
	int numClerks;
	int wakeUpClerk;
	int clerksOnBreak;
	int lineLock;

	numClerks = clerkGroups[clerkType].numClerks;
	wakeUpClerk = 0;
	clerksOnBreak = 0;
	lineLock = clerkGroups[clerkType].lineLock;

	for (clerkID = 0; clerkID < numClerks; clerkID++)
	{
		/* If I have more than 3 people in my line, need to wake up another clerk. */
		if (clerkGroups[clerkType].clerks[clerkID].lineLength >= 3 && clerkGroups[clerkType].clerks[clerkID].state != ONBREAK)
		{
			wakeUpClerk++;
		}

		/* Keep track of how many clerks are on break. */
		if (clerkGroups[clerkType].clerks[clerkID].state == ONBREAK)
		{
			clerksOnBreak++;
		}
	}

	for (clerkID = 0; clerkID < numClerks; clerkID++)
	{
		/* If all clerks are on break, but they have people in their line, wake up that clerk. */
		if (clerksOnBreak == numClerks && clerkGroups[clerkType].clerks[clerkID].lineLength > 0)
		{
			clerkGroups[clerkType].clerks[clerkID].state = BUSY;

			Signal(clerkGroups[clerkType].breakCVs[clerkID], lineLock);
			WriteOutput(Manager_WokeUpClerk, clerkType, MANAGER, clerkID, clerkID);
			continue;
		}

		/* If a clerk is on break, and another clerk has 3+ people in their line, wake up that clerk. */
		if (clerkGroups[clerkType].clerks[clerkID].state == ONBREAK && wakeUpClerk > 0)
		{
			wakeUpClerk--;

			clerkGroups[clerkType].clerks[clerkID].state = BUSY;
			Signal(clerkGroups[clerkType].breakCVs[clerkID], lineLock);
			WriteOutput(Manager_WokeUpClerk, clerkType, MANAGER, clerkID, clerkID);
		}
	}
}

int ManageClerk (enum persontype clerkType)
{
	int lineLock;
	int numClerks;
	int groupMoney;

	lineLock = clerkGroups[clerkType].lineLock;

	AcquireLock(lineLock);

	groupMoney = CollectMoney(clerkType);

	TakeClerksOffBreak(clerkType);

	ReleaseLock(lineLock);

	return groupMoney;
}

void Manager ()
{
	int previousTotal;

	WriteOutput(Manager_CountedMoneyForClerk, APPLICATION, MANAGER, manager.appclerkMoney, -1);
	WriteOutput(Manager_CountedMoneyForClerk, PICTURE, MANAGER, manager.picclerkMoney, -1);
	WriteOutput(Manager_CountedMoneyForClerk, PASSPORT, MANAGER, manager.passportclerkMoney, -1);
	WriteOutput(Manager_CountedMoneyForClerk, CASHIER, MANAGER, manager.cashierMoney, -1);
	WriteOutput(Manager_CountedTotalMoney, MANAGER, MANAGER, manager.totalMoney, -1);

	do
	{
		previousTotal = manager.totalMoney;

		manager.appclerkMoney = ManageClerk(APPLICATION);
		manager.picclerkMoney = ManageClerk(PICTURE);
		manager.passportclerkMoney = ManageClerk(PASSPORT);
		manager.cashierMoney = ManageClerk(CASHIER);

		manager.totalMoney = manager.appclerkMoney + manager.picclerkMoney + manager.passportclerkMoney + manager.cashierMoney;

		if (previousTotal != manager.totalMoney || numCustomersFinished == (numCustomers + numSenators))
		{
			WriteOutput(Manager_CountedMoneyForClerk, APPLICATION, MANAGER, manager.appclerkMoney, -1);
			WriteOutput(Manager_CountedMoneyForClerk, PICTURE, MANAGER, manager.picclerkMoney, -1);
			WriteOutput(Manager_CountedMoneyForClerk, PASSPORT, MANAGER, manager.passportclerkMoney, -1);
			WriteOutput(Manager_CountedMoneyForClerk, CASHIER, MANAGER, manager.cashierMoney, -1);
			WriteOutput(Manager_CountedTotalMoney, MANAGER, MANAGER, manager.totalMoney, -1);
		}

		Yield();
	} while (numCustomersFinished < (numCustomers + numSenators));

	Exit(0);
}
int main() 
{
	doCreate();
	InitializeManager();
	Manager();
}