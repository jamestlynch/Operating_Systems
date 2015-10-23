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
	bool photoFiled;
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

int numCustomersFinished = 0; /* Compare this to (numCustomers + numSenators) to see when program completes. */

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
		customers[ssn].photoFiled = false;
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

int senatorIndoorLock; /* Synchronizes isSenatorPresent */
int senatorIndoorCV; /* Wait on this whenever a senator is inside PPOffice */
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
		customers[ssn].photoFiled = false;
		customers[ssn].passportCertified = false;
		customers[ssn].passportRecorded = false;

		/* people holds all private data that people must update and make public to clerks. */
		people[ssn].id = ssn;
		people[ssn].money = InitialMoney();
		people[ssn].type = SENATOR;
	}

	senatorIndoorLock = CreateLock("SenatorIndoorLock", 17);
	senatorIndoorCV = CreateCV("SenatorIndoorCV", 15);
	/* TODO: Is this necessary? */
	/*	senatorOutdoorCV = CreateCV("SenatorOutdoorCV", 16); */
}

/******************************************/
/* 			    Clerk Data 				  */
/******************************************/

enum clerkstate { AVAILABLE, BUSY, ONBREAK };
enum clerkinteraction { DOINTERACTION, TAKEBREAK, ACCEPTBRIBE };

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
};

int numSystemJobs = 50;
struct SystemJob jobs[50];

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
	}

	filingPictureLock = CreateLock("FilingPictureLock", 17);
	filingApplicationLock = CreateLock("FilingApplicationLock", 21);
	certifyingPassportLock = CreateLock("CertifyingPassportLock", 22);
}

/* ========================================================================================================================================= */
/*																																			 */
/*		SIMULATION OUTPUT																													 */
/*																																			 */
/* ========================================================================================================================================= */

enum outputstatement { 
	Clerk_SignalledCustomer, Clerk_ReceivedSSN, Clerk_RecordedCompletedApplication
	Clerk_ReceivedBribe, Clerk_GoingOnBreak, Clerk_ComingOffBreak,
	Clerk_TookPicture, Clerk_ToldByCustomerDoesNotLikePicture, Clerk_ToldByCustomerDoesLikePicture,
	Clerk_DeterminedAppAndPicNotCompleted, Clerk_DeterminedAppAndPicCompleted, Clerk_RecordedPassport,
	Clerk_VerifiedPassportCertified, Clerk_ReceivedPayment, Clerk_ReceivedPaymentGoBackInLine,
	Clerk_ProvidedPassport, Clerk_RecordedCustomerGivenPassport,
	
	Manager_WokeUpClerk, Manager_CountedMoneyForClerk, Manager_CountedTotalMoney,
	
