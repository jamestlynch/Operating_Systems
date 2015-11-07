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
int threadParam = 0;
int paramLock;
int threadInitializedCV;

/******************************************/
/* 		  PPOffice Simulation Data   	  */
/******************************************/

/* We need a way of knowing whether the simulation has finished without 	*/
/*	busy waiting. We achieve this by adding the following: 					*/
/*	-	numCustomersFinished - keeps track of amount finished so Clerks 	*/
/*		  know when to Exit and Managers know when to call WakeUpAllClerks	*/
/*	-	customersFinishedLock - synchronize updates/reads of 				*/
/*		  numCustomersFinished.												*/
/*	-	numClerksFinished - last Clerk needs to know when to Signal to 		*/
/*		  Manager that all clerks are finished. 							*/
/*	-	clerksFinishedLock - synchronize updates/reads of numClerksFinished */
/*	-	allAgentsFinishedCV – the condition of whether or not there are  	*/
/*		  any unterminated threads left in the simulation. 					*/
/*	Note: It may be more *correct* to acquire the customersFinishedLock 	*/
/* 	  inside of the Clerks/Manager loops, but we do not do that. While it 	*/
/*	  would save a "waisted" loop where the clerks are doing work they do  	*/
/*	  not need to do, it would create deadlock whenever a Clerk gets 		*/
/*	  context switched in the middle of checking the condition. This likely */
/*	  occurs more often than the number of waisted loops.					*/
int numCustomersFinished = 0;
int numClerksFinished = 0;
int customersFinishedLock;
int clerksFinishedLock;
int allClerksFinishedCV;
int allAgentsFinishedCV;

/******************************************/
/* 		  	    Person Data 			  */
/******************************************/

typedef enum { APPLICATION, PICTURE, PASSPORT, CASHIER, CUSTOMER, SENATOR, MANAGER } persontype;

struct Person {
	int id;
	int money;
	persontype type;
};

struct Person people[81]; /* numCustomers + numSenators + numAppClerks + numPicClerks + numPassportClerks + numCashiers + numManagers */

/******************************************/
/* 		  	   Customer Data 			  */
/******************************************/

typedef struct Customer {
	bool turnedInApplication;
	bool acceptedPassport;
	bool gotPassport;
	bool applicationFiled;
	bool pictureFiled;
	bool passportCertified;
	bool passportRecorded;
} Customer;

int moneyOptions[4] = {100, 600, 1100, 1600};

int numCustomers = 50;
Customer customers[60]; /* Same info for customers/senators: SIZE = numCustomers + numSenators */


/* Randomly select either $100, $600, $1100, or $1600. */
int InitialMoney()
{
	int moneyIndex;

	moneyIndex = Random(0, 4);

	return moneyOptions[moneyIndex];
}

/* Initialize data for number of customers in the simulation. Also add 	*/
/*	to the people collection so all we need to know is their ssn to 	*/
/*	find out more info about them (SENATOR or not, etc.) 				*/
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
/* Senators are stored inside of customers array. See above. */

int isSenatorPresent = 0; /* Used to determine whether or not customers should wait for senators to leave PPOffice */

int senatorPresentLock; /* Synchronizes isSenatorPresent */
int senatorPresentCV; /* Wait on this whenever a senator is inside PPOffice */
/* TODO: Is this necessary? */
/*	int senatorOutdoorCV; */

/* Initialize data for number of senators in the simulation. Add		*/
/*	senator to people collection so all we need to know is their ssn to */
/*	find out more info about them (SENATOR or not, etc.). Also create 	*/
/*	the locks and CVs so any agent can check if a senator is present. 	*/
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

typedef enum { AVAILABLE, BUSY, ONBREAK } clerkstate;
typedef enum { DOINTERACTION, TAKEBREAK, ACCEPTBRIBE } clerkinteraction;
typedef enum { NORMALLINE, BRIBELINE, SENATORLINE } linetype;

typedef struct Clerk {
	clerkstate state;
	int currentCustomer;
	int money;

	int lineLength;
	int bribeLineLength;
	int senatorLineLength;

	int numCustomersBribing;
	bool customerLikedPhoto;
	bool customerAppReadyToCertify;
	bool customerAppReadyForPayment;
} Clerk;

int numAppClerks = 5;
int numPicClerks = 5;
int numPassportClerks = 5;
int numCashiers = 5;

typedef struct ClerkGroup {
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
} ClerkGroup;

ClerkGroup clerkGroups[4];

/* Initialize data for a single type of clerk. Add to the people array	*/
/* 	so that we can figure out all other info about clerk from ssn 		*/
/*	(their type tells us which clerkGroup to look inside, their clerkID */
/*	tells us which clerk they are inside that group). 					*/
void InitializeClerkData (int numClerks, persontype clerkType)
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

