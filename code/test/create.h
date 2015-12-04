#include "syscall.h"

int numCustomers = 5;
int numCustomersFinished = 0;


int globalSSN = CreateMV("globalSSN", sizeof("globalSSN"), 1);

int customers = CreateMV("numCustomers", sizeof("numCustomers"), numCustomers);

void InitializeData()
{
	SetMV(globalSSN, 0, 0);
}

void InitializeCustomerData ()
{
	int ssn = GetMV(globalSSN, 0);
	SetMV(globalSSN, 0, ssn + 1);

	customerData = CreateMV("customerData", sizeof("customerData"), 6 );

	SetMV(customerData, turnedInApplication, 0);
	SetMV(customerData, acceptedPassport, 0);
	SetMV(customerData, gotPassport, 0);
	SetMV(customerData, applicationFiled, 0);
	SetMV(customerData, pictureFiled, 0);
	SetMV(customerData, passportCertified, 0);
	SetMV(customerData, passportRecorded, 0);

	SetMV(customers, ssn, customerData);
}

int clerkstate; /*{ 0 = AVAILABLE, 1 = BUSY};*/

/******************************************/
/* 		  	    CLERK DATA  			  */
/******************************************/
int state;
int currentCustomer;
int money;

int lineLength;

int numAppClerks = 0;

/******************************************/
/* 		  	   ClerkGroup Data 		      */
/******************************************/

int numClerks = 5;

int lineLock;
int lineCVs[5];

int clerkLocks[5];
int workCVs[5];

/*struct ClerkGroup clerkGroups[4];*/

/* MV */
int clerkData;
int clerks;
int clerkGroups;
int clerkLock;
int line;
int people;
int appclerkdata;
int passportclerkdata;
int picclerkdata;
int cashierdata;

void InitializeClerkData (int numClerks, enum persontype clerkType)
{
	clerks = CreateMV("clerks", sizeof("clerks"), numClerks);

	clerkData = CreateMV("clerkData", sizeof("clerkData"), 9);

	SetMV(clerkData, state, 0);
	SetMV(clerkData, currentCustomer, -1);
	SetMV(clerkData, money, 0);

	SetMV(clerkData, lineLength, 0);
	SetMV(clerkData, bribeLineLength, 0);
	SetMV(clerkData, senatorLineLength, 0);

	SetMV(clerkData, numCustomersBribing, 0);
	SetMV(clerkData, customerLikedPhoto, 0);
	SetMV(clerkData, customerAppReadyToCertify, 0);
	SetMV(clerkData, customerAppReadyForPayment, 0); 
	/*clerkData(customerAppReadyForPayment)=0*/
	
	SetMV(clerks, clerkID, clerkData); /*clerks(clerkID)=clerkData*/
	/* clerks(clerkID).clerkData(customerAppReadyForPayment)=0 */

	ssn = ssnOffset + clerkID;


	peopleData = CreateMV("peopleData", sizeof("peopleData"), 2);

	SetMV(peopleData, id, clerkID);
	SetMV(peopleData, money, 0);
	SetMV(peopleData, type, clerkType);

	SetMV(people, ssn, peopleData);

	SetMV(clerkGroups, clerkType, clerks); /* clerkGroups(clerkType)=clerks; */
}

void InitializeApplicationClerkData ()
{
	enum persontype clerkType = APPLICATION;

	InitializeClerkData(numAppClerks, clerkType);

	SetMV(clerkType, numClerks, numAppClerks);
	SetMV(clerkType, moneyLock, CreateLock("ApplicationClerks-MoneyLock", 27));
	SetMV(clerkType, groupMoney, 0);
	SetMV(clerkType, lineLock, CreateLock("ApplicationClerks-LineLock", 26));

	/*clerkGroups[clerkType][numClerks] = numAppClerks;
	clerkGroups[clerkType].numClerks = numAppClerks;
	clerkGroups[clerkType].moneyLock = CreateLock("ApplicationClerks-MoneyLock", 27);
	clerkGroups[clerkType].groupMoney = 0;
	clerkGroups[clerkType].lineLock = CreateLock("ApplicationClerks-LineLock", 26);*/

	/*clerkGroups[clerkType].clerks[i].state
	int appclerkdata = GetMV(clerkGroups, clerkType);
	int appclerks = GetMV(appclerkdata, clerks);
	int clerk = GetMV(clerks, i);
	SetMV(clerk, state, BUSY);

	clerkGroups[clerkType].clerks[clerkID].senatorLineLength;
	*/


	appclerkdata = GetMV(clerkGroups, clerkType);
	line = GetMV(appclerkdata, lineCVs);
	SetMV(line, 0, CreateCV("App:0-LineCV", 12));

	appclerkdata = GetMV(clerkGroups, clerkType);
	line= GetMV(appclerkdata, lineCVs);
	SetMV(line, 1, CreateCV("App:1-LineCV", 12));

	appclerkdata = GetMV(clerkGroups, clerkType);
	line= GetMV(appclerkdata, lineCVs);
	SetMV(line, 2, CreateCV("App:2-LineCV", 12));

	appclerkdata = GetMV(clerkGroups, clerkType);
	line= GetMV(appclerkdata, lineCVs);
	SetMV(line, 3, CreateCV("App:3-LineCV", 12));

	appclerkdata = GetMV(clerkGroups, clerkType);
	line= GetMV(appclerkdata, lineCVs);
	SetMV(line, 4, CreateCV("App:4-LineCV", 12));

	appclerkdata = GetMV(clerkGroups, clerkType);
	clerkLock= GetMV(appclerkdata, clerkLocks);
	SetMV(clerkLocks, 0, CreateCV("App:0-ClerkLock", 15));

	appclerkdata = GetMV(clerkGroups, clerkType);
	clerkLock= GetMV(appclerkdata, clerkLocks);
	SetMV(clerkLocks, 1, CreateCV("App:1-ClerkLock", 15));

	appclerkdata = GetMV(clerkGroups, clerkType);
	clerkLock= GetMV(appclerkdata, clerkLocks);
	SetMV(clerkLocks, 2, CreateCV("App:2-ClerkLock", 15));

	appclerkdata = GetMV(clerkGroups, clerkType);
	clerkLock= GetMV(appclerkdata, clerkLocks);
	SetMV(clerkLocks, 3, CreateCV("App:3-ClerkLock", 15));

	appclerkdata = GetMV(clerkGroups, clerkType);
	clerkLock= GetMV(appclerkdata, clerkLocks);
	SetMV(clerkLocks, 4, CreateCV("App:4-ClerkLock", 15));

	appclerkdata = GetMV(clerkGroups, clerkType);
	line= GetMV(appclerkdata, lineCVs);
	SetMV(line, 0, CreateCV("App:0-WorkCV", 12));

	appclerkdata = GetMV(clerkGroups, clerkType);
	line= GetMV(appclerkdata, lineCVs);
	SetMV(line, 1, CreateCV("App:1-WorkCV", 12));

	appclerkdata = GetMV(clerkGroups, clerkType);
	line= GetMV(appclerkdata, lineCVs);
	SetMV(line, 2, CreateCV("App:2-WorkCV", 12));

	appclerkdata = GetMV(clerkGroups, clerkType);
	line= GetMV(appclerkdata, lineCVs);
	SetMV(line, 3, CreateCV("App:3-WorkCV", 12));

	appclerkdata = GetMV(clerkGroups, clerkType);
	line= GetMV(appclerkdata, lineCVs);
	SetMV(line, 4, CreateCV("App:4-WorkCV", 12));
}