	Customer_GotInRegularLine, Customer_GotInBribeLine, Customer_GaveSSN,
	Customer_DoesNotLikePicture, Customer_DoesLikePicture, Customer_WentTooSoon,
	Customer_PaidForPassport, Customer_GoingOutsideForSenator, Customer_LeavingPassportOffice,
}

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
					WriteOne("ApplicationClerk %d has signalled a Customer to come to their counter.", clerkID);
					break;
				case PICTURE:
					WriteOne("PictureClerk %d has signalled a Customer to come to their counter.", clerkID);
					break;
				case PASSPORT:
					WriteOne("PassportClerk %d has signalled a Customer to come to their counter.", clerkID);
					break;
				case CASHIER:
					WriteOne("Cashier %d has signalled a Customer to come to their counter.", clerkID);
					break;
			}
			break;
		case Clerk_ReceivedSSN:
			switch(clerkType)
			{
				case APPLICATION:
					WriteThree("ApplicationClerk %d has received SSN %d from Customer %d", clerkID, ssn, clerkID);
					break;
				case PICTURE:
					WriteThree("PictureClerk %d has received SSN %d from Customer %d", clerkID, ssn, clerkID);
					break;
				case PASSPORT:
					WriteThree("PassportClerk %d has received SSN %d from Customer %d", clerkID, ssn, clerkID);
					break;
				case CASHIER:
					WriteThree("Cashier %d has received SSN %d from Customer %d", clerkID, ssn, clerkID);
					break;
			}
			break;
		case Clerk_GoingOnBreak:
			switch(clerkType)
			{
				case APPLICATION:
					WriteOne("ApplicationClerk %d is going on break", clerkID);
					break;
				case PICTURE:
					WriteOne("PictureClerk %d is going on break", clerkID);
					break;
				case PASSPORT:
					WriteOne("PassportClerk %d is going on break", clerkID);
					break;
				case CASHIER:
					WriteOne("Cashier %d is going on break", clerkID);
					break;
			}
			break;
		case Clerk_ComingOffBreak:
			switch(clerkType)
			{
				case APPLICATION:
					WriteOne("ApplicationClerk %d is coming off break", clerkID);
					break;
				case PICTURE:
					WriteOne("PictureClerk %d is coming off break", clerkID);
					break;
				case PASSPORT:
					WriteOne("PassportClerk %d is coming off break", clerkID);
					break;
				case CASHIER:
					WriteOne("Cashier %d is coming off break", clerkID);
					break;
			}
			break;
		case Clerk_RecordedCompletedApplication:
			WriteTwo("ApplicationClerk %d has recorded a completed application for Customer %d", clerkID, ssn);
			break;
		case Clerk_ReceivedBribe:
			WriteTwo("ApplicationClerk %d has received $500 from Customer %d", clerkID, ssn);
			break;
		case Clerk_TookPicture:
			WriteTwo("PictureClerk %d has taken a picture of Customer %d", clerkID, ssn);
			break;
		case Clerk_ToldByCustomerDoesNotLikePicture:
			WriteTwo("PictureClerk %d has been told that Customer %d does not like their picture", clerkID, ssn);
			break;
		case Clerk_ToldByCustomerDoesLikePicture:
			WriteTwo("PictureClerk %d has been told that Customer %d does like their picture", clerkID, ssn);
			break;
		case Clerk_DeterminedAppAndPicNotCompleted:
			WriteTwo("PassportClerk %d has determined that Customer %d does not have both their application and picture completed", clerkID, ssn);
			break;
		case Clerk_DeterminedAppAndPicCompleted:
			WriteTwo("PassportClerk %d has determined that Customer %d has both their application and picture completed", clerkID, ssn);
			break;
		case Clerk_RecordedPassport:
			WriteTwo("PassportClerk %d has recorded Customer %d passport documentation", clerkID, ssn);
			break;
		case Clerk_VerifiedPassportCertified:
			WriteTwo("Cashier %d has verified that Customer %d has been certified by a PassportClerk", clerkID, ssn);
			break;
		case Clerk_ReceivedPayment:
			WriteTwo("Cashier %d has received the $100 from Customer %d after certification", clerkID, ssn);
			break;
		case Clerk_ReceivedPaymentGoBackInLine:
			WriteTwo("Cashier %d has received the $100 from Customer %d before certification. They are to go to the back of my line.", clerkID, ssn);
			break;
		case Clerk_ProvidedPassport:
			WriteTwo("Cashier %d has provided Customer %d their completed passport", clerkID, ssn);
			break;
		case Clerk_RecordedCustomerGivenPassport:
			WriteTwo("Cashier %d has recorded that Customer %d has been given their completed passport", clerkID, ssn);
			break;
	
		
		case Manager_WokeUpClerk:
			switch(clerkType)
			{
				case APPLICATION:
					Write("Manager has woken up an ApplicationClerk");
					break;
				case PICTURE:
					Write("Manager has woken up an PictureClerk");
					break;
				case PASSPORT:
					Write("Manager has woken up an PassportClerk");
					break;
				case CASHIER:
					Write("Manager has woken up an Cashier");
					break;
			}
			break;
		case Manager_CountedMoneyForClerk:
			switch(clerkType)
			{
				case APPLICATION:
					WriteOne("Manager has counted a total of $%d for ApplicationClerks", money);
					break;
				case PICTURE:
					WriteOne("Manager has counted a total of $%d for PictureClerks", money);
					break;
				case PASSPORT:
					WriteOne("Manager has counted a total of $%d for PassportClerks", money);
					break;
				case CASHIER:
					WriteOne("Manager has counted a total of $%d for Cashiers", money);
					break;
			}
			break;
		case Manager_CountedTotalMoney:
			WriteOne("Manager has counted a total of $%d for the passport office", money);
			break;
	
	
		case Customer_GotInRegularLine:
			switch(clerkType)
			{
				case APPLICATION:
					switch(customerType)
					{
						case CUSTOMER:
							WriteTwo("Customer %d has gotten in regular line for ApplicationClerk %d.", ssn, clerkID);
							break;
						case SENATOR:
							WriteTwo("Senator %d has gotten in regular line for ApplicationClerk %d.", ssn, clerkID);
							break;
					}
					break;
				case PICTURE:
					switch(customerType)
					{
						case CUSTOMER:
							WriteTwo("Customer %d has gotten in regular line for PictureClerk %d.", ssn, clerkID);
							break;
						case SENATOR:
							WriteTwo("Senator %d has gotten in regular line for PictureClerk %d.", ssn, clerkID);
							break;
					}
					break;
				case PASSPORT:
					switch(customerType)
					{
						case CUSTOMER:
							WriteTwo("Customer %d has gotten in regular line for PassportClerk %d.", ssn, clerkID);
							break;
						case SENATOR:
							WriteTwo("Senator %d has gotten in regular line for PassportClerk %d.", ssn, clerkID);
							break;
					}
					break;
				case CASHIER:
					switch(customerType)
					{
						case CUSTOMER:
							WriteTwo("Customer %d has gotten in regular line for Cashier %d.", ssn, clerkID);
							break;
						case SENATOR:
							WriteTwo("Senator %d has gotten in regular line for Cashier %d.", ssn, clerkID);
							break;
					}
					break;
			}
			break;
		case Customer_GotInBribeLine:
			switch(clerkType)
			{
				case APPLICATION:
					WriteTwo("Customer %d has gotten in bribe line for ApplicationClerk %d.", ssn, clerkID);
					break;
				case PICTURE:
					WriteTwo("Customer %d has gotten in bribe line for PictureClerk %d.", ssn, clerkID);
					break;
				case PASSPORT:
					WriteTwo("Customer %d has gotten in bribe line for PassportClerk %d.", ssn, clerkID);
					break;
				case CASHIER:
					WriteTwo("Customer %d has gotten in bribe line for Cashier %d.", ssn, clerkID);
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
							WriteThree("Customer %d has given SSN %d to ApplicationClerk %d.", ssn, ssn, clerkID);
							break;
						case SENATOR:
							WriteThree("Senator %d has given SSN %d to ApplicationClerk %d.", ssn, ssn, clerkID);
							break;
					}
					break;
				case PICTURE:
					switch(customerType)
					{
						case CUSTOMER:
							WriteThree("Customer %d has given SSN %d to PictureClerk %d.", ssn, ssn, clerkID);
							break;
						case SENATOR:
							WriteThree("Senator %d has given SSN %d to PictureClerk %d.", ssn, ssn, clerkID);
							break;
					}
					break;
				case PASSPORT:
					switch(customerType)
					{
						case CUSTOMER:
							WriteThree("Customer %d has given SSN %d to PassportClerk %d.", ssn, ssn, clerkID);
							break;
						case SENATOR:
							WriteThree("Senator %d has given SSN %d to PassportClerk %d.", ssn, ssn, clerkID);
							break;
					}
					break;
				case CASHIER:
					switch(customerType)
					{
						case CUSTOMER:
							WriteThree("Customer %d has given SSN %d to Cashier %d.", ssn, ssn, clerkID);
							break;
						case SENATOR:
							WriteThree("Senator %d has given SSN %d to Cashier %d.", ssn, ssn, clerkID);
							break;
					}
					break;
			}
			break;
		case Customer_DoesNotLikePicture:
			switch(customerType)
			{
				case CUSTOMER:
					WriteTwo("Customer %d does not like their picture from PictureClerk %d.", ssn, clerkID);
					break;
				case SENATOR:
					WriteTwo("Senator %d does not like their picture from PictureClerk %d.", ssn, clerkID);
					break;
			}
			break;
		case Customer_DoesLikePicture:
			switch(customerType)
			{
				case CUSTOMER:
					WriteTwo("Customer %d does like their picture from PictureClerk %d.", ssn, clerkID);
					break;
				case SENATOR:
					WriteTwo("Senator %d does like their picture from PictureClerk %d.", ssn, clerkID);
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
							WriteTwo("Customer %d has gone to PassportClerk %d too soon. They are going to the back of the line.", ssn, clerkID);
							break;
						case SENATOR:
							WriteTwo("Senator %d has gone to PassportClerk %d too soon. They are going to the back of the line.", ssn, clerkID);
							break;
					}
					break;
				case CASHIER:
					switch(customerType)
					{
						case CUSTOMER:
							WriteTwo("Customer %d has gone to Cashier %d too soon. They are going to the back of the line.", ssn, clerkID);
							break;
						case SENATOR:
							WriteTwo("Senator %d has gone to Cashier %d too soon. They are going to the back of the line.", ssn, clerkID);
							break;
					}
					break;
			break;
		case Customer_PaidForPassport:
			switch(customerType)
			{
				case CUSTOMER:
					WriteTwo("Customer %d has given Cashier %d $100.", ssn, clerkID);
					break;
				case SENATOR:
					WriteTwo("Senator %d has given Cashier %d $100.", ssn, clerkID);
					break;
			}
			break;
		case Customer_GoingOutsideForSenator:
			WriteOne("Customer %d is going outside the Passport Office because their is a Senator present.", ssn);
			break;
		case Customer_LeavingPassportOffice:
			switch(customerType)
			{
				case CUSTOMER:
					WriteOne("Customer %d is leaving the Passport Office.", ssn);
					break;
				case SENATOR:
					WriteOne("Senator %d is leaving the Passport Office.", ssn);
					break;
			}
			break;
	}
}

