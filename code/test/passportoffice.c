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

int moneyOptions[4] = {100, 600, 1100, 1600};

struct Person {
	int ssn;
	int money;
	bool isSenator;
};

struct Person people[60]; /* numCustomers + numSenators */

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

int numCustomers = 50;
struct Customer customers[60]; /* customers holds all customers and senators - Allows us to keep ssn as index of arrays while still keeping everyone's SSN different. */

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
		people[ssn].ssn = ssn;
		people[ssn].money = InitialMoney();
		people[ssn].isSenator = false;
	}
}

/******************************************/
/* 		  	    Senator Data 			  */
/******************************************/

int numSenators = 10;
/* Senators are stored inside of customers array. See above. */

int isSenatorPresent = 0; /* Used to determine whether or not customers should wait for senators to leave PPOffice */

int senatorIndoorLock; /* Synchronizes isSenatorPresent */
int senatorIndoorCV; /* Wait on this whenever a senator is inside PPOffice */
/* TODO: Is this necessary? */
/*	int senatorOutdoorCV; */

void InitializeSenatorData ()
{
	int ssn;

	for (ssn = numCustomers; ssn < (numCustomers + numSenators); ssn++)
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
		people[ssn].ssn = ssn;
		people[ssn].money = InitialMoney();
		people[ssn].isSenator = true;
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
enum clerktype { APPLICATION, PICTURE, PASSPORT, CASHIER };

struct Clerk {
	enum clerkstate state;
	int currentCustomer;
	int money;

	int lineLength;
	int bribeLineLength;
	int senatorLineLength;

	bool isBeingBribed;
	bool customerLikedPhoto;
	bool customerAppReadyToCertify;
	bool customerAppReadyForPayment;
};

void InitializeClerkData (Clerk clerks[5], int numClerks)
{
	int clerkID;

	for (clerkID = 0; clerkID < numClerks; clerkID++)
	{
		clerks[i].state = AVAILABLE;
		clerks[i].currentCustomer = -1;
		clerks[i].money = 0;

		clerks[i].lineLength = 0;
		clerks[i].bribeLineLength = 0;
		clerks[i].senatorLineLength = 0;

		clerks[i].isBeingBribed = false;
		clerks[i].customerLikedPhoto = false;
		clerks[i].customerAppReadyToCertify = false;
		clerks[i].customerAppReadyForPayment = false;
	}
}

/******************************************/
/* 			  Clerk Group Data 			  */
/******************************************/

int numAppClerks = 5;
struct Clerk appclerks[5];
int appclerkLocks[5];
int appclerkLineCVs[5];
int appclerkBribeLineCVs[5];
int appclerkSenatorLineCVs[5];
int appclerkWorkCVs[5];
int appclerkBribeCVs[5];
int appclerkBreakCVs[5];

int numPicClerks = 5;
struct Clerk picclerks[5];
int picclerkLocks[5];
int picclerkLineCVs[5];
int picclerkBribeLineCVs[5];
int picclerkSenatorLineCVs[5];
int picclerkWorkCVs[5];
int picclerkBribeCVs[5];
int picclerkBreakCVs[5];

int numPassportClerks = 5;
struct Clerk passportclerks[5];
int passportclerkLocks[5];
int passportclerkLineCVs[5];
int passportclerkBribeLineCVs[5];
int passportclerkSenatorLineCVs[5];
int passportclerkWorkCVs[5];
int passportclerkBribeCVs[5];
int passportclerkBreakCVs[5];

int numCashiers = 5;
struct Clerk cashiers[5];
int cashierLocks[5];
int cashierLineCVs[5];
int cashierBribeLineCVs[5];
int cashierSenatorLineCVs[5];
int cashierWorkCVs[5];
int cashierBribeCVs[5];
int cashierBreakCVs[5];

struct ClerkGroup {
	Clerk clerks[5];
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

void InitializeApplicationClerkData ()
{
	int clerkType = APPLICATION;

	InitializeClerkData(appclerks, numAppClerks);

	appclerkLocks[0] = CreateLock("App:0-ClerkLock", 15);
	appclerkLocks[1] = CreateLock("App:1-ClerkLock", 15);
	appclerkLocks[2] = CreateLock("App:2-ClerkLock", 15);
	appclerkLocks[3] = CreateLock("App:3-ClerkLock", 15);
	appclerkLocks[4] = CreateLock("App:4-ClerkLock", 15);

	appclerkLineCVs[0] = CreateCV("App:0-LineCV", 12);
	appclerkLineCVs[1] = CreateCV("App:1-LineCV", 12);
	appclerkLineCVs[2] = CreateCV("App:2-LineCV", 12);
	appclerkLineCVs[3] = CreateCV("App:3-LineCV", 12);
	appclerkLineCVs[4] = CreateCV("App:4-LineCV", 12);

	appclerkBribeLineCVs[0] = CreateCV("App:0-BribeLineCV", 17);
	appclerkBribeLineCVs[1] = CreateCV("App:1-BribeLineCV", 17);
	appclerkBribeLineCVs[2] = CreateCV("App:2-BribeLineCV", 17);
	appclerkBribeLineCVs[3] = CreateCV("App:3-BribeLineCV", 17);
	appclerkBribeLineCVs[4] = CreateCV("App:4-BribeLineCV", 17);

	appclerkSenatorLineCVs[0] = CreateCV("App:0-SenatorLineCV", 19);
	appclerkSenatorLineCVs[1] = CreateCV("App:1-SenatorLineCV", 19);
	appclerkSenatorLineCVs[2] = CreateCV("App:2-SenatorLineCV", 19);
	appclerkSenatorLineCVs[3] = CreateCV("App:3-SenatorLineCV", 19);
	appclerkSenatorLineCVs[4] = CreateCV("App:4-SenatorLineCV", 19);

	appclerkWorkCVs[0] = CreateCV("App:0-WorkCV", 12);
	appclerkWorkCVs[1] = CreateCV("App:1-WorkCV", 12);
	appclerkWorkCVs[2] = CreateCV("App:2-WorkCV", 12);
	appclerkWorkCVs[3] = CreateCV("App:3-WorkCV", 12);
	appclerkWorkCVs[4] = CreateCV("App:4-WorkCV", 12);

	appclerkBribeCVs[0] = CreateCV("App:0-BribeCV", 13);
	appclerkBribeCVs[1] = CreateCV("App:1-BribeCV", 13);
	appclerkBribeCVs[2] = CreateCV("App:2-BribeCV", 13);
	appclerkBribeCVs[3] = CreateCV("App:3-BribeCV", 13);
	appclerkBribeCVs[4] = CreateCV("App:4-BribeCV", 13);

	appclerkBreakCVs[0] = CreateCV("App:0-BreakCV", 13);
	appclerkBreakCVs[1] = CreateCV("App:1-BreakCV", 13);
	appclerkBreakCVs[2] = CreateCV("App:2-BreakCV", 13);
	appclerkBreakCVs[3] = CreateCV("App:3-BreakCV", 13);
	appclerkBreakCVs[4] = CreateCV("App:4-BreakCV", 13);

	clerkGroups[clerkType].clerks = appclerks;
	clerkGroups[clerkType].numClerks = numAppClerks;

	clerkGroups[clerkType].moneyLock = CreateLock("ApplicationClerks-MoneyLock", 27);
	clerkGroups[clerkType].groupMoney = 0;

	clerkGroups[clerkType].lineLock = CreateLock("ApplicationClerks-LineLock", 26);
	clerkGroups[clerkType].lineCVs = appclerkLineCVs;
	clerkGroups[clerkType].bribeLineCVs = appclerkBribeLineCVs;
	clerkGroups[clerkType].senatorLineCVs = appclerkSenatorLineCVs;

	clerkGroups[clerkType].clerkLocks = appclerkLocks;
	clerkGroups[clerkType].workCVs = appclerkWorkCVs;
	clerkGroups[clerkType].bribeCVs = appclerkBribeCVs;
	clerkGroups[clerkType].breakCVs = appclerkBreakCVs;
}

void InitializePictureClerkData ()
{
	int clerkType = PICTURE;

	InitializeClerkData(picclerks, numPicClerks);

	picclerkLocks[0] = CreateLock("Pic:0-ClerkLock", 15);
	picclerkLocks[1] = CreateLock("Pic:1-ClerkLock", 15);
	picclerkLocks[2] = CreateLock("Pic:2-ClerkLock", 15);
	picclerkLocks[3] = CreateLock("Pic:3-ClerkLock", 15);
	picclerkLocks[4] = CreateLock("Pic:4-ClerkLock", 15);

	picclerkLineCVs[0] = CreateCV("Pic:0-LineCV", 12);
	picclerkLineCVs[1] = CreateCV("Pic:1-LineCV", 12);
	picclerkLineCVs[2] = CreateCV("Pic:2-LineCV", 12);
	picclerkLineCVs[3] = CreateCV("Pic:3-LineCV", 12);
	picclerkLineCVs[4] = CreateCV("Pic:4-LineCV", 12);

	picclerkBribeLineCVs[0] = CreateCV("Pic:0-BribeLineCV", 17);
	picclerkBribeLineCVs[1] = CreateCV("Pic:1-BribeLineCV", 17);
	picclerkBribeLineCVs[2] = CreateCV("Pic:2-BribeLineCV", 17);
	picclerkBribeLineCVs[3] = CreateCV("Pic:3-BribeLineCV", 17);
	picclerkBribeLineCVs[4] = CreateCV("Pic:4-BribeLineCV", 17);

	picclerkSenatorLineCVs[0] = CreateCV("Pic:0-SenatorLineCV", 19);
	picclerkSenatorLineCVs[1] = CreateCV("Pic:1-SenatorLineCV", 19);
	picclerkSenatorLineCVs[2] = CreateCV("Pic:2-SenatorLineCV", 19);
	picclerkSenatorLineCVs[3] = CreateCV("Pic:3-SenatorLineCV", 19);
	picclerkSenatorLineCVs[4] = CreateCV("Pic:4-SenatorLineCV", 19);

	picclerkWorkCVs[0] = CreateCV("Pic:0-WorkCV", 12);
	picclerkWorkCVs[1] = CreateCV("Pic:1-WorkCV", 12);
	picclerkWorkCVs[2] = CreateCV("Pic:2-WorkCV", 12);
	picclerkWorkCVs[3] = CreateCV("Pic:3-WorkCV", 12);
	picclerkWorkCVs[4] = CreateCV("Pic:4-WorkCV", 12);

	picclerkBribeCVs[0] = CreateCV("Pic:0-BribeCV", 13);
	picclerkBribeCVs[1] = CreateCV("Pic:1-BribeCV", 13);
	picclerkBribeCVs[2] = CreateCV("Pic:2-BribeCV", 13);
	picclerkBribeCVs[3] = CreateCV("Pic:3-BribeCV", 13);
	picclerkBribeCVs[4] = CreateCV("Pic:4-BribeCV", 13);

	picclerkBreakCVs[0] = CreateCV("Pic:0-BreakCV", 13);
	picclerkBreakCVs[1] = CreateCV("Pic:1-BreakCV", 13);
	picclerkBreakCVs[2] = CreateCV("Pic:2-BreakCV", 13);
	picclerkBreakCVs[3] = CreateCV("Pic:3-BreakCV", 13);
	picclerkBreakCVs[4] = CreateCV("Pic:4-BreakCV", 13);

	clerkGroups[clerkType].clerks = picclerks;
	clerkGroups[clerkType].numClerks = numPicClerks;

	clerkGroups[clerkType].moneyLock = CreateLock("PictureClerks-MoneyLock", 23);
	clerkGroups[clerkType].groupMoney = 0;

	clerkGroups[clerkType].lineLock = CreateLock("PictureClerks-LineLock", 22);
	clerkGroups[clerkType].lineCVs = picclerkLineCVs;
	clerkGroups[clerkType].bribeLineCVs = picclerkBribeLineCVs;
	clerkGroups[clerkType].senatorLineCVs = picclerkSenatorLineCVs;

	clerkGroups[clerkType].clerkLocks = picclerkLocks;
	clerkGroups[clerkType].workCVs = picclerkWorkCVs;
	clerkGroups[clerkType].bribeCVs = picclerkBribeCVs;
	clerkGroups[clerkType].breakCVs = picclerkBreakCVs;
}

void InitializePassportClerkData ()
{
	int clerkType = PASSPORT;

	InitializeClerkData(passportclerks, numPassportClerks);

	passportclerkLocks[0] = CreateLock("Pas:0-ClerkLock", 15);
	passportclerkLocks[1] = CreateLock("Pas:1-ClerkLock", 15);
	passportclerkLocks[2] = CreateLock("Pas:2-ClerkLock", 15);
	passportclerkLocks[3] = CreateLock("Pas:3-ClerkLock", 15);
	passportclerkLocks[4] = CreateLock("Pas:4-ClerkLock", 15);

	passportclerkLineCVs[0] = CreateCV("Pas:0-LineCV", 12);
	passportclerkLineCVs[1] = CreateCV("Pas:1-LineCV", 12);
	passportclerkLineCVs[2] = CreateCV("Pas:2-LineCV", 12);
	passportclerkLineCVs[3] = CreateCV("Pas:3-LineCV", 12);
	passportclerkLineCVs[4] = CreateCV("Pas:4-LineCV", 12);

	passportclerkBribeLineCVs[0] = CreateCV("Pas:0-BribeLineCV", 17);
	passportclerkBribeLineCVs[1] = CreateCV("Pas:1-BribeLineCV", 17);
	passportclerkBribeLineCVs[2] = CreateCV("Pas:2-BribeLineCV", 17);
	passportclerkBribeLineCVs[3] = CreateCV("Pas:3-BribeLineCV", 17);
	passportclerkBribeLineCVs[4] = CreateCV("Pas:4-BribeLineCV", 17);

	passportclerkSenatorLineCVs[0] = CreateCV("Pas:0-SenatorLineCV", 19);
	passportclerkSenatorLineCVs[1] = CreateCV("Pas:1-SenatorLineCV", 19);
	passportclerkSenatorLineCVs[2] = CreateCV("Pas:2-SenatorLineCV", 19);
	passportclerkSenatorLineCVs[3] = CreateCV("Pas:3-SenatorLineCV", 19);
	passportclerkSenatorLineCVs[4] = CreateCV("Pas:4-SenatorLineCV", 19);

	passportclerkWorkCVs[0] = CreateCV("Pas:0-WorkCV", 12);
	passportclerkWorkCVs[1] = CreateCV("Pas:1-WorkCV", 12);
	passportclerkWorkCVs[2] = CreateCV("Pas:2-WorkCV", 12);
	passportclerkWorkCVs[3] = CreateCV("Pas:3-WorkCV", 12);
	passportclerkWorkCVs[4] = CreateCV("Pas:4-WorkCV", 12);

	passportclerkBribeCVs[0] = CreateCV("Pas:0-BribeCV", 13);
	passportclerkBribeCVs[1] = CreateCV("Pas:1-BribeCV", 13);
	passportclerkBribeCVs[2] = CreateCV("Pas:2-BribeCV", 13);
	passportclerkBribeCVs[3] = CreateCV("Pas:3-BribeCV", 13);
	passportclerkBribeCVs[4] = CreateCV("Pas:4-BribeCV", 13);

	passportclerkBreakCVs[0] = CreateCV("Pas:0-BreakCV", 13);
	passportclerkBreakCVs[1] = CreateCV("Pas:1-BreakCV", 13);
	passportclerkBreakCVs[2] = CreateCV("Pas:2-BreakCV", 13);
	passportclerkBreakCVs[3] = CreateCV("Pas:3-BreakCV", 13);
	passportclerkBreakCVs[4] = CreateCV("Pas:4-BreakCV", 13);

	clerkGroups[clerkType].clerks = passportclerks;
	clerkGroups[clerkType].numClerks = numPassportClerks;

	clerkGroups[clerkType].moneyLock = CreateLock("PassportClerks-MoneyLock", 24);
	clerkGroups[clerkType].groupMoney = 0;

	clerkGroups[clerkType].lineLock = CreateLock("PassportClerks-LineLock", 23);
	clerkGroups[clerkType].lineCVs = passportclerkLineCVs;
	clerkGroups[clerkType].bribeLineCVs = passportclerkBribeLineCVs;
	clerkGroups[clerkType].senatorLineCVs = passportclerkSenatorLineCVs;

	clerkGroups[clerkType].clerkLocks = passportclerkLocks;
	clerkGroups[clerkType].workCVs = passportclerkWorkCVs;
	clerkGroups[clerkType].bribeCVs = passportclerkBribeCVs;
	clerkGroups[clerkType].breakCVs = passportclerkBreakCVs;
}

void InitializeCashierData ()
{
	int clerkType = CASHIER;

	InitializeClerkData(cashiers, numCashiers);

	cashierLocks[0] = CreateLock("Csh:0-ClerkLock", 15);
	cashierLocks[1] = CreateLock("Csh:1-ClerkLock", 15);
	cashierLocks[2] = CreateLock("Csh:2-ClerkLock", 15);
	cashierLocks[3] = CreateLock("Csh:3-ClerkLock", 15);
	cashierLocks[4] = CreateLock("Csh:4-ClerkLock", 15);

	cashierLineCVs[0] = CreateCV("Csh:0-LineCV", 12);
	cashierLineCVs[1] = CreateCV("Csh:1-LineCV", 12);
	cashierLineCVs[2] = CreateCV("Csh:2-LineCV", 12);
	cashierLineCVs[3] = CreateCV("Csh:3-LineCV", 12);
	cashierLineCVs[4] = CreateCV("Csh:4-LineCV", 12);

	cashierBribeLineCVs[0] = CreateCV("Csh:0-BribeLineCV", 17);
	cashierBribeLineCVs[1] = CreateCV("Csh:1-BribeLineCV", 17);
	cashierBribeLineCVs[2] = CreateCV("Csh:2-BribeLineCV", 17);
	cashierBribeLineCVs[3] = CreateCV("Csh:3-BribeLineCV", 17);
	cashierBribeLineCVs[4] = CreateCV("Csh:4-BribeLineCV", 17);

	cashierSenatorLineCVs[0] = CreateCV("Csh:0-SenatorLineCV", 19);
	cashierSenatorLineCVs[1] = CreateCV("Csh:1-SenatorLineCV", 19);
	cashierSenatorLineCVs[2] = CreateCV("Csh:2-SenatorLineCV", 19);
	cashierSenatorLineCVs[3] = CreateCV("Csh:3-SenatorLineCV", 19);
	cashierSenatorLineCVs[4] = CreateCV("Csh:4-SenatorLineCV", 19);

	cashierWorkCVs[0] = CreateCV("Csh:0-WorkCV", 12);
	cashierWorkCVs[1] = CreateCV("Csh:1-WorkCV", 12);
	cashierWorkCVs[2] = CreateCV("Csh:2-WorkCV", 12);
	cashierWorkCVs[3] = CreateCV("Csh:3-WorkCV", 12);
	cashierWorkCVs[4] = CreateCV("Csh:4-WorkCV", 12);

	cashierBribeCVs[0] = CreateCV("Csh:0-BribeCV", 13);
	cashierBribeCVs[1] = CreateCV("Csh:1-BribeCV", 13);
	cashierBribeCVs[2] = CreateCV("Csh:2-BribeCV", 13);
	cashierBribeCVs[3] = CreateCV("Csh:3-BribeCV", 13);
	cashierBribeCVs[4] = CreateCV("Csh:4-BribeCV", 13);

	cashierBreakCVs[0] = CreateCV("Csh:0-BreakCV", 13);
	cashierBreakCVs[1] = CreateCV("Csh:1-BreakCV", 13);
	cashierBreakCVs[2] = CreateCV("Csh:2-BreakCV", 13);
	cashierBreakCVs[3] = CreateCV("Csh:3-BreakCV", 13);
	cashierBreakCVs[4] = CreateCV("Csh:4-BreakCV", 13);

	clerkGroups[clerkType].clerks = cashiers;
	clerkGroups[clerkType].numClerks = numCashiers;

	clerkGroups[clerkType].moneyLock = CreateLock("Cashiers-MoneyLock", 18);
	clerkGroups[clerkType].groupMoney = 0;

	clerkGroups[clerkType].lineLock = CreateLock("Cashiers-LineLock", 17);
	clerkGroups[clerkType].lineCVs = cashierLineCVs;
	clerkGroups[clerkType].bribeLineCVs = cashierBribeLineCVs;
	clerkGroups[clerkType].senatorLineCVs = cashierSenatorLineCVs;

	clerkGroups[clerkType].clerkLocks = cashierLocks;
	clerkGroups[clerkType].workCVs = cashierWorkCVs;
	clerkGroups[clerkType].bribeCVs = cashierBribeCVs;
	clerkGroups[clerkType].breakCVs = cashierBreakCVs;
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
	manager.appclerkMoney = 0;
	manager.picclerkMoney = 0;
	manager.passportclerkMoney = 0;
	manager.cashierMoney = 0;
	manager.totalMoney = 0;
}

/******************************************/
/* 			 System Job Data 			  */
/******************************************/

struct SystemJob {
	int ssn;
	int clerkID;
};

int numSystemJobs = 50;
int jobs[50];

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

/* ========================================================================================================================================= */
/*																																			 */
/*		CUSTOMER																															 */
/*																																			 */
/* ========================================================================================================================================= */

int DecideLine (int ssn, int clerkType)
{
	int lineLock;
	int numClerks;
	int clerksData[5];
	int clerkLocks[5];
	int clerkWorkCVs[5];
	int bribeCVs[5];
	int lineCVs[5];
	int bribeLineCVs[5];
	int senatorLineCVs[5];

	int i;

	int currentLine = -1;
	int currentLineLength = 1000;

	int shortestLine = -1;
	int shortestLineLength = 1000;

	int clerkLineLength; /* Keeps track of either normal line count or senator line count depending on if isSenator */

	lineLock = clerkGroupData[clerkType][index_lineLock];
	numClerks = clerkCounts[clerkType];
	clerksData = clerkGroupData[clerkType][index_clerksData];
	clerkLocks = clerkGroupData[clerkType][index_clerkLocks];
	clerkWorkCVs = clerkGroupData[clerkType][index_clerkWorkCVs];
	bribeCVs = clerkGroupData[clerkType][index_bribeCVs];
	lineCVs = clerkGroupData[clerkType][index_lineCVs];
	bribeLineCVs = clerkGroupData[clerkType][index_bribeLineCVs];
	senatorLineCVs = clerkGroupData[clerkType][index_senatorLineCVs];

	/*  TODO: Arrays were cast as unsigned ints, how to convert/cast back to array; or just how to access data inside array, otherwise? */

	/* Check if the senator is present, and if so, "go outside" by waiting on the CV. */
	/* 	By placing this here, we ensure line order remains consistent, conveniently. */
	AcquireLock(senatorIndoorLock);
	if (isSenatorPresent && !isSenator)
	{
		/* TODO: Some kind of printf function for writing the required output with ints inside of strings. */
		WriteOutput("Customer %d is going outside the Passport Office because there is a Senator present.", ssn);
		WaitCV(senatorIndoorCV);
	}
	ReleaseLock(senatorIndoorLock);

	AcquireLock(lineLock);
	for (i = 0; i < numClerks; i++)
	{
		/* Different lines depending on whether customer or senator. */
		if (isSenator)
		{
			clerkLineLength = clerkData[i][index_senatorLineLength];
		}
		else
		{
			clerkLineLength = clerkData[i][index_lineLength];
		}

		/* If clerk is available, go there. */
		if (clerkLineLength == 0 && clerkData[i][index_state] == state_clerkAvailable)
		{
			currentLine = i;
			currentLineLength = clerkLineLength;
			break;
		}

		/* Pick the shortest line of clerks not on break. */
		if (clerkLineLength < shortestLineLength && clerkData[i][index_state] != state_clerkOnBreak)
		{
			currentLine = i;
			currentLineLength = clerkLineLength;
		}

		/* If everyone is on break, need to know which line is shortest so we join it. Managers will eventually wake clerks up. */
		if (clerkLineLength < shortestLineLength)
		{
			shortestLine = i;
			shortestLineLength = clerkLineLength;
		}
	}

	/* If everyone was on break, set your line to be the shortest line. */
	if (currentLine == -1)
	{
		currentLine = shortestLine;
		currentLineLength = shortestLineLength;
	}

	/* Now that customer has selected a clerk's line, figure out whether to go: */
	/* 	- Straight to the counter */
	/* 	- In line */
	/*	- Bribe */
	/*	- Senator line (if isSenator) */

	if (clerkData[currentLine][index_state] != state_clerkAvailable)
	{ /* Clerk is unavailable; Rule out going straight to counter, so now decide which line to wait in. */
		if (isSenator)
		{
			/* TODO: Can we do direct incrementation on array value? */
			clerkData[currentLine][index_senatorLineLength]++;
			/* TODO: See above. */
			/* TODO: Different approach from string array in original PPOffice for clerk type string */
			/*	WriteOutput("Senator %d has gotten in a regular line for %s %d.", ssn, ); */
			Wait(senatorLineCVs[currentLine], lineLock);
			/* TODO: See above. */
			clerkData[currentLine][index_senatorLineLength]--;
		}
		else
		{ /* Ruled out straight to counter and senator line. Bribe or no bribe? */
			if (currentLineLength >= 1 && money >= 600 && clerkData[index_state] != state_clerkOnBreak)
			{ /* If customer has enough money and she's not in a line for a clerk that is on break, always bribe. */
				/* Let clerk know you are trying to bribe her. */
				/* TODO: See above. */
				clerkData[currentLine][index_isBeingBribed]++;
				Wait(bribeCVs[currentLine], lineLock);
				/* TODO: See above. */
				clerkData[currentLine][index_isBeingBribed]--;
				ReleaseLock(lineLock);

				/* Do customer side of bribe. */
				AcquireLock(clerkLocks[currentLine]);

				clerkData[currentLine][index_currentCustomer] = ssn;
				money -= 500;
				Signal(clerkWorkCVs[currentLine], lineLock);
				Wait(clerkWorkCVs[currentLine], lineLock);
				/* TODO: See above. (WriteOutput) */
				/* TODO: See above. (string array) */
				/*	WriteOutput("Customer %d has gotten in bribe line for %s %d.", ssn, ); */
				
				ReleaseLock(clerkLocks[currentLine]);

				/* Now get into bribe line. */
				AcquireLock(lineLock);

				/* TODO: See above. (increment operator) */
				clerkData[currentLine][index_bribeLineLength]++;
				Wait(bribeLineCVs[currentLine], lineLock);

				/* Woken up. Make sure no senators have entered so that I can do my business with clerk. */
				AcquireLock(senatorIndoorLock);
				if (isSenatorPresent && !isSenator)
				{
					/* TODO: See above. (WriteOutput) */
					WriteOutput("Customer %d is going outside the Passport Office because there is a Senator present.", ssn);
					Wait(senatorIndoorCV, senatorIndoorLock);
					ReleaseLock(senatorIndoorLock); /* Lock gets reacquired inside of Wait, release it and re-decide line. */
					return DecideLine(ssn, money, clerkType, isSenator);
				}
				else
				{
					ReleaseLock(senatorIndoorLock);
				}

				/* Made it out of line. */
				/* TODO: See above. (Decrement operator) */
				clerkData[currentLine][index_bribeLineLength]--;
			}
			else
			{ /* No other options. Get in regular line. */
				/* TODO: See above. (Increment operator) */
				clerkData[currentLine][index_lineLength]++;
				/* TODO: See above. (WriteOutput) */
				/* WriteOutput("Customer %d has gotten in a regular line for %s %d.", ssn, currentLine); */
				Wait(lineCVs[currentLine], lineLock);

				/* Woken up. Make sure no senators have entered so that I can do my business with clerk. */
				AcquireLock(senatorIndoorLock);
				if (isSenatorPresent && !isSenator)
				{
					/* TODO: See above. (WriteOutput) */
					WriteOutput("Customer %d is going outside the Passport Office because there is a Senator present.", ssn);
					Wait(senatorIndoorCV, senatorIndoorLock);
					ReleaseLock(senatorIndoorLock); /* Lock gets reacquired inside of Wait, release it and re-decide line. */
					return DecideLine(ssn, money, clerkType, isSenator);
				}
				else
				{
					ReleaseLock(senatorIndoorLock);
				}

				/* Made it out of line. */
				/* TODO: See above. (Decrement operator) */
				clerkData[currentLine][index_lineLength]--;
			}
		}
	}
	else
	{ /* Line was empty when customer joined, go straight to the counter. */
		/* TODO: Will this produce the same weird error as before when in C++ */
		/* 	Before needed to do: clerkGroupData[clerkType].clerkData[currentLine].state = BUSY; */
		clerkData[index_state] = state_clerkBusy;
	}
	ReleaseLock(lineLock);

	return currentLine;
}

int main () 
{
	InitializeData();
}