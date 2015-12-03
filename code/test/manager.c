#include "syscall.h"

struct Manager {
	int appclerkMoney;
	int picclerkMoney;
	int passportclerkMoney;
	int cashierMoney;
	int totalMoney;
};

struct Manager manager;
void doCreate()
{

}

void InitializeManager ()
{
	int ssn;

	manager.appclerkMoney = 0;
	manager.picclerkMoney = 0;
	manager.passportclerkMoney = 0;
	manager.cashierMoney = 0;
	manager.totalMoney = 0;

	ssn = numCustomers + numSenators + numAppClerks + numPicClerks + numPassportClerks + numCashiers;

	people[ssn].id = 0;
	people[ssn].money = 0;
	people[ssn].type = MANAGER;
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
	while(true)
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
}
int main() 
{
	doCreate();
	InitializeManager();
	Manager();
}