/* ========================================================================================================================================= */
/*																																			 */
/*		CUSTOMER - APPLICATION CLERK INTERACTION 																							 */
/*																																			 */
/* ========================================================================================================================================= */

void FileApplication (int ssn, int clerkID)
{

}

void ApplicationClerkWork (int clerkID)
{

}

/* ========================================================================================================================================= */
/*																																			 */
/*		CUSTOMER - PICTURE CLERK INTERACTION 																								 */
/*																																			 */
/* ========================================================================================================================================= */

void GetPictureTaken (int ssn, int clerkID)
{

}

void PictureClerkWork (int clerkID)
{

}

/* ========================================================================================================================================= */
/*																																			 */
/*		CUSTOMER - PASSPORT CLERK INTERACTION 																								 */
/*																																			 */
/* ========================================================================================================================================= */

void CertifyPassport (int ssn, int clerkID)
{

}

void PassportClerkWork (int clerkID)
{

}

/* ========================================================================================================================================= */
/*																																			 */
/*		CUSTOMER - CASHIER INTERACTION 																										 */
/*																																			 */
/* ========================================================================================================================================= */

void PayForPassport (int ssn, int clerkID)
{

}

void CashierWork (int clerkID)
{

}


/* ========================================================================================================================================= */
/*																																			 */
/*		CUSTOMER																															 */
/*																																			 */
/* ========================================================================================================================================= */

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

	/*  TODO: Arrays were cast as unsigned ints, how to convert/cast back to array; or just how to access data inside array, otherwise? */

	/* Check if the senator is present, and if so, "go outside" by waiting on the CV. */
	/* 	By placing this here, we ensure line order remains consistent, conveniently. */
	AcquireLock(senatorIndoorLock);
	if (isSenatorPresent && people[ssn].type != SENATOR)
	{
		WriteOutput(Customer_GoingOutsideForSenator, clerkType, CUSTOMER, ssn, clerkID);
		Wait(senatorIndoorCV, senatorIndoorLock);
	}
	ReleaseLock(senatorIndoorLock);

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