/* Initialize data for a group of clerks: APPLICATION. Needed to create */
/*	a method for initializing each type of clerk group because we were  */
/*  unable to create arrays of strings / the name creation is the bulk  */
/*  of the work for initilizing a clerk group.							*/
void InitializeApplicationClerkData ()
{
	persontype clerkType = APPLICATION;

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

/* Initialize data for a group of clerks: PICTURE. Needed to create a	*/
/*	method for initializing each type of clerk group because we were 	*/
/*  unable to create arrays of strings / the name creation is the bulk  */
/*  of the work for initilizing a clerk group.							*/
void InitializePictureClerkData ()
{
	persontype clerkType = PICTURE;

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

/* Initialize data for a group of clerks: PASSPORT. Needed to create a	*/
/*	method for initializing each type of clerk group because we were 	*/
/*  unable to create arrays of strings / the name creation is the bulk  */
/*  of the work for initilizing a clerk group.							*/
void InitializePassportClerkData ()
{
	persontype clerkType = PASSPORT;

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

/* Initialize data for a group of clerks: CASHIER. Needed to create a	*/
/*	method for initializing each type of clerk group because we were 	*/
/*  unable to create arrays of strings / the name creation is the bulk  */
/*  of the work for initilizing a clerk group.							*/
void InitializeCashierData ()
{
	persontype clerkType = CASHIER;

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

typedef struct Manager {
	int appclerkMoney;
	int picclerkMoney;
	int passportclerkMoney;
	int cashierMoney;
	int totalMoney;
} Manager;

Manager manager;

/* Initizes a single Manager's data. We only need one Manager. */
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

typedef struct SystemJob {
	int ssn;
	int clerkID;
	persontype type;
} SystemJob;

int numSystemJobs = 50;
SystemJob jobs[50];

int systemJobFindLock; /* Used to synchronize search for available system jobs */
int filingPictureLock;
int filingApplicationLock;
int certifyingPassportLock;

/* Initializes a collection of SystemJobs and the Locks synchronizing 	*/
/*	the data entries that they must update. 							*/
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

typedef enum { 
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
} outputstatement;

/* Writes the necessary system output for the PassportOffice simulation */
/* 	to the console. Since we are unable to pass pointers around, this 	*/
/*	method allows us to duplicate code for Clerks/Customers while still */
/*	calling the appropriate output by passing in their persontype and 	*/
/*	the persontype of who they are interacting with.					*/
/* Params: 	statement – which output statement or type of output 		*/
/*			  statement do we need to output. Maps to output string. 	*/
/*			clerkType - the type of the clerk interacting				*/
/*			customerType - whether SENATOR or normal CUSTOMER 			*/
/*			ssn - (also money) the ssn of the calling agent (if needed;	*/
/*			  may also pass in -1 if not)								*/
/*			clerkID - the identifier of the clerk (if not needed, -1)	*/
void WriteOutput (outputstatement statement, persontype clerkType, persontype customerType, int ssn, int clerkID)
{
	int money; /* Only Manager print statements need money */

	money = ssn;

	switch (statement)
	{
		case Clerk_SignalledCustomer:
			switch(clerkType)
			{
				case APPLICATION:
					PrintfOne("ApplicationClerk %d has signalled a Customer to come to their counter.\n", 
						sizeof("ApplicationClerk %d has signalled a Customer to come to their counter.\n"), 
						clerkID);
					break;
				case PICTURE:
					PrintfOne("PictureClerk %d has signalled a Customer to come to their counter.\n", 
						sizeof("PictureClerk %d has signalled a Customer to come to their counter.\n"),
						clerkID);
					break;
				case PASSPORT:
					PrintfOne("PassportClerk %d has signalled a Customer to come to their counter.\n",
						sizeof("PassportClerk %d has signalled a Customer to come to their counter.\n"),
						clerkID);
					break;
				case CASHIER:
					PrintfOne("Cashier %d has signalled a Customer to come to their counter.\n",
						sizeof("Cashier %d has signalled a Customer to come to their counter.\n"),
						clerkID);
					break;
			}
			break;
		case Clerk_ReceivedSSN:
			switch(clerkType)
			{
				case APPLICATION:
					PrintfTwo("ApplicationClerk %d has received SSN from Customer %d\n",
						sizeof("ApplicationClerk %d has received SSN from Customer %d\n"),
						clerkID,
						ssn);
					break;
				case PICTURE:
					PrintfTwo("PictureClerk %d has received SSN from Customer %d\n", 
						sizeof("PictureClerk %d has received SSN from Customer %d\n"),
						clerkID,
						ssn);
					break;
				case PASSPORT:
					PrintfTwo("PassportClerk %d has received SSN from Customer %d\n",
						sizeof("PassportClerk %d has received SSN from Customer %d\n"),
						clerkID,
						ssn);
					break;
				case CASHIER:
					PrintfTwo("Cashier %d has received SSN from Customer %d\n",
						sizeof("Cashier %d has received SSN from Customer %d\n"),
						clerkID,
						ssn);
					break;
			}
			break;
		case Clerk_GoingOnBreak:
			switch(clerkType)
			{
				case APPLICATION:
					PrintfOne("ApplicationClerk %d is going on break\n",
						sizeof("ApplicationClerk %d is going on break\n"),
						clerkID);
					break;
				case PICTURE:
					PrintfOne("PictureClerk %d is going on break\n",
						sizeof("PictureClerk %d is going on break\n"),
						clerkID);
					break;
				case PASSPORT:
					PrintfOne("PassportClerk %d is going on break\n",
						sizeof("PassportClerk %d is going on break\n"),
						clerkID);
					break;
				case CASHIER:
					PrintfOne("Cashier %d is going on break\n",
						sizeof("Cashier %d is going on break\n"),
						clerkID);
					break;
			}
			break;
		case Clerk_ComingOffBreak:
			switch(clerkType)
			{
				case APPLICATION:
					PrintfOne("ApplicationClerk %d is coming off break\n",
						sizeof("ApplicationClerk %d is coming off break\n"),
						clerkID);
					break;
				case PICTURE:
					PrintfOne("PictureClerk %d is coming off break\n",
						sizeof("PictureClerk %d is coming off break\n"),
						clerkID);
					break;
				case PASSPORT:
					PrintfOne("PassportClerk %d is coming off break\n",
						sizeof("PassportClerk %d is coming off break\n"),
						clerkID);
					break;
				case CASHIER:
					PrintfOne("Cashier %d is coming off break\n",
						sizeof("Cashier %d is coming off break\n"),
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
							PrintfTwo("ApplicationClerk %d has recorded a completed application for Customer %d\n",
								sizeof("ApplicationClerk %d has recorded a completed application for Customer %d\n"),
								clerkID,
								ssn);
							break;
						case SENATOR:
							PrintfTwo("ApplicationClerk %d has recorded a completed application for Senator %d\n",
								sizeof("ApplicationClerk %d has recorded a completed application for Senator %d\n"),
								clerkID,
								ssn);
							break;
					}
					break;
				case PICTURE:	
					switch(customerType)
					{
						case CUSTOMER:
							PrintfTwo("PictureClerk %d has filed a picture for Customer %d\n",
								sizeof("PictureClerk %d has filed a picture for Customer %d\n"),
								clerkID,
								ssn);
							break;
						case SENATOR:
							PrintfTwo("PictureClerk %d has filed a picture for Senator %d\n",
								sizeof("PictureClerk %d has filed a picture for Senator %d\n"),
								clerkID,
								ssn);
							break;
					}
					break;
				case PASSPORT:	
					switch(customerType)
					{
						case CUSTOMER:
							PrintfTwo("PassportClerk %d has recorded Customer %d passport documentation\n",
								sizeof("PassportClerk %d has recorded Customer %d passport documentation\n"),
								clerkID,
								ssn);
							break;
						case SENATOR:
							PrintfTwo("PassportClerk %d has recorded Senator %d passport documentation\n",
								sizeof("PassportClerk %d has recorded Senator %d passport documentation\n"),
								clerkID,
								ssn);
							break;
					}
					break;
			}
			break;
		case Clerk_ReceivedBribe:
			PrintfTwo("ApplicationClerk %d has received $500 from Customer %d\n",
				sizeof("ApplicationClerk %d has received $500 from Customer %d\n"),
				clerkID,
				ssn);
			break;
		case Clerk_TookPicture:
			PrintfTwo("PictureClerk %d has taken a picture of Customer %d\n",
				sizeof("PictureClerk %d has taken a picture of Customer %d\n"),
				clerkID,
				ssn);
			break;
		case Clerk_ToldByCustomerDoesNotLikePicture:
			PrintfTwo("PictureClerk %d has been told that Customer %d does not like their picture\n",
				sizeof("PictureClerk %d has been told that Customer %d does not like their picture\n"),
				clerkID,
				ssn);
			break;
		case Clerk_ToldByCustomerDoesLikePicture:
			PrintfTwo("PictureClerk %d has been told that Customer %d does like their picture\n",
				sizeof("PictureClerk %d has been told that Customer %d does like their picture\n"),
				clerkID,
				ssn);
			break;
		case Clerk_DeterminedAppAndPicNotCompleted:
			PrintfTwo("PassportClerk %d has determined that Customer %d does not have both their application and picture completed\n",
				sizeof("PassportClerk %d has determined that Customer %d does not have both their application and picture completed\n"),
				clerkID,
				ssn);
			break;
		case Clerk_DeterminedAppAndPicCompleted:
			PrintfTwo("PassportClerk %d has determined that Customer %d has both their application and picture completed\n",
				sizeof("PassportClerk %d has determined that Customer %d has both their application and picture completed\n"),
				clerkID,
				ssn);
			break;
		case Clerk_VerifiedPassportCertified:
			PrintfTwo("Cashier %d has verified that Customer %d has been certified by a PassportClerk\n",
				sizeof("Cashier %d has verified that Customer %d has been certified by a PassportClerk\n"),
				clerkID,
				ssn);
			break;
		case Clerk_ReceivedPayment:
			PrintfTwo("Cashier %d has received the $100 from Customer %d after certification\n",
				sizeof("Cashier %d has received the $100 from Customer %d after certification\n"),
				clerkID,
				ssn);
			break;
		case Clerk_ReceivedPaymentGoBackInLine:
			PrintfTwo("Cashier %d has received the $100 from Customer %d before certification. They are to go to the back of my line.\n",
				sizeof("Cashier %d has received the $100 from Customer %d before certification. They are to go to the back of my line.\n"),
				clerkID,
				ssn);
			break;
		case Clerk_ProvidedPassport:
			PrintfTwo("Cashier %d has provided Customer %d their completed passport\n",
				sizeof("Cashier %d has provided Customer %d their completed passport\n"),
				clerkID,
				ssn);
			break;
		case Clerk_RecordedCustomerGivenPassport:
			PrintfTwo("Cashier %d has recorded that Customer %d has been given their completed passport\n",
				sizeof("Cashier %d has recorded that Customer %d has been given their completed passport\n"),
				clerkID,
				ssn);
			break;
	
		
		case Manager_WokeUpClerk:
			switch(clerkType)
			{
				case APPLICATION:
					Write("Manager has woken up an ApplicationClerk\n",
						sizeof("Manager has woken up an ApplicationClerk\n"),
						1);
					break;
				case PICTURE:
					Write("Manager has woken up an PictureClerk\n",
						sizeof("Manager has woken up an PictureClerk\n"),
						1);
					break;
				case PASSPORT:
					Write("Manager has woken up an PassportClerk\n",
						sizeof("Manager has woken up an PassportClerk\n"),
						1);
					break;
				case CASHIER:
					Write("Manager has woken up an Cashier\n",
						sizeof("Manager has woken up an Cashier\n"),
						1);
					break;
			}
			break;
		case Manager_CountedMoneyForClerk:
			switch(clerkType)
			{
				case APPLICATION:
					PrintfOne("Manager has counted a total of $%d for ApplicationClerks\n",
						sizeof("Manager has counted a total of $%d for ApplicationClerks\n"),
						money);
					break;
				case PICTURE:
					PrintfOne("Manager has counted a total of $%d for PictureClerks\n",
						sizeof("Manager has counted a total of $%d for PictureClerks\n"),
						money);
					break;
				case PASSPORT:
					PrintfOne("Manager has counted a total of $%d for PassportClerks\n",
						sizeof("Manager has counted a total of $%d for PassportClerks\n"),
						money);
					break;
				case CASHIER:
					PrintfOne("Manager has counted a total of $%d for Cashiers\n",
						sizeof("Manager has counted a total of $%d for Cashiers\n"),
						money);
					break;
			}
			break;
		case Manager_CountedTotalMoney:
			PrintfOne("Manager has counted a total of $%d for the passport office\n",
				sizeof("Manager has counted a total of $%d for the passport office\n"),
				money);
			break;
	
	
		case Customer_GotInRegularLine:
			switch(clerkType)
			{
				case APPLICATION:
					switch(customerType)
					{
						case CUSTOMER:
							PrintfTwo("Customer %d has gotten in regular line for ApplicationClerk %d.\n",
								sizeof("Customer %d has gotten in regular line for ApplicationClerk %d.\n"),
								ssn,
								clerkID);
							break;
						case SENATOR:
							PrintfTwo("Senator %d has gotten in regular line for ApplicationClerk %d.\n",
								sizeof("Senator %d has gotten in regular line for ApplicationClerk %d.\n"),
								ssn,
								clerkID);
							break;
					}
					break;
				case PICTURE:
					switch(customerType)
					{
						case CUSTOMER:
							PrintfTwo("Customer %d has gotten in regular line for PictureClerk %d.\n",
								sizeof("Customer %d has gotten in regular line for PictureClerk %d.\n"),
								ssn,
								clerkID);
							break;
						case SENATOR:
							PrintfTwo("Senator %d has gotten in regular line for PictureClerk %d.\n",
								sizeof("Senator %d has gotten in regular line for PictureClerk %d.\n"),
								ssn,
								clerkID);
							break;
					}
					break;
				case PASSPORT:
					switch(customerType)
					{
						case CUSTOMER:
							PrintfTwo("Customer %d has gotten in regular line for PassportClerk %d.\n",
								sizeof("Customer %d has gotten in regular line for PassportClerk %d.\n"),
								ssn,
								clerkID);
							break;
						case SENATOR:
							PrintfTwo("Senator %d has gotten in regular line for PassportClerk %d.\n",
								sizeof("Senator %d has gotten in regular line for PassportClerk %d.\n"),
								ssn,
								clerkID);
							break;
					}
					break;
				case CASHIER:
					switch(customerType)
					{
						case CUSTOMER:
							PrintfTwo("Customer %d has gotten in regular line for Cashier %d.\n",
								sizeof("Customer %d has gotten in regular line for Cashier %d.\n"),
								ssn,
								clerkID);
							break;
						case SENATOR:
							PrintfTwo("Senator %d has gotten in regular line for Cashier %d.\n",
								sizeof("Senator %d has gotten in regular line for Cashier %d.\n"),
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
					PrintfTwo("Customer %d has gotten in bribe line for ApplicationClerk %d.\n",
						sizeof("Customer %d has gotten in bribe line for ApplicationClerk %d.\n"),
						ssn,
						clerkID);
					break;
				case PICTURE:
					PrintfTwo("Customer %d has gotten in bribe line for PictureClerk %d.\n",
						sizeof("Customer %d has gotten in bribe line for PictureClerk %d.\n"),
						ssn,
						clerkID);
					break;
				case PASSPORT:
					PrintfTwo("Customer %d has gotten in bribe line for PassportClerk %d.\n",
						sizeof("Customer %d has gotten in bribe line for PassportClerk %d.\n"),
						ssn,
						clerkID);
					break;
				case CASHIER:
					PrintfTwo("Customer %d has gotten in bribe line for Cashier %d.\n",
						sizeof("Customer %d has gotten in bribe line for Cashier %d.\n"),
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
							PrintfTwo("Customer %d has given SSN to ApplicationClerk %d.\n", 
								sizeof("Customer %d has given SSN to ApplicationClerk %d.\n"),
								ssn,
								clerkID);
							break;
						case SENATOR:
							PrintfTwo("Senator %d has given SSN to ApplicationClerk %d.\n",
								sizeof("Senator %d has given SSN to ApplicationClerk %d.\n"),
								ssn,
								clerkID);
							break;
					}
					break;
				case PICTURE:
					switch(customerType)
					{
						case CUSTOMER:
							PrintfTwo("Customer %d has given SSN to PictureClerk %d.\n",
								sizeof("Customer %d has given SSN to PictureClerk %d.\n"),
								ssn,
								clerkID);
							break;
						case SENATOR:
							PrintfTwo("Senator %d has given SSN to PictureClerk %d.\n",
								sizeof("Senator %d has given SSN to PictureClerk %d.\n"),
								ssn, 
								clerkID);
							break;
					}
					break;
				case PASSPORT:
					switch(customerType)
					{
						case CUSTOMER:
							PrintfTwo("Customer %d has given SSN to PassportClerk %d.\n",
								sizeof("Customer %d has given SSN to PassportClerk %d.\n"),
								ssn,
								clerkID);
							break;
						case SENATOR:
							PrintfTwo("Senator %d has given SSN to PassportClerk %d.\n",
								sizeof("Senator %d has given SSN to PassportClerk %d.\n"),
								ssn,
								clerkID);
							break;
					}
					break;
				case CASHIER:
					switch(customerType)
					{
						case CUSTOMER:
							PrintfTwo("Customer %d has given SSN to Cashier %d.\n",
								sizeof("Customer %d has given SSN to Cashier %d.\n"),
								ssn,
								clerkID);
							break;
						case SENATOR:
							PrintfTwo("Senator %d has given SSN to Cashier %d.\n",
								sizeof("Senator %d has given SSN to Cashier %d.\n"),
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
					PrintfTwo("Customer %d does not like their picture from PictureClerk %d.\n",
						sizeof("Customer %d does not like their picture from PictureClerk %d.\n"),
						ssn,
						clerkID);
					break;
				case SENATOR:
					PrintfTwo("Senator %d does not like their picture from PictureClerk %d.\n",
						sizeof("Senator %d does not like their picture from PictureClerk %d.\n"),
						ssn,
						clerkID);
					break;
			}
			break;
		case Customer_DoesLikePicture:
			switch(customerType)
			{
				case CUSTOMER:
					PrintfTwo("Customer %d does like their picture from PictureClerk %d.\n",
						sizeof("Customer %d does like their picture from PictureClerk %d.\n"),
						ssn,
						clerkID);
					break;
				case SENATOR:
					PrintfTwo("Senator %d does like their picture from PictureClerk %d.\n",
						sizeof("Senator %d does like their picture from PictureClerk %d.\n"),
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
							PrintfTwo("Customer %d has gone to PassportClerk %d too soon. They are going to the back of the line.\n",
								sizeof("Customer %d has gone to PassportClerk %d too soon. They are going to the back of the line.\n"),
								ssn,
								clerkID);
							break;
						case SENATOR:
							PrintfTwo("Senator %d has gone to PassportClerk %d too soon. They are going to the back of the line.\n",
								sizeof("Senator %d has gone to PassportClerk %d too soon. They are going to the back of the line.\n"),
								ssn,
								clerkID);
							break;
					}
					break;
				case CASHIER:
					switch(customerType)
					{
						case CUSTOMER:
							PrintfTwo("Customer %d has gone to Cashier %d too soon. They are going to the back of the line.\n",
								sizeof("Customer %d has gone to Cashier %d too soon. They are going to the back of the line.\n"),
								ssn,
								clerkID);
							break;
						case SENATOR:
							PrintfTwo("Senator %d has gone to Cashier %d too soon. They are going to the back of the line.\n",
								sizeof("Senator %d has gone to Cashier %d too soon. They are going to the back of the line.\n"),
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
					PrintfTwo("Customer %d has given Cashier %d $100.\n",
						sizeof("Customer %d has given Cashier %d $100.\n"),
						ssn,
						clerkID);
					break;
				case SENATOR:
					PrintfTwo("Senator %d has given Cashier %d $100.\n",
						sizeof("Senator %d has given Cashier %d $100.\n"),
						ssn,
						clerkID);
					break;
			}
			break;
		case Customer_GoingOutsideForSenator:
			PrintfOne("Customer %d is going outside the Passport Office because their is a Senator present.\n",
				sizeof("Customer %d is going outside the Passport Office because their is a Senator present.\n"),
				ssn);
			break;
		case Customer_LeavingPassportOffice:
			switch(customerType)
			{
				case CUSTOMER:
					PrintfOne("Customer %d is leaving the Passport Office.\n",
						sizeof("Customer %d is leaving the Passport Office.\n"),
						ssn);
					break;
				case SENATOR:
					PrintfOne("Senator %d is leaving the Passport Office.\n",
						sizeof("Senator %d is leaving the Passport Office.\n"),
						ssn);
					break;
			}
			break;
		}
	}
}

/* ========================================================================================================================================= */
/*																																			 */
/*		PPOffice Agents 																													 */
/*																																			 */
/* ========================================================================================================================================= */

/* Leave the Passport Office. Customers and Senators update the counter */
/*	of number of customers that have finished, which Clerks depend on   */
/*	in order to leave the PassportOffice. When all clerks have left the */
/*	Passport Office, the last clerk signals to the Manager so he can    */
/*	end the simulation. 												*/
/* Params: 	ssn - used for output 					 					*/
/*			type - used to decide which type of agent is finishing	 	*/
void Leave (int ssn, persontype type)
{
	switch (type)
	{
		case SENATOR:
			AcquireLock(senatorPresentLock);
			isSenatorPresent = false;
			Broadcast(senatorPresentCV, senatorPresentLock);
			ReleaseLock(senatorPresentLock);
		
		case CUSTOMER: /* && SENATOR */
			AcquireLock(customersFinishedLock);
			numCustomersFinished++;
			ReleaseLock(customersFinishedLock);
			WriteOutput(Customer_LeavingPassportOffice, type, type, ssn, ssn);
			break;
		
		case APPLICATION:
		case PICTURE:
		case PASSPORT:
		case CASHIER:
			AcquireLock(clerksFinishedLock);
			numClerksFinished++;
			if (numClerksFinished == (numAppClerks + numPicClerks + numPassportClerks + numCashiers))
			{
				Signal(allClerksFinishedCV, clerksFinishedLock);
			}
			ReleaseLock(clerksFinishedLock);
			break;
	}

	Exit(0);
}

/* ========================================================================================================================================= */
/*																																			 */
/*		CUSTOMER LINE DECISIONS																												 */
/*																																			 */
/* ========================================================================================================================================= */

/* -------------------------------------------------------------------- */
/* CheckIfSenatorPresent			
/* Customers need to check if the Senator is present before starting an */
/* 	interaction or getting in a line. If there is a clerk present, the  */
/*	Customer needs to "go outside" by waiting. 							*/
/* Params: 	ssn - used to check if thread is a Senator 					*/
/*			clerkID - used to get back in the same line (-1 if haven't  */
/*			  picked a line yet)										*/
/*			clerkType - used to decide which clerk's line to get into 	*/
/* -------------------------------------------------------------------- */
bool CheckIfSenatorPresent (int ssn, int clerkID, persontype clerkType)
{
	AcquireLock(senatorPresentLock);
	if (isSenatorPresent && people[ssn].type != SENATOR)
	{
		WriteOutput(Customer_GoingOutsideForSenator, clerkType, CUSTOMER, ssn, clerkID);
		Wait(senatorPresentCV, senatorPresentLock); /* Wait for Senator to finish. */
		ReleaseLock(senatorPresentLock); /* Lock gets reacquired inside of Wait, release it and continue. */	
		return true;
	}
	else
	{
		ReleaseLock(senatorPresentLock); /* No Senator present or I am a Senator, continue with business. */
		return false;
	}
}

/* Whenever Customer has enough money, they bribe. Customer first lets  */
/*	Clerk know she is trying to bribe, then does the interaction. 		*/
/* Params: 	ssn - used to update Customer's money 						*/
/*			clerkID - used to get locks and CVs for bribing 			*/
/*			clerkType - used to get locks and CVs for bribing			*/
void BribeClerk (int ssn, int clerkID, persontype clerkType)
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
	ReleaseLock(clerkLock);

	/* Reacquire lock so Customer can get in line. */
	AcquireLock(lineLock);
}

void WaitInLine (int ssn, int clerkID, persontype clerkType, linetype lineType)
{
	int lineLock;
	int lineCV;
	int bribeLineCV;

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
int DecideClerk (int ssn, persontype clerkType)
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

void DecideLine (int ssn, int clerkID, persontype clerkType)
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

				/* 	Make sure no senators have entered since joining line. */
				if (CheckIfSenatorPresent(ssn, clerkID, clerkType))
				{
					/* If Senator is present, "go outside" and get back in line when 	*/
					/*	allowed back in. 												*/
					DecideLine(ssn, clerkID, clerkType);
				}
			}
			else
			{ 
				/* No other options. Get in regular line. */
				WaitInLine(ssn, clerkID, clerkType, NORMALLINE);

				/* 	Make sure no senators have entered since joining line. */
				if (CheckIfSenatorPresent(ssn, clerkID, clerkType))
				{
					/* If Senator is present, "go outside" and get back in line when 	*/
					/*	allowed back in. 												*/
					DecideLine(ssn, clerkID, clerkType);
				}
			}
		}
	}

	ReleaseLock(lineLock); /* Acquired lock in DecideClerk; Release so next Customer can decide their line. */

	return;
}

/* ========================================================================================================================================= */
/*																																			 */
/*		CUSTOMER INTERACTIONS 																												 */
/*																																			 */
/* ========================================================================================================================================= */

void GetBackInLine (int ssn, persontype clerkType);

void MakePhotoDecision (int ssn, int clerkID, persontype clerkType)
{
	int clerkLock;
	int workCV;

	int amountLiked;

	clerkLock = clerkGroups[clerkType].clerkLocks[clerkID];
	workCV = clerkGroups[clerkType].workCVs[clerkID];

	/* Picture Clerk already took my picture, decide if I like it. */

	amountLiked = Random(1, 100);
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

void PunishTooSoon (int ssn, int clerkID, persontype clerkType)
{
	int clerkLock;
	int workCV;

	int punishmentTime;
	int i;

	clerkLock = clerkGroups[clerkType].clerkLocks[clerkID];
	workCV = clerkGroups[clerkType].workCVs[clerkID];

	/* Tell clerk you are done interacting. */
	Signal(workCV, clerkLock);
	ReleaseLock(clerkLock);

	punishmentTime = Random(100, 1000);

	for (i = 0; i < punishmentTime; i++)
	{
		Yield();
	}

	GetBackInLine(ssn, clerkType);
}

void CustomerInteraction (int ssn, int clerkID, persontype clerkType)
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

void GetBackInLine (int ssn, persontype clerkType)
{
	int clerkID;

	clerkID = DecideClerk(ssn, clerkType);
	DecideLine(ssn, clerkID, clerkType);
	CustomerInteraction(ssn, clerkID, clerkType);
}

void RunCustomer ()
{
	int ssn;
	int clerkID;
	persontype clerkType;

	int applicationFirst;
	persontype passportSequence[4] = { APPLICATION, PICTURE, PASSPORT, CASHIER };

	/* Nachos fork does not allow parameters to be passed in to new threads. 	*/
	AcquireLock(paramLock);
	ssn = threadParam;
	Signal(threadInitializedCV, paramLock);
	ReleaseLock(paramLock);

	/* Senator Checks. If a senator is present:									*/
	/* 	-	Don't even enter the PPOffice (both Customers and Senators) 		*/
	/*	-	If I am a senator, set that a senator is present. 					*/
	AcquireLock(senatorPresentLock);
	if (isSenatorPresent)
	{
		Wait(senatorPresentCV, senatorPresentLock);
	}
	else if (people[ssn].type == SENATOR)
	{
		isSenatorPresent = true;
	}
	ReleaseLock(senatorPresentLock);

	/* Customer (randomly) decides whether she wants to file application */
	/* 	or take picture first. */
	applicationFirst = Random(0, 100);
	if (applicationFirst < 50)
	{
		/* Go to PictureClerk first, instead. */
		passportSequence[0] = PICTURE;
		passportSequence[1] = APPLICATION;
	}

	for (clerkType = passportSequence[0]; clerkType <= CASHIER; clerkType++)
	{
		clerkID = DecideClerk(ssn, clerkType);
		DecideLine(ssn, clerkID, clerkType);
		CustomerInteraction(ssn, clerkID, clerkType);
	}

	Leave(ssn, CUSTOMER);
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
void AcceptBribe (int clerkID, persontype clerkType)
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

void TakeBreak (int clerkID, persontype clerkType) 
{
	int lineLock;
	int breakCV;

	lineLock = clerkGroups[clerkType].lineLock;
	breakCV = clerkGroups[clerkType].breakCVs[clerkID];

	WriteOutput(Clerk_GoingOnBreak, clerkType, clerkType, -1, clerkID);
	Wait(breakCV, lineLock); /* Waiting on breakCV = "going on break" */
	WriteOutput(Clerk_ComingOffBreak, clerkType, clerkType, -1, clerkID);
}

int CreateSystemJob (int ssn, int clerkID, persontype clerkType)
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

	/* Nachos fork does not allow parameters to be passed in to new threads. 	*/
	AcquireLock(paramLock);
	jobID = threadParam;
	Signal(threadInitializedCV, paramLock);
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

	filingTime = Random(20, 100);

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

void ClerkInteraction (int clerkID, persontype clerkType)
{
	int lineLock;
	int clerkLock;
	int workCV;

	int customerSSN;

	int jobID;

	lineLock = clerkGroups[clerkType].lineLock;
	clerkLock = clerkGroups[clerkType].clerkLocks[clerkID];
	workCV = clerkGroups[clerkType].workCVs[clerkID];

	AcquireLock(clerkLock);
	ReleaseLock(lineLock);
	Wait(workCV, clerkLock);

	customerSSN = clerkGroups[clerkType].clerks[clerkID].currentCustomer;

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
				Wait(threadInitializedCV, paramLock);
				ReleaseLock(paramLock);
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
			Wait(threadInitializedCV, paramLock);
			ReleaseLock(paramLock);
			break; /* Application Clerk already did work */
		case PICTURE:
			if (clerkGroups[clerkType].clerks[clerkID].customerLikedPhoto == true)
			{
				/* File Photo in the system */
				jobID = CreateSystemJob(customerSSN, clerkID, clerkType);
				AcquireLock(paramLock);
				threadParam = jobID;
				Fork("PictureFilingJob", sizeof("PictureFilingJob"), RunSystemJob);
				Wait(threadInitializedCV, paramLock);
				ReleaseLock(paramLock);
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

clerkinteraction DecideInteraction (int clerkID, persontype clerkType)
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

void RunClerk()
{
	int ssn;
	struct Person clerk;
	clerkinteraction interaction;

	AcquireLock(paramLock);
	ssn = threadParam;
	Signal(threadInitializedCV, paramLock);
	ReleaseLock(paramLock);

	clerk = people[ssn];

	do
	{
		/* Select next interaction based on: 										*/
		/* 	(1) if there are people trying to bribe > AcceptBribe 					*/
		/*	(2) if there are people waiting in any of my lines > ClerkInteraction 	*/
		/*	(3) if there are no people waiting in any of my lines > TakeBreak 		*/
		interaction = DecideInteraction (clerk.id, clerk.type);

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
	} while (numCustomersFinished < (numCustomers + numSenators));

	Exit(0);
}

/* ========================================================================================================================================= */
/*																																			 */
/*		MANAGER 																															 */
/*																																			 */
/* ========================================================================================================================================= */

int CollectMoney (persontype clerkType)
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

void TakeClerksOffBreak (persontype clerkType)
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

int ManageClerk (persontype clerkType)
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

void WakeUpAllClerks ()
{
	int lineLock;
	persontype clerkType;
	int clerkID;

	for (clerkType = APPLICATION; clerkType <= CASHIER; clerkType++)
	{
		lineLock = clerkGroups[clerkType].lineLock;
		AcquireLock(lineLock);
		Broadcast(clerkGroups[clerkType].breakCVs[clerkID], lineLock);
	}
}

void RunManager ()
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

	/* Wake all sleeping threads so they can Exit themselves. */
	WakeUpAllClerks();

	/* Once all clerks have left passport office, let simulation know it can terminate. */
	AcquireLock(clerksFinishedLock);
	Wait(allClerksFinishedCV, clerksFinishedLock);
	Signal(allAgentsFinishedCV, clerksFinishedLock);

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

	/* PassportOffice Simulation Data */
	paramLock = CreateLock("ParamLock", sizeof("ParamLock"));
	threadInitializedCV = CreateCV("threadInitializedCV", sizeof("threadInitializedCV"));

	customersFinishedLock = CreateLock("CustomersFinishedLock", sizeof("CustomersFinishedLock"));
	clerksFinishedLock = CreateLock("ClerksFinishedLock", sizeof("ClerksFinishedLock"));
	allClerksFinishedCV = CreateCV("allClerksFinishedCV", sizeof("allClerksFinishedCV"));
	allAgentsFinishedCV = CreateCV("AllAgentsFinishedCV", sizeof("AllAgentsFinishedCV"));
}

void ForkAgents ()
{
	int ssn;

	AcquireLock(paramLock);

	Fork("ManagerThread", sizeof("ManagerThread"), RunManager);

	for (ssn = (numCustomers + numSenators); ssn < (numCustomers + numSenators) + (numAppClerks + numPicClerks + numPassportClerks + numCashiers); ssn++)
	{
		/* TODO: Debugging, delete. */
		PrintfOne("Initializing Clerk with SSN %d\n", sizeof("Initializing Clerk with SSN %d\n"), ssn);
		threadParam = ssn;
		Fork("ClerkThread", sizeof("ClerkThread"), RunClerk);
		Wait(threadInitializedCV, paramLock);
	}

	for (ssn = 0; ssn < (numCustomers + numSenators); ssn++)
	{
		/* TODO: Debugging, delete. */
		PrintfOne("Initializing Customer with SSN %d\n", sizeof("Initializing Customer with SSN %d\n"), ssn);
		threadParam = ssn;
		Fork("CustomerThread", sizeof("CustomerThread"), RunCustomer);
		Wait(threadInitializedCV, paramLock);
	}

	ReleaseLock(paramLock);
}

void CleanUpData ()
{
	int clerkType;
	int clerkNum;
	int numClerks;

	DestroyLock(senatorPresentLock);
	DestroyCV(senatorPresentCV);

	DestroyLock(systemJobFindLock);
	DestroyLock(filingApplicationLock);
	DestroyLock(filingPictureLock);
	DestroyLock(certifyingPassportLock);

	for (clerkType = APPLICATION; clerkType <= CASHIER; clerkType++)
	{
		DestroyLock(clerkGroups[clerkType].lineLock);
		DestroyLock(clerkGroups[clerkType].moneyLock);

		numClerks = clerkGroups[clerkType].numClerks;

		for (clerkNum = 0; clerkNum < numClerks; clerkNum++)
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

	/* Wait for simulation to finish. = Wait for all Clerks/Customers to finish. */
	AcquireLock(numClerksFinished);
	Wait(allAgentsFinishedCV, numClerksFinished);
	Yield(); /* Safety precaution in case Manager was context switched after signalling, but before it could Exit. */

	/* Clean up the simulation. */ 
	CleanUpData();
	Exit(0);
}