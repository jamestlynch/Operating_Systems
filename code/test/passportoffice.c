/*	passportoffice.c
 */

#include "syscall.h"

typedef enum { false = 0, true = 1 } bool;

/* ========================================================================================================================================= */
/*																																			 */
/*		DATA SETUP																															 */
/*																																			 */
/* ========================================================================================================================================= */

/******************************************/
/* 	        Thread Creation Data 		  */
/******************************************/

/* Nachos fork does not allow parameters to be passed in to new threads. 	*/
/* 	To get around this, we need the following: 								*/
/*	-	threadParam - the value we would otherwise be passing in. (In the	*/
/*			PPOffice this will always be an identifier for getting the full	*/
/*			set of data about a thread (i.e., the ssn for the people array 	*/
/*			the jobID for the jobs array)).									*/
/*	-	paramLock - this synchronizes the threadParam so that if a thread  	*/
/*			gets context switched while forking a new thread, it can finish */
/*			creating the thread with the proper data when it resumes. 		*/
int threadParam;
int paramLock;

/******************************************/
/* 		  PPOffice Simulation Data   	  */
/******************************************/

/* We need a way of knowing whether the simulation has finished without 	*/
/*	busy waiting. We achieve this by adding the following: 					*/
/*	-	numCustomersFinished - keeps track of amount finished so we know 	*/
/*		  when to Signal that the simulation can terminate.					*/
/*	-	allCustomersFinishedCV – the condition of whether or not there are  */
/*		  any Customers still moving through the simulation. 				*/
/*		  numCustomersFinished == (numCustomers + numSenators) 				*/
/*	-	numCustomersFinishedLock - synchronize updates/reads of 			*/
/*		  numCustomersFinished.												*/
/*	Note: It may be more *correct* to acquire the numCustomersFinishedLock 	*/
/* 	  inside of the Clerks/Manager loops, but we do not do that. While it 	*/
/*	  would save a "waisted" loop where the clerks are doing work they do  	*/
/*	  not need to do, it would create deadlock whenever a Clerk gets 		*/
/*	  context switched in the middle of checking the condition. This likely */
/*	  occurs more often than the number of waisted loops.					*/
int numCustomersFinished = 0;
int allCustomersFinishedCV;
int numCustomersFinishedLock;

/******************************************/
/* 		  	    Person Data 			  */
/******************************************/

enum persontype { APPLICATION, PICTURE, PASSPORT, CASHIER, CUSTOMER, SENATOR, MANAGER };

struct Person {
	int id;
	int money;
	enum persontype type;
};

struct Person people[81]; /* numCustomers + numSenators + numAppClerks + numPicClerks + numPassportClerks + numCashiers + numManagers */

/******************************************/
/* 		  	   Customer Data 			  */
/******************************************/

struct Customer {
	bool turnedInApplication;
	bool acceptedPassport;
	bool gotPassport;
	bool applicationFiled;
	bool pictureFiled;
	bool passportCertified;
	bool passportRecorded;
};

int moneyOptions[4] = {100, 600, 1100, 1600};

int InitialMoney()
{
	int moneyIndex;

	/* TODO: Random syscall. */
	/* 	moneyIndex = Random(0, 4); */
	moneyIndex = 1;

	return moneyOptions[moneyIndex];
}


int numCustomers = 50;
struct Customer customers[60]; /* Same info for customers/senators: SIZE = numCustomers + numSenators */

void InitializeCustomerData ()
{
	int ssn;

	for (ssn = 0; ssn < numCustomers; ssn++)
	{ 
		/* customers hold all shared data that clerks update */
		customers[ssn].turnedInApplication = false;
		customers[ssn].acceptedPassport = false;
		customers[ssn].gotPassport = false;
		customers[ssn].applicationFiled = false;
		customers[ssn].pictureFiled = false;
		customers[ssn].passportCertified = false;
		customers[ssn].passportRecorded = false;

		/* people holds all private data that people must update and make public to clerks. */
		people[ssn].id = ssn;
		people[ssn].money = InitialMoney();
		people[ssn].type = CUSTOMER;
	}
}

/******************************************/
/* 		  	    Senator Data 			  */
/******************************************/

int numSenators = 10;
struct Customer senators[10];
/* Senators are stored inside of customers array. See above. */

int isSenatorPresent = 0; /* Used to determine whether or not customers should wait for senators to leave PPOffice */

int senatorPresentLock; /* Synchronizes isSenatorPresent */
int senatorPresentCV; /* Wait on this whenever a senator is inside PPOffice */
/* TODO: Is this necessary? */
/*	int senatorOutdoorCV; */

void InitializeSenatorData ()
{
	int ssn;
	int ssnOffset;

	ssnOffset = numCustomers;

	for (ssn = ssnOffset; ssn < numSenators; ssn++)
	{
		/* customers hold all shared data that clerks update */
		customers[ssn].turnedInApplication = false;
		customers[ssn].acceptedPassport = false;
		customers[ssn].gotPassport = false;
		customers[ssn].applicationFiled = false;
		customers[ssn].pictureFiled = false;
		customers[ssn].passportCertified = false;
		customers[ssn].passportRecorded = false;

		/* people holds all private data that people must update and make public to clerks. */
		people[ssn].id = ssn;
		people[ssn].money = InitialMoney();
		people[ssn].type = SENATOR;
	}

	senatorPresentLock = CreateLock("SenatorPresentLock", sizeof("SenatorPresentLock"));
	senatorPresentCV = CreateCV("SenatorPresentCV", sizeof("SenatorPresentCV"));
	/* TODO: Is senatorOutdoorCV necessary? */
	/*	senatorOutdoorCV = CreateCV("SenatorOutdoorCV", 16); */
}

/******************************************/
/* 			    Clerk Data 				  */
/******************************************/

enum clerkstate { AVAILABLE, BUSY, ONBREAK };
enum clerkinteraction { DOINTERACTION, TAKEBREAK, ACCEPTBRIBE };
enum linetype { NORMALLINE, BRIBELINE, SENATORLINE };

struct Clerk {
	enum clerkstate state;
	int currentCustomer;
	int money;

	int lineLength;
	int bribeLineLength;
	int senatorLineLength;

	int numCustomersBribing;
	bool customerLikedPhoto;
	bool customerAppReadyToCertify;
	bool customerAppReadyForPayment;
};

int numAppClerks = 5;
int numPicClerks = 5;
int numPassportClerks = 5;
int numCashiers = 5;

struct ClerkGroup {
	struct Clerk clerks[5];
	int numClerks;

	int moneyLock;
	int groupMoney;
	