void WaitInLine (int ssn, int clerkID, enum persontype clerkType)
{	
	int lineLock;
	int lineCV;
	int bribeLineCV;
	int senatorLineCV;
	int clerkLock;
	int workCV;
	int bribeCV;

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

void Customer (int ssn)
{
	struct Person customer;
	int applicationFirst;
	int clerkID;

	customer = people[ssn];

	/* Customer (randomly) decides whether she wants to file application */
	/* 	or take picture first. */
	/* TODO: Random syscall. */
	applicationFirst = 1;

	if (applicationFirst)
	{
		/* Go to Application Clerk */
		clerkID = DecideClerk(ssn, APPLICATION);
		WaitInLine(ssn, clerkID, APPLICATION);
		FileApplication(ssn, clerkID);
		
		/* Go to Picture Clerk */
		clerkID = DecideClerk(ssn, PICTURE);
		WaitInLine(ssn, clerkID, PICTURE);
		GetPictureTaken(ssn, clerkID);
	}
	else
	{
		/* Go to Picture Clerk */
		clerkID = DecideClerk(ssn, PICTURE);
		WaitInLine(ssn, clerkID, PICTURE);
		GetPictureTaken(ssn, clerkID);
		
		/* Go to Application Clerk */
		clerkID = DecideClerk(ssn, APPLICATION);
		WaitInLine(ssn, clerkID, APPLICATION);
		FileApplication(ssn, clerkID);
	}

	/* Go to Passport Clerk */
	clerkID = DecideClerk(ssn, PASSPORT);
	WaitInLine(ssn, clerkID, PASSPORT);
	CertifyPassport(ssn, clerkID);

	/* Go to Cashier */
	clerkID = DecideClerk(ssn, CASHIER);
	WaitInLine(ssn, clerkID, CASHIER);
	PayForPassport(ssn, clerkID);
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
	WriteOutput(Clerk_ReceivedBribe, clerkType, CUSTOMER, ssn, clerkID);

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

	WriteOutput(Clerk_GoingOnBreak, clerkType, clerkType, ssn, clerkID);
	Wait(breakCV, lineLock); /* Waiting on breakCV = "going on break" */
	WriteOutput(Clerk_ComingOffBreak, clerkType, clerkType, ssn, clerkID);
}

void DoInteraction (int clerkID, enum persontype clerkType)
{
	switch (clerkType) 
	{
		case APPLICATION:
			ApplicationClerkWork(clerkID);
			break;
		case PICTURE:
			PictureClerkWork(clerkID);
			break;
		case PASSPORT:
			PassportClerkWork(clerkID);
			break;
		case CASHIER:
			CashierWork(clerkID);
			break;
	}
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
	
	AcquireLock(senatorIndoorLock); /* Synchronize senator present check */
	if (isSenatorPresent)
	{
		/* If senator is present, customers need to be woken up so they can "go outside." */
		ReleaseLock(senatorIndoorLock); /* Done checking if senator is present. (Release ASAP for other clerks) */
		
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
	ReleaseLock(senatorIndoorLock); /* Done checking if senator is present. */

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

void Clerk(int ssn)
{
	struct Person clerk;
	enum clerkinteraction interaction;

	clerk = people[ssn];

	while (true)
	{
		interaction = DecideInteraction (clerk.id, clerk.type);

		switch(interaction) 
		{
			case ACCEPTBRIBE:
				AcceptBribe(clerk.id, clerk.type);
				break;
			case DOINTERACTION:
				DoInteraction(clerk.id, clerk.type);
				break;
			case TAKEBREAK:
				TakeBreak(clerk.id, clerk.type);
		}
	}
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
}

int main () 
{
	InitializeData();
}