	int lineLock;
	int lineCVs[5];
	int bribeLineCVs[5];
	int senatorLineCVs[5];

	int clerkLocks[5];
	int workCVs[5];
	int bribeCVs[5];
	int breakCVs[5];
};

struct ClerkGroup clerkGroups[4];

void InitializeClerkData (int numClerks, enum persontype clerkType)
{
	int clerkID;
	int ssn;
	int ssnOffset;
	
	ssnOffset = (clerkType * 5) + numCustomers + numSenators;

	for (clerkID = 0; clerkID < numClerks; clerkID++)
	{
		clerkGroups[clerkType].clerks[clerkID].state = AVAILABLE;
		clerkGroups[clerkType].clerks[clerkID].currentCustomer = -1;
		clerkGroups[clerkType].clerks[clerkID].money = 0;

		clerkGroups[clerkType].clerks[clerkID].lineLength = 0;
		clerkGroups[clerkType].clerks[clerkID].bribeLineLength = 0;
		clerkGroups[clerkType].clerks[clerkID].senatorLineLength = 0;

		clerkGroups[clerkType].clerks[clerkID].numCustomersBribing = 0;
		clerkGroups[clerkType].clerks[clerkID].customerLikedPhoto = false;
		clerkGroups[clerkType].clerks[clerkID].customerAppReadyToCertify = false;
		clerkGroups[clerkType].clerks[clerkID].customerAppReadyForPayment = false;

		ssn = ssnOffset + clerkID;

		people[ssn].id = clerkID;
		people[ssn].money = 0;
		people[ssn].type = clerkType;
	}
}

void InitializeApplicationClerkData ()
{
	enum persontype clerkType = APPLICATION;

	InitializeClerkData(numAppClerks, clerkType);

	clerkGroups[clerkType].numClerks = numAppClerks;

	clerkGroups[clerkType].moneyLock = CreateLock("ApplicationClerks-MoneyLock", 27);
	clerkGroups[clerkType].groupMoney = 0;

	clerkGroups[clerkType].lineLock = CreateLock("ApplicationClerks-LineLock", 26);

	clerkGroups[clerkType].lineCVs[0] = CreateCV("App:0-LineCV", 12);
	clerkGroups[clerkType].lineCVs[1] = CreateCV("App:1-LineCV", 12);
	clerkGroups[clerkType].lineCVs[2] = CreateCV("App:2-LineCV", 12);
	clerkGroups[clerkType].lineCVs[3] = CreateCV("App:3-LineCV", 12);
	clerkGroups[clerkType].lineCVs[4] = CreateCV("App:4-LineCV", 12);

	clerkGroups[clerkType].bribeLineCVs[0] = CreateCV("App:0-BribeLineCV", 17);
	clerkGroups[clerkType].bribeLineCVs[1] = CreateCV("App:1-BribeLineCV", 17);
	clerkGroups[clerkType].bribeLineCVs[2] = CreateCV("App:2-BribeLineCV", 17);
	clerkGroups[clerkType].bribeLineCVs[3] = CreateCV("App:3-BribeLineCV", 17);
	clerkGroups[clerkType].bribeLineCVs[4] = CreateCV("App:4-BribeLineCV", 17);

	clerkGroups[clerkType].senatorLineCVs[0] = CreateCV("App:0-SenatorLineCV", 19);
	clerkGroups[clerkType].senatorLineCVs[1] = CreateCV("App:1-SenatorLineCV", 19);
	clerkGroups[clerkType].senatorLineCVs[2] = CreateCV("App:2-SenatorLineCV", 19);
	clerkGroups[clerkType].senatorLineCVs[3] = CreateCV("App:3-SenatorLineCV", 19);
	clerkGroups[clerkType].senatorLineCVs[4] = CreateCV("App:4-SenatorLineCV", 19);

	clerkGroups[clerkType].clerkLocks[0] = CreateLock("App:0-ClerkLock", 15);
	clerkGroups[clerkType].clerkLocks[1] = CreateLock("App:1-ClerkLock", 15);
	clerkGroups[clerkType].clerkLocks[2] = CreateLock("App:2-ClerkLock", 15);
	clerkGroups[clerkType].clerkLocks[3] = CreateLock("App:3-ClerkLock", 15);
	clerkGroups[clerkType].clerkLocks[4] = CreateLock("App:4-ClerkLock", 15);

	clerkGroups[clerkType].workCVs[0] = CreateCV("App:0-WorkCV", 12);
	clerkGroups[clerkType].workCVs[1] = CreateCV("App:1-WorkCV", 12);
	clerkGroups[clerkType].workCVs[2] = CreateCV("App:2-WorkCV", 12);
	clerkGroups[clerkType].workCVs[3] = CreateCV("App:3-WorkCV", 12);
	clerkGroups[clerkType].workCVs[4] = CreateCV("App:4-WorkCV", 12);

	clerkGroups[clerkType].bribeCVs[0] = CreateCV("App:0-BribeCV", 13);
	clerkGroups[clerkType].bribeCVs[1] = CreateCV("App:1-BribeCV", 13);
	clerkGroups[clerkType].bribeCVs[2] = CreateCV("App:2-BribeCV", 13);
	clerkGroups[clerkType].bribeCVs[3] = CreateCV("App:3-BribeCV", 13);
	clerkGroups[clerkType].bribeCVs[4] = CreateCV("App:4-BribeCV", 13);

	clerkGroups[clerkType].breakCVs[0] = CreateCV("App:0-BreakCV", 13);
	clerkGroups[clerkType].breakCVs[1] = CreateCV("App:1-BreakCV", 13);
	clerkGroups[clerkType].breakCVs[2] = CreateCV("App:2-BreakCV", 13);
	clerkGroups[clerkType].breakCVs[3] = CreateCV("App:3-BreakCV", 13);
	clerkGroups[clerkType].breakCVs[4] = CreateCV("App:4-BreakCV", 13);
}

void InitializePictureClerkData ()
{
	enum persontype clerkType = PICTURE;

	InitializeClerkData(numPicClerks, clerkType);

	clerkGroups[clerkType].numClerks = numPicClerks;

	clerkGroups[clerkType].moneyLock = CreateLock("PictureClerks-MoneyLock", 23);
	clerkGroups[clerkType].groupMoney = 0;

	clerkGroups[clerkType].lineLock = CreateLock("PictureClerks-LineLock", 22);

	clerkGroups[clerkType].lineCVs[0] = CreateCV("Pic:0-LineCV", 12);
	clerkGroups[clerkType].lineCVs[1] = CreateCV("Pic:1-LineCV", 12);
	clerkGroups[clerkType].lineCVs[2] = CreateCV("Pic:2-LineCV", 12);
	clerkGroups[clerkType].lineCVs[3] = CreateCV("Pic:3-LineCV", 12);
	clerkGroups[clerkType].lineCVs[4] = CreateCV("Pic:4-LineCV", 12);

	clerkGroups[clerkType].bribeLineCVs[0] = CreateCV("Pic:0-BribeLineCV", 17);
	clerkGroups[clerkType].bribeLineCVs[1] = CreateCV("Pic:1-BribeLineCV", 17);
	clerkGroups[clerkType].bribeLineCVs[2] = CreateCV("Pic:2-BribeLineCV", 17);
	clerkGroups[clerkType].bribeLineCVs[3] = CreateCV("Pic:3-BribeLineCV", 17);
	clerkGroups[clerkType].bribeLineCVs[4] = CreateCV("Pic:4-BribeLineCV", 17);

	clerkGroups[clerkType].senatorLineCVs[0] = CreateCV("Pic:0-SenatorLineCV", 19);
	clerkGroups[clerkType].senatorLineCVs[1] = CreateCV("Pic:1-SenatorLineCV", 19);
	clerkGroups[clerkType].senatorLineCVs[2] = CreateCV("Pic:2-SenatorLineCV", 19);
	clerkGroups[clerkType].senatorLineCVs[3] = CreateCV("Pic:3-SenatorLineCV", 19);
	clerkGroups[clerkType].senatorLineCVs[4] = CreateCV("Pic:4-SenatorLineCV", 19);

	clerkGroups[clerkType].clerkLocks[0] = CreateLock("Pic:0-ClerkLock", 15);
	clerkGroups[clerkType].clerkLocks[1] = CreateLock("Pic:1-ClerkLock", 15);
	clerkGroups[clerkType].clerkLocks[2] = CreateLock("Pic:2-ClerkLock", 15);
	clerkGroups[clerkType].clerkLocks[3] = CreateLock("Pic:3-ClerkLock", 15);
	clerkGroups[clerkType].clerkLocks[4] = CreateLock("Pic:4-ClerkLock", 15);

	clerkGroups[clerkType].workCVs[0] = CreateCV("Pic:0-WorkCV", 12);
	clerkGroups[clerkType].workCVs[1] = CreateCV("Pic:1-WorkCV", 12);
	clerkGroups[clerkType].workCVs[2] = CreateCV("Pic:2-WorkCV", 12);
	clerkGroups[clerkType].workCVs[3] = CreateCV("Pic:3-WorkCV", 12);
	clerkGroups[clerkType].workCVs[4] = CreateCV("Pic:4-WorkCV", 12);

	clerkGroups[clerkType].bribeCVs[0] = CreateCV("Pic:0-BribeCV", 13);
	clerkGroups[clerkType].bribeCVs[1] = CreateCV("Pic:1-BribeCV", 13);
	clerkGroups[clerkType].bribeCVs[2] = CreateCV("Pic:2-BribeCV", 13);
	clerkGroups[clerkType].bribeCVs[3] = CreateCV("Pic:3-BribeCV", 13);
	clerkGroups[clerkType].bribeCVs[4] = CreateCV("Pic:4-BribeCV", 13);

	clerkGroups[clerkType].breakCVs[0] = CreateCV("Pic:0-BreakCV", 13);
	clerkGroups[clerkType].breakCVs[1] = CreateCV("Pic:1-BreakCV", 13);
	clerkGroups[clerkType].breakCVs[2] = CreateCV("Pic:2-BreakCV", 13);
	clerkGroups[clerkType].breakCVs[3] = CreateCV("Pic:3-BreakCV", 13);
	clerkGroups[clerkType].breakCVs[4] = CreateCV("Pic:4-BreakCV", 13);
}

void InitializePassportClerkData ()
{
	enum persontype clerkType = PASSPORT;

	InitializeClerkData(numPassportClerks, clerkType);

	clerkGroups[clerkType].numClerks = numPassportClerks;

	clerkGroups[clerkType].moneyLock = CreateLock("PassportClerks-MoneyLock", 24);
	clerkGroups[clerkType].groupMoney = 0;

	clerkGroups[clerkType].lineLock = CreateLock("PassportClerks-LineLock", 23);

	clerkGroups[clerkType].lineCVs[0] = CreateCV("Pas:0-LineCV", 12);
	clerkGroups[clerkType].lineCVs[1] = CreateCV("Pas:1-LineCV", 12);
	clerkGroups[clerkType].lineCVs[2] = CreateCV("Pas:2-LineCV", 12);
	clerkGroups[clerkType].lineCVs[3] = CreateCV("Pas:3-LineCV", 12);
	clerkGroups[clerkType].lineCVs[4] = CreateCV("Pas:4-LineCV", 12);

	clerkGroups[clerkType].bribeLineCVs[0] = CreateCV("Pas:0-BribeLineCV", 17);
	clerkGroups[clerkType].bribeLineCVs[1] = CreateCV("Pas:1-BribeLineCV", 17);
	clerkGroups[clerkType].bribeLineCVs[2] = CreateCV("Pas:2-BribeLineCV", 17);
	clerkGroups[clerkType].bribeLineCVs[3] = CreateCV("Pas:3-BribeLineCV", 17);
	clerkGroups[clerkType].bribeLineCVs[4] = CreateCV("Pas:4-BribeLineCV", 17);

	clerkGroups[clerkType].senatorLineCVs[0] = CreateCV("Pas:0-SenatorLineCV", 19);
	clerkGroups[clerkType].senatorLineCVs[1] = CreateCV("Pas:1-SenatorLineCV", 19);
	clerkGroups[clerkType].senatorLineCVs[2] = CreateCV("Pas:2-SenatorLineCV", 19);
	clerkGroups[clerkType].senatorLineCVs[3] = CreateCV("Pas:3-SenatorLineCV", 19);
	clerkGroups[clerkType].senatorLineCVs[4] = CreateCV("Pas:4-SenatorLineCV", 19);

	clerkGroups[clerkType].clerkLocks[0] = CreateLock("Pas:0-ClerkLock", 15);
	clerkGroups[clerkType].clerkLocks[1] = CreateLock("Pas:1-ClerkLock", 15);
	clerkGroups[clerkType].clerkLocks[2] = CreateLock("Pas:2-ClerkLock", 15);
	clerkGroups[clerkType].clerkLocks[3] = CreateLock("Pas:3-ClerkLock", 15);
	clerkGroups[clerkType].clerkLocks[4] = CreateLock("Pas:4-ClerkLock", 15);

	clerkGroups[clerkType].workCVs[0] = CreateCV("Pas:0-WorkCV", 12);
	clerkGroups[clerkType].workCVs[1] = CreateCV("Pas:1-WorkCV", 12);
	clerkGroups[clerkType].workCVs[2] = CreateCV("Pas:2-WorkCV", 12);
	clerkGroups[clerkType].workCVs[3] = CreateCV("Pas:3-WorkCV", 12);
	clerkGroups[clerkType].workCVs[4] = CreateCV("Pas:4-WorkCV", 12);

	clerkGroups[clerkType].bribeCVs[0] = CreateCV("Pas:0-BribeCV", 13);
	clerkGroups[clerkType].bribeCVs[1] = CreateCV("Pas:1-BribeCV", 13);
	clerkGroups[clerkType].bribeCVs[2] = CreateCV("Pas:2-BribeCV", 13);
	clerkGroups[clerkType].bribeCVs[3] = CreateCV("Pas:3-BribeCV", 13);
	clerkGroups[clerkType].bribeCVs[4] = CreateCV("Pas:4-BribeCV", 13);

	clerkGroups[clerkType].breakCVs[0] = CreateCV("Pas:0-BreakCV", 13);
	clerkGroups[clerkType].breakCVs[1] = CreateCV("Pas:1-BreakCV", 13);
	clerkGroups[clerkType].breakCVs[2] = CreateCV("Pas:2-BreakCV", 13);
	clerkGroups[clerkType].breakCVs[3] = CreateCV("Pas:3-BreakCV", 13);
	clerkGroups[clerkType].breakCVs[4] = CreateCV("Pas:4-BreakCV", 13);
}

void InitializeCashierData ()
{
	enum persontype clerkType = CASHIER;

	InitializeClerkData(numCashiers, clerkType);

	clerkGroups[clerkType].numClerks = numCashiers;

	clerkGroups[clerkType].moneyLock = CreateLock("Cashiers-MoneyLock", 18);
	clerkGroups[clerkType].groupMoney = 0;

	clerkGroups[clerkType].lineLock = CreateLock("Cashiers-LineLock", 17);

	clerkGroups[clerkType].lineCVs[0] = CreateCV("Csh:0-LineCV", 12);
	clerkGroups[clerkType].lineCVs[1] = CreateCV("Csh:1-LineCV", 12);
	clerkGroups[clerkType].lineCVs[2] = CreateCV("Csh:2-LineCV", 12);
	clerkGroups[clerkType].lineCVs[3] = CreateCV("Csh:3-LineCV", 12);
	clerkGroups[clerkType].lineCVs[4] = CreateCV("Csh:4-LineCV", 12);

	clerkGroups[clerkType].bribeLineCVs[0] = CreateCV("Csh:0-BribeLineCV", 17);
	clerkGroups[clerkType].bribeLineCVs[1] = CreateCV("Csh:1-BribeLineCV", 17);
	clerkGroups[clerkType].bribeLineCVs[2] = CreateCV("Csh:2-BribeLineCV", 17);
	clerkGroups[clerkType].bribeLineCVs[3] = CreateCV("Csh:3-BribeLineCV", 17);
	clerkGroups[clerkType].bribeLineCVs[4] = CreateCV("Csh:4-BribeLineCV", 17);

	clerkGroups[clerkType].senatorLineCVs[0] = CreateCV("Csh:0-SenatorLineCV", 19);
	clerkGroups[clerkType].senatorLineCVs[1] = CreateCV("Csh:1-SenatorLineCV", 19);
	clerkGroups[clerkType].senatorLineCVs[2] = CreateCV("Csh:2-SenatorLineCV", 19);
	clerkGroups[clerkType].senatorLineCVs[3] = CreateCV("Csh:3-SenatorLineCV", 19);
	clerkGroups[clerkType].senatorLineCVs[4] = CreateCV("Csh:4-SenatorLineCV", 19);

	clerkGroups[clerkType].clerkLocks[0] = CreateLock("Csh:0-ClerkLock", 15);
	clerkGroups[clerkType].clerkLocks[1] = CreateLock("Csh:1-ClerkLock", 15);
	clerkGroups[clerkType].clerkLocks[2] = CreateLock("Csh:2-ClerkLock", 15);
	clerkGroups[clerkType].clerkLocks[3] = CreateLock("Csh:3-ClerkLock", 15);
	clerkGroups[clerkType].clerkLocks[4] = CreateLock("Csh:4-ClerkLock", 15);

	clerkGroups[clerkType].workCVs[0] = CreateCV("Csh:0-WorkCV", 12);
	clerkGroups[clerkType].workCVs[1] = CreateCV("Csh:1-WorkCV", 12);
	clerkGroups[clerkType].workCVs[2] = CreateCV("Csh:2-WorkCV", 12);
	clerkGroups[clerkType].workCVs[3] = CreateCV("Csh:3-WorkCV", 12);
	clerkGroups[clerkType].workCVs[4] = CreateCV("Csh:4-WorkCV", 12);

	clerkGroups[clerkType].bribeCVs[0] = CreateCV("Csh:0-BribeCV", 13);
	clerkGroups[clerkType].bribeCVs[1] = CreateCV("Csh:1-BribeCV", 13);
	clerkGroups[clerkType].bribeCVs[2] = CreateCV("Csh:2-BribeCV", 13);
	clerkGroups[clerkType].bribeCVs[3] = CreateCV("Csh:3-BribeCV", 13);
	clerkGroups[clerkType].bribeCVs[4] = CreateCV("Csh:4-BribeCV", 13);

	clerkGroups[clerkType].breakCVs[0] = CreateCV("Csh:0-BreakCV", 13);
	clerkGroups[clerkType].breakCVs[1] = CreateCV("Csh:1-BreakCV", 13);
	clerkGroups[clerkType].breakCVs[2] = CreateCV("Csh:2-BreakCV", 13);
	clerkGroups[clerkType].breakCVs[3] = CreateCV("Csh:3-BreakCV", 13);
	clerkGroups[clerkType].breakCVs[4] = CreateCV("Csh:4-BreakCV", 13);
}

/******************************************/
/* 			   Manager Data 			  */
/******************************************/

struct Manager {
	int appclerkMoney;
	int picclerkMoney;
	int passportclerkMoney;
	int cashierMoney;
	int totalMoney;
};

struct Manager manager;

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

/******************************************/
/* 			 System Job Data 			  */
/******************************************/

struct SystemJob {
	int ssn;
	int clerkID;
	enum persontype type;
};

int numSystemJobs = 50;
struct SystemJob jobs[50];

int systemJobFindLock; /* Used to synchronize search for available system jobs */
int filingPictureLock;
int filingApplicationLock;
int certifyingPassportLock;

void InitializeSystemJobs ()
{
	int jobID;

	for (jobID = 0; jobID < numSystemJobs; jobID++)
	{
		jobs[jobID].ssn = -1;
		jobs[jobID].clerkID = -1;
		jobs[jobID].type = CUSTOMER;
	}

	systemJobFindLock = CreateLock("SystemJobFindLock", 17);
	filingApplicationLock = CreateLock("FilingApplicationLock", 21);
	filingPictureLock = CreateLock("FilingPictureLock", 17);
	certifyingPassportLock = CreateLock("CertifyingPassportLock", 22);
}

/* ========================================================================================================================================= */
/*																																			 */
/*		SIMULATION OUTPUT																													 */
/*																																			 */
/* ========================================================================================================================================= */

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



/* ========================================================================================================================================= */
/*																																			 */
/*		CUSTOMER																															 */
/*																																			 */
/* ========================================================================================================================================= */

void CheckIfSenatorPresent (int ssn, int clerkID, enum persontype clerkType)
{
	AcquireLock(senatorPresentLock);
	if (isSenatorPresent && people[ssn].type != SENATOR)
	{
		WriteOutput(Customer_GoingOutsideForSenator, clerkType, CUSTOMER, ssn, clerkID);
		Wait(senatorPresentCV, senatorPresentLock); /* Wait for Senator to finish. */
		ReleaseLock(senatorPresentLock); /* Lock gets reacquired inside of Wait, release it and continue. */
		
		if (clerkID == -1)
		{
			return DecideClerk(ssn, clerkType); /* Never decided which line to get in, start from the top. */
		}
		else
		{
			return DecideLine(ssn, clerkID, clerkType); /* Get back in one of same clerk's line. */
		}
	}
	else
	{
		ReleaseLock(senatorPresentLock); /* No Senator present or I am a Senator, continue with business. */
	}
}

void BribeClerk (int ssn, int clerkID, enum persontype clerkType)
{
	int lineLock;
	int clerkLock;
	int bribeCV;
	int workCV;

	lineLock = clerkGroups[clerkType].lineLock;
	clerkLock = clerkGroups[clerkType].clerkLocks[clerkID];
	workCV = clerkGroups[clerkType].workCVs[clerkID];
	bribeCV = clerkGroups[clerkType].bribeCVs[clerkID];

	/* Let clerk know you are trying to bribe her. */
	clerkGroups[clerkType].clerks[clerkID].numCustomersBribing++;
	Wait(bribeCV, lineLock);
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

	/* Reacquire lock so Customer can get in line. */
	AcquireLock(lineLock);
}

void WaitInLine (int ssn, int clerkID, enum persontype clerkType, enum linetype lineType)
{
	int lineLock;
	int lineCV;

	lineLock = clerkGroups[clerkType].lineLock;
	lineCV = clerkGroups[clerkType].lineCVs[clerkID];
	bribeLineCV = clerkGroups[clerkType].bribeLineCVs[clerkID];

	switch (lineType)
	{
		case NORMALLINE:
			lineCV = clerkGroups[clerkType].lineCVs[clerkID];
			clerkGroups[clerkType].clerks[clerkID].lineLength++;
			WriteOutput(Customer_GotInRegularLine, clerkType, CUSTOMER, ssn, clerkID);
			Wait(lineCV, lineLock); /* Wait in line */
			clerkGroups[clerkType].clerks[clerkID].lineLength--;
			break;
		case BRIBELINE:
			lineCV = clerkGroups[clerkType].bribeLineCVs[clerkID];
			clerkGroups[clerkType].clerks[clerkID].bribeLineLength++;
			WriteOutput(Customer_GotInBribeLine, clerkType, CUSTOMER, ssn, clerkID);
			Wait(lineCV, lineLock); /* Wait in line */
			clerkGroups[clerkType].clerks[clerkID].bribeLineLength--;
			break;
		case SENATORLINE:
			lineCV = clerkGroups[clerkType].senatorLineCVs[clerkID];
			clerkGroups[clerkType].clerks[clerkID].senatorLineLength++;
			WriteOutput(Customer_GotInRegularLine, clerkType, SENATOR, ssn, clerkID);
			Wait(lineCV, lineLock); /* Wait in line */
			clerkGroups[clerkType].clerks[clerkID].senatorLineLength--;
			break;
	}
}

/* Customers decide which clerk has the shortest line that isn't on 	*/
/* 	on break. After choosing which clerk, customers then decide (based 	*/
/*	on whether they have enough money) whether to get into that clerk's */
/* 	bribe line, or not. If the person is a senator, they have a 		*/
/*	different group of lines. 											*/
/* Params: 	customer's ssn 												*/
/*			type of clerk line (0 = App, 1 = Pic, 2 = Pas, 3 = Csh) 	*/
/* Return: 	clerkID for line customer picked 							*/
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

	numClerks = clerkGroups[clerkType].numClerks;

	lineLock = clerkGroups[clerkType].lineLock;

	/* If Senator is present, "go outside" instead of picking line. */
	CheckIfSenatorPresent(ssn, currentClerk, clerkType);	

	AcquireLock(lineLock);
	for (clerkID = 0; clerkID < numClerks; clerkID++)
	{
		/* Different lines depending on whether customer or senator. */
		if (people[ssn].type == SENATOR)
		{
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

void DecideLine (int ssn, int clerkID, enum persontype clerkType)
{	
	int lineLock;

	lineLock = clerkGroups[clerkType].lineLock;

	/* Now that customer has selected a clerk's line, either go:			*/ 
	/* 	(1)	Straight to the counter											*/
	/* 	(2) (If SENATOR) Senator line 										*/
	/*	(3) (If >= $600) Bribe line 										*/
	/*	(4)	Normal line 													*/			

	/* If line was empty when customer joined, go straight to the counter. */
	if (clerkGroups[clerkType].clerks[clerkID].state == AVAILABLE)
	{
		clerkGroups[clerkType].clerks[clerkID].state = BUSY; /* Set BUSY so no other Customer comes straight to counter. */
	}

	/* Clerk is unavailable; Which line to wait in? */
	else
	{ 
		/* All Senators get in the Senator line */
		if (people[ssn].type == SENATOR)
		{
			WaitInLine(ssn, clerkID, clerkType, SENATORLINE);
		}

		/* Not a Senator; Bribe or no bribe? */
		else
		{
			if (clerkGroups[clerkType].clerks[clerkID].state != ONBREAK && people[ssn].money >= 600 && clerkGroups[clerkType].clerks[clerkID].lineLength >= 1)
			{ 
				/* If customer has enough money and she's not in a line for a clerk 	*/
				/*	that is on break, always bribe. 									*/
				/* 	(1)	Let clerk know you are trying to bribe, and do bribe. 			*/
				/*	(2) Wait in the bribe line. 										*/
				/*	(3) Check if a Senator has joined, if so need to "go outside" and 	*/
				/*		  re-decide line. 												*/
				BribeClerk(ssn, clerkID, clerkType);
				WaitInLine(ssn, clerkID, clerkType, BRIBELINE);
				CheckIfSenatorPresent(ssn);
			}
			else
			{ 
				/* No other options. Get in regular line. */
				WaitInLine(ssn, clerkID, clerkType, NORMALLINE);
				CheckIfSenatorPresent(ssn); /* 	Make sure no senators have entered since joining line. */
			}
		}
	}

	ReleaseLock(lineLock); /* Acquired lock in DecideClerk; Release so next Customer can decide their line. */

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
	DecideLine(ssn, clerkID, clerkType);
	CustomerInteraction(ssn, clerkID, clerkType);
}

void Leave (int ssn)
{
	numCustomersFinished++;

	if (people[ssn].type == SENATOR)
	{
		WriteOutput(Customer_LeavingPassportOffice, SENATOR, SENATOR, ssn, ssn);
		AcquireLock(senatorPresentLock);
		isSenatorPresent = false;
		Broadcast(senatorPresentCV, senatorPresentLock);
		ReleaseLock(senatorPresentLock);
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

	ssn = threadParam;
	ReleaseLock(paramLock);

	customer = people[ssn];

	/* Customer (randomly) decides whether she wants to file application */
	/* 	or take picture first. */
	/* TODO: Random syscall. */
	applicationFirst = 1;

	AcquireLock(senatorPresentLock);
	if (isSenatorPresent)
	{
		Wait(senatorPresentCV, senatorPresentLock);
	}
	else if (customer.type == SENATOR)
	{
		isSenatorPresent = true;
	}
	ReleaseLock(senatorPresentLock);

	if (applicationFirst > 50)
	{
		/* Go to Application Clerk */
		clerkID = DecideClerk(ssn, APPLICATION);
		DecideLine(ssn, clerkID, APPLICATION);
		CustomerInteraction(ssn, clerkID, APPLICATION);
		
		/* Go to Picture Clerk */
		clerkID = DecideClerk(ssn, PICTURE);
		DecideLine(ssn, clerkID, PICTURE);
		CustomerInteraction(ssn, clerkID, PICTURE);
	}
	else
	{
		/* Go to Picture Clerk */
		clerkID = DecideClerk(ssn, PICTURE);
		DecideLine(ssn, clerkID, PICTURE);
		CustomerInteraction(ssn, clerkID, PICTURE);
		
		/* Go to Application Clerk */
		clerkID = DecideClerk(ssn, APPLICATION);
		DecideLine(ssn, clerkID, APPLICATION);
		CustomerInteraction(ssn, clerkID, APPLICATION);
	}

	/* Go to Passport Clerk */
	clerkID = DecideClerk(ssn, PASSPORT);
	DecideLine(ssn, clerkID, PASSPORT);
	CustomerInteraction(ssn, clerkID, PASSPORT);

	/* Go to Cashier */
	clerkID = DecideClerk(ssn, CASHIER);
	DecideLine(ssn, clerkID, CASHIER);
	CustomerInteraction(ssn, clerkID, CASHIER);
}

/* ========================================================================================================================================= */
/*																																			 */
/*		CLERKS																																 */
/*																																			 */
/* ========================================================================================================================================= */

/* Clerks accept a bribe of $500 from any customer who has determined 	*/
/* 	they have sufficient money. Clerk resumes deciding whether she 		*/
/*	should signal a customer to come to her counter, go on break, 		*/
/*	or accept another bribe after accepting this bribe. 				*/
/* Params:	clerk's ID 													*/
/*			type of clerk line (0 = App, 1 = Pic, 2 = Pas, 3 = Csh) 	*/
void AcceptBribe (int clerkID, enum persontype clerkType)
{
	int lineLock;
	int moneyLock;
	int clerkLock;
	int workCV;
	int bribeCV;

	lineLock = clerkGroups[clerkType].lineLock;
	moneyLock = clerkGroups[clerkType].moneyLock;
	clerkLock = clerkGroups[clerkType].clerkLocks[clerkID];
	workCV = clerkGroups[clerkType].workCVs[clerkID];
	bribeCV = clerkGroups[clerkType].bribeCVs[clerkID];

	/* Should already have lineLock from Clerk decision function */

	Signal(bribeCV, lineLock); /* Let customer know she can wake up to pay you bribe */
	AcquireLock(clerkLock); /* Synchronize clerk's work for bribe interaction */
	ReleaseLock(lineLock); /* Done deciding which customer from line */
	Wait(workCV, clerkLock); /* Wait until she pays after customer wakes up from signal */
	
	AcquireLock(moneyLock); /* Synchronize update to clerk's pool of money */
	clerkGroups[clerkType].groupMoney += 500;
	ReleaseLock(moneyLock);
	WriteOutput(Clerk_ReceivedBribe, clerkType, CUSTOMER, -1, clerkID);

	Signal(workCV, clerkLock); /* Let customer know she can get in bribe line. */
	clerkGroups[clerkType].clerks[clerkID].currentCustomer = -1;
	ReleaseLock(clerkLock); /* Done with clerk's work for bribe interaction. */
}

void TakeBreak (int clerkID, enum persontype clerkType) 
{
	int lineLock;
	int breakCV;

	lineLock = clerkGroups[clerkType].lineLock;
	breakCV = clerkGroups[clerkType].breakCVs[clerkID];

	WriteOutput(Clerk_GoingOnBreak, clerkType, clerkType, -1, clerkID);
	Wait(breakCV, lineLock); /* Waiting on breakCV = "going on break" */
	WriteOutput(Clerk_ComingOffBreak, clerkType, clerkType, -1, clerkID);
}

int CreateSystemJob (int ssn, int clerkID, enum persontype clerkType)
{
	int jobID;

	AcquireLock(systemJobFindLock);

	/* Find free system job */
	for (jobID = 0; jobID < numSystemJobs; jobID++)
	{
		if (jobs[jobID].ssn == -1 && jobs[jobID].clerkID == -1 && jobs[jobID].type == CUSTOMER)
		{
			jobs[jobID].ssn = ssn;
			jobs[jobID].clerkID = clerkID;
			jobs[jobID].type = clerkType;
			return jobID;
		}
	}

	ReleaseLock(systemJobFindLock);

	return -1;
}

void ResetSystemJob (int jobID)
{
	AcquireLock(systemJobFindLock);

	/* Reset job so other System Jobs can use. */
	jobs[jobID].ssn = -1;
	jobs[jobID].clerkID = -1;
	jobs[jobID].type = CUSTOMER;

	ReleaseLock(systemJobFindLock);
}

void RunSystemJob ()
{
	int jobID;
	int systemLock;
	int filingTime;
	int elapsedTime;
	int ssn;

	jobID = threadParam;
	ReleaseLock(paramLock);

	ssn = jobs[jobID].ssn;

	switch (jobs[jobID].type)
	{
		case APPLICATION:
			systemLock = filingApplicationLock;
			break;
		case PICTURE:
			systemLock = filingPictureLock;
			break;
		case PASSPORT:
			systemLock = certifyingPassportLock;
			break;
		case CASHIER:
			break; /* Casier not responsible for filing anything in the system */
	}

	/* TODO: Random syscall. See above. */
	/* 	filingTime = Random(20, 100); */
	filingTime = 20;

	for (elapsedTime = 0; elapsedTime < filingTime; elapsedTime++)
	{
		Yield();
	}

	AcquireLock(systemLock);

	switch (jobs[jobID].type)
	{
		case APPLICATION:
			customers[ssn].applicationFiled = true;
			break;
		case PICTURE:
			customers[ssn].pictureFiled = true;
			break;
		case PASSPORT:
			customers[ssn].passportCertified = true;
			break;
		case CASHIER:
			break; /* Casier not responsible for filing anything in the system */
	}

	WriteOutput(Clerk_SystemJobComplete, jobs[jobID].type, people[ssn].type, ssn, jobs[jobID].clerkID);
	ReleaseLock(systemLock);

	ResetSystemJob(jobID);

	Exit(0);
}

void ClerkInteraction (int clerkID, enum persontype clerkType)
{
	int lineLock;
	int clerkLock;
	int workCV;

	int customerSSN;

	int jobID;

	lineLock = clerkGroups[clerkType].lineLock;
	clerkLock = clerkGroups[clerkType].clerkLocks[clerkID];
	workCV = clerkGroups[clerkType].workCVs[clerkID];

	customerSSN = clerkGroups[clerkType].clerks[clerkID].currentCustomer;

	AcquireLock(clerkLock);
	ReleaseLock(lineLock);
	Wait(workCV, clerkLock);
	Yield(); /* Simulate work */

	switch (clerkType) 
	{
		case APPLICATION:
			break; /* Below: File application. */
		case PICTURE:
			break; /* Below: Take picture with Signal, File if Customer liked picture. */
		case PASSPORT:
			AcquireLock(filingApplicationLock);
			AcquireLock(filingPictureLock);
			if (customers[customerSSN].applicationFiled == false || customers[customerSSN].pictureFiled == false)
			{
				clerkGroups[clerkType].clerks[clerkID].customerAppReadyToCertify = false;
				WriteOutput(Clerk_DeterminedAppAndPicNotCompleted, clerkType, people[customerSSN].type, customerSSN, clerkID);
			}
			else
			{
				clerkGroups[clerkType].clerks[clerkID].customerAppReadyToCertify = true;
				WriteOutput(Clerk_DeterminedAppAndPicCompleted, clerkType, people[customerSSN].type, customerSSN, clerkID);
				
				/* Certify Passport in the system */
				jobID = CreateSystemJob(customerSSN, clerkID, clerkType);
				AcquireLock(paramLock);
				threadParam = jobID;
				Fork("CertifyPassportJob", sizeof("CertifyPassportJob"), RunSystemJob);
			}
			ReleaseLock(filingApplicationLock);
			ReleaseLock(filingPictureLock);
			break;
		case CASHIER:
			AcquireLock(certifyingPassportLock);
			if (customers[customerSSN].passportCertified == false)
			{
				clerkGroups[clerkType].clerks[clerkID].customerAppReadyForPayment = false;
			}
			else
			{
				clerkGroups[clerkType].clerks[clerkID].customerAppReadyForPayment = true;
				Signal(workCV, clerkLock);
				Wait(workCV, clerkLock);

				AcquireLock(clerkGroups[clerkType].moneyLock);
				clerkGroups[clerkType].groupMoney += 100;
				WriteOutput(Clerk_ReceivedPayment, clerkType, people[customerSSN].type, customerSSN, clerkID);
				ReleaseLock(clerkGroups[clerkType].moneyLock);

				customers[customerSSN].gotPassport = true;
			}
			ReleaseLock(certifyingPassportLock);
			break;
	}

	Signal(workCV, clerkLock);
	Wait(workCV, clerkLock);

	switch (clerkType)
	{
		case APPLICATION:
			/* File Application in the system */
			jobID = CreateSystemJob(customerSSN, clerkID, clerkType);
			AcquireLock(paramLock);
			threadParam = jobID;
			Fork("ApplicationFilingJob", sizeof("ApplicationFilingJob"), RunSystemJob);
			break; /* Application Clerk already did work */
		case PICTURE:
			if (clerkGroups[clerkType].clerks[clerkID].customerLikedPhoto == true)
			{
				/* File Photo in the system */
				jobID = CreateSystemJob(customerSSN, clerkID, clerkType);
				AcquireLock(paramLock);
				threadParam = jobID;
				Fork("PictureFilingJob", sizeof("PictureFilingJob"), RunSystemJob);
			}
			clerkGroups[clerkType].clerks[clerkID].customerLikedPhoto = false; /* Done checking if Customer liked photo; "forget" so ready for next Customer. */
			break;
		case PASSPORT:
			clerkGroups[clerkType].clerks[clerkID].customerAppReadyToCertify = false; /* Done checking if Customer App/Picture filed; "forget" so ready for next Customer. */
			break;
		case CASHIER:
			clerkGroups[clerkType].clerks[clerkID].customerAppReadyForPayment = false; /* Done checking if Customer can pay for passport; "forget" so ready for next Customer. */
			break;
	}

	/* Done with Customer, reset so we're ready for next Customer. */
	clerkGroups[clerkType].clerks[clerkID].currentCustomer = -1;

	ReleaseLock(clerkLock);
}

enum clerkinteraction DecideInteraction (int clerkID, enum persontype clerkType)
{
	int lineLock;
	int lineCV;
	int bribeLineCV;
	int senatorLineCV;
	int breakCV;

	lineLock = clerkGroups[clerkType].lineLock;
	lineCV = clerkGroups[clerkType].lineCVs[clerkID];
	bribeLineCV = clerkGroups[clerkType].bribeLineCVs[clerkID];
	senatorLineCV = clerkGroups[clerkType].senatorLineCVs[clerkID];
	breakCV = clerkGroups[clerkType].breakCVs[clerkID];

	AcquireLock(lineLock); /* Synchronize line checks (no customers can join while scanning lines). */
	
	AcquireLock(senatorPresentLock); /* Synchronize senator present check */
	if (isSenatorPresent)
	{
		/* If senator is present, customers need to be woken up so they can "go outside." */
		ReleaseLock(senatorPresentLock); /* Done checking if senator is present. (Release ASAP for other clerks) */
		
		if (clerkGroups[clerkType].clerks[clerkID].lineLength > 0)
		{
			Broadcast(lineCV, lineLock);
		}
		
		if (clerkGroups[clerkType].clerks[clerkID].bribeLineLength > 0)
		{
			Broadcast(bribeLineCV, lineLock);
		}

		/* Now that all customers "went outside," handle senator(s). */
		if (clerkGroups[clerkType].clerks[clerkID].senatorLineLength > 0)
		{
			/* WriteOutput(Clerk_SignalledCustomer, clerkType, SENATOR, -1, clerkID); */
			Signal(senatorLineCV, lineLock); /* Let first waiting senator know she can come to counter. */
			clerkGroups[clerkType].clerks[clerkID].state = BUSY;
			return DOINTERACTION;
		}
	}
	ReleaseLock(senatorPresentLock); /* Done checking if senator is present. */

	/* Next priority: Take care of customers trying to bribe me. */
	if (clerkGroups[clerkType].clerks[clerkID].numCustomersBribing > 0)
	{
		return ACCEPTBRIBE;
	}

	/* Nobody is actively trying to bribe, but past bribers are waiting. */
	else if (clerkGroups[clerkType].clerks[clerkID].bribeLineLength > 0)
	{
		WriteOutput(Clerk_SignalledCustomer, clerkType, CUSTOMER, -1, clerkID);
		Signal(bribeLineCV, lineLock); /* Let first waiting briber know she can come to counter. */
		clerkGroups[clerkType].clerks[clerkID].state = BUSY;
		return DOINTERACTION;
	}

	/* No bribers, take care of normal customers (if there are any). */
	else if (clerkGroups[clerkType].clerks[clerkID].lineLength > 0)
	{
		WriteOutput(Clerk_SignalledCustomer, clerkType, CUSTOMER, -1, clerkID);
		Signal(lineCV, lineLock); /* Let first waiting customer know she can come to counter. */
		clerkGroups[clerkType].clerks[clerkID].state = BUSY;
		return DOINTERACTION;
	}

	/* No customers to take care of, go on break until manager wakes me up. */
	else
	{
		clerkGroups[clerkType].clerks[clerkID].state = ONBREAK;
		return TAKEBREAK;	
	}
}

void Clerk()
{
	int ssn;
	struct Person clerk;
	enum clerkinteraction interaction;

	ssn = threadParam;
	ReleaseLock(paramLock);

	clerk = people[ssn];

	interaction = DOINTERACTION; /* First interaction: Clerks should be prepared for first customer. */

	do
	{
		switch(interaction) 
		{
			case ACCEPTBRIBE:
				AcceptBribe(clerk.id, clerk.type);
				break;
			case DOINTERACTION:
				ClerkInteraction(clerk.id, clerk.type);
				break;
			case TAKEBREAK:
				TakeBreak(clerk.id, clerk.type);
		}

		/* Select next interaction based on: 										*/
		/* 	(1) if there are people trying to bribe > AcceptBribe 					*/
		/*	(2) if there are people waiting in any of my lines > ClerkInteraction 	*/
		/*	(3) if there are no people waiting in any of my lines > TakeBreak 		*/
		interaction = DecideInteraction (clerk.id, clerk.type);
	} while (numCustomersFinished < (numCustomers + numSenators));

	Exit(0);
}

/* ========================================================================================================================================= */
/*																																			 */
/*		MANAGER 																															 */
/*																																			 */
/* ========================================================================================================================================= */

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

	/* TODO: Need to wake up sleeping threads so they can Exit themselves. */

	Exit(0);
}

/* ========================================================================================================================================= */
/*																																			 */
/*		PASSPORT OFFICE																														 */
/*																																			 */
/* ========================================================================================================================================= */

void InitializeData ()
{
	InitializeCustomerData();
	InitializeSenatorData();
	InitializeApplicationClerkData();
	InitializePictureClerkData();
	InitializePassportClerkData();
	InitializeCashierData();
	InitializeManager();
	InitializeSystemJobs();

	paramLock = CreateLock("ParamLock", sizeof("ParamLock"));
}

void ForkAgents ()
{
	int ssn;

	for (ssn = 0; ssn < (numCustomers + numSenators); ssn++)
	{
		AcquireLock(paramLock);
		threadParam = ssn;
		Fork("CustomerThread", sizeof("CustomerThread"), Customer);
	}

	for (; ssn < (numCustomers + numSenators) + (numAppClerks + numPicClerks + numPassportClerks + numCashiers); ssn++)
	{
		AcquireLock(paramLock);
		threadParam = ssn;
		Fork("ClerkThread", sizeof("ClerkThread"), Clerk);
	}

	Fork("ManagerThread", sizeof("ManagerThread"), Manager);
}

void CleanUpData ()
{
	int clerkType;
	int clerkNum;

	DestroyLock(senatorPresentLock);
	DestroyCV(senatorPresentCV);

	DestroyLock(systemJobFindLock);
	DestroyLock(filingApplicationLock);
	DestroyLock(filingPictureLock);
	DestroyLock(certifyingPassportLock);

	for (clerkType = 0; clerkType < 4; clerkType++)
	{
		DestroyLock(clerkGroups[clerkType].lineLock);
		DestroyLock(clerkGroups[clerkType].moneyLock);

		for (clerkNum = 0; clerkNum < 5; clerkNum++)
		{
			DestroyLock(clerkGroups[clerkType].clerkLocks[clerkNum]);
			DestroyCV(clerkGroups[clerkType].lineCVs[clerkNum]);
			DestroyCV(clerkGroups[clerkType].bribeLineCVs[clerkNum]);
			DestroyCV(clerkGroups[clerkType].senatorLineCVs[clerkNum]);
			DestroyCV(clerkGroups[clerkType].workCVs[clerkNum]);
			DestroyCV(clerkGroups[clerkType].bribeCVs[clerkNum]);
			DestroyCV(clerkGroups[clerkType].breakCVs[clerkNum]);
		}
	}
}

int main () 
{
	/* Initialize the simulation. */
	InitializeData();
	ForkAgents();

	while (num)

	/* Clean up the simulation. */ 
	CleanUpData();
	Exit(0);
}