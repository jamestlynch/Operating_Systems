#ifndef create_h
#define create_h

#include "syscall.h"
/*
	0=false
	1=true

	0=applicaiton
	1=picture
	2=passport
	3=cashier

customerData function: {turnedInApplication, acceptedPassport, gotPassport, applicationFiled, pictureFiled, passportCertified, passportRecorded;*/

int numCustomers;
int numCustomersFinished;

int customers = CreateMV(numCustomers);

for (ssn = 0; ssn < numCustomers; ssn++)
{
	int customerData = CreateMV(6, );
	SetMV(customers, ssn, customerData);
}

int moneyOptions[4] = {100, 600, 1100, 1600};

int turnedInApplication;
int acceptedPassport;
int gotPassport;
int applicationFiled;
int pictureFiled;
int passportCertified;
int passportRecorded;

/* MV */
int customerData;
int customers;
int people;

void InitializeCustomerData ()
{
	int ssn;

	for (ssn = 0; ssn < numCustomers; ssn++)
	{ 
		/* customers hold all shared data that clerks update */
		customerData = CreateMV("customerData", sizeof("customerData"), 6 );

		SetMV(customerData, turnedInApplication, 0);
		SetMV(customerData, acceptedPassport, 0);
		SetMV(customerData, gotPassport, 0);
		SetMV(customerData, applicationFiled, 0);
		SetMV(customerData, pictureFiled, 0);
		SetMV(customerData, passportCertified, 0);
		SetMV(customerData, passportRecorded, 0);

		SetMV(customers, ssn, customerData); //customers[ssn] = customerData;

		/* people holds all private data that people must update and make public to clerks. */
		people[ssn].id = ssn;
		people[ssn].money = InitialMoney();
		people[ssn].type = CUSTOMER;
	}
}

int clerkstate; /*{ 0=AVAILABLE, 1=BUSY, 2=ONBREAK };*/
int clerkinteraction; /*{ 0=DOINTERACTION, 1=TAKEBREAK, 2=ACCEPTBRIBE };*/

/******************************************/
/* 		  	    CLERK DATA  			  */
/******************************************/
	int state;
	int currentCustomer;
	int money;

	int lineLength;
	int bribeLineLength;
	int senatorLineLength;

	int numCustomersBribing;
	int customerLikedPhoto;
	int customerAppReadyToCertify;
	int customerAppReadyForPayment;

	int numAppClerks = 0;
	int numPicClerks = 0;
	int numPassportClerks = 0;
	int numCashiers = 0;

/******************************************/
/* 		  	   ClerkGroup Data 		      */
/******************************************/

	struct Clerk clerks[5];
	int numClerks[1];

	int moneyLock[1];
	int groupMoney[1];
	
	int lineLock[1];
	int lineCVs[5];
	int bribeLineCVs[5];
	int senatorLineCVs[5];

	int clerkLocks[5];
	int workCVs[5];
	int bribeCVs[5];
	int breakCVs[5];
};

/*struct ClerkGroup clerkGroups[4];*/


/* MV */
int clerkData;
int clerks;
int clerkGroups;

void InitializeClerkData (int numClerks, enum persontype clerkType)
{
	int clerkID;
	int ssn;
	int ssnOffset;

	clerks = CreateMV("clerks", sizeof("clerks"), numClerks);
	clerkGroups= CreateMV("clerkGroups", sizeof("clerkGroups"), 3);
	
	ssnOffset = (clerkType * 5) + numCustomers + numSenators;

	for (clerkID = 0; clerkID < numClerks; clerkID++)
	{
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
		SetMV(clerkData, customerAppReadyForPayment, 0); /*clerkData(customerAppReadyForPayment)=0*/
		
		SetMV(clerks, clerkID, clerkData); /*clerks(clerkID)=clerkData*/
		/* clerks(clerkID).clerkData(customerAppReadyForPayment)=0 */

		ssn = ssnOffset + clerkID;

		people[ssn].id = clerkID;
		people[ssn].money = 0;
		people[ssn].type = clerkType;
	}
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

	SetMV(lineCVs, 0, CreateCV("App:0-LineCV", 12));
	SetMV(clerkGroups, clerkType, lineCVs);
	SetMV(lineCVs, 1, CreateCV("App:1-LineCV", 12));
	SetMV(clerkGroups, clerkType, lineCVs);
	SetMV(lineCVs, 2, CreateCV("App:2-LineCV", 12));
	SetMV(clerkGroups, clerkType, lineCVs);
	SetMV(lineCVs, 3, CreateCV("App:3-LineCV", 12));
	SetMV(clerkGroups, clerkType, lineCVs);
	SetMV(lineCVs, 4, CreateCV("App:4-LineCV", 12));
	SetMV(clerkGroups, clerkType, lineCVs);

	SetMV(bribeLineCVs, 0, CreateCV("App:0-BribeLineCV", 17));
	SetMV(clerkGroups, clerkType, bribeLineCVs);
	SetMV(bribeLineCVs, 1, CreateCV("App:1-BribeLineCV", 17));
	SetMV(clerkGroups, clerkType, bribeLineCVs);
	SetMV(bribeLineCVs, 2, CreateCV("App:2-BribeLineCV", 17));
	SetMV(clerkGroups, clerkType, bribeLineCVs);
	SetMV(bribeLineCVs, 3, CreateCV("App:3-BribeLineCV", 17));
	SetMV(clerkGroups, clerkType, bribeLineCVs);
	SetMV(bribeLineCVs, 4, CreateCV("App:4-BribeLineCV", 17));
	SetMV(clerkGroups, clerkType, bribeLineCVs);

	SetMV(senatorLineCVs, 0, CreateCV("App:0-SenatorLineCV", 19));
	SetMV(clerkGroups, clerkType, senatorLineCVs);
	SetMV(senatorLineCVs, 1, CreateCV("App:1-SenatorLineCV", 19));
	SetMV(clerkGroups, clerkType, senatorLineCVs);
	SetMV(senatorLineCVs, 2, CreateCV("App:2-SenatorLineCV", 19));
	SetMV(clerkGroups, clerkType, senatorLineCVs);
	SetMV(senatorLineCVs, 3, CreateCV("App:3-SenatorLineCV", 19));
	SetMV(clerkGroups, clerkType, senatorLineCVs);
	SetMV(senatorLineCVs, 4, CreateCV("App:4-SenatorLineCV", 19));
	SetMV(clerkGroups, clerkType, senatorLineCVs);

	SetMV(clerkLocks, 0, CreateCV("App:0-ClerkLock", 15));
	SetMV(clerkGroups, clerkType, clerkLocks);
	SetMV(clerkLocks, 1, CreateCV("App:1-ClerkLock", 15));
	SetMV(clerkGroups, clerkType, clerkLocks);
	SetMV(clerkLocks, 2, CreateCV("App:2-ClerkLock", 15));
	SetMV(clerkGroups, clerkType, clerkLocks);
	SetMV(clerkLocks, 3, CreateCV("App:3-ClerkLock", 15));
	SetMV(clerkGroups, clerkType, clerkLocks);
	SetMV(clerkLocks, 4, CreateCV("App:4-ClerkLock", 15));
	SetMV(clerkGroups, clerkType, clerkLocks);

	SetMV(workCVs, 0, CreateCV("App:0-WorkCV", 12));
	SetMV(clerkGroups, clerkType, workCVs);
	SetMV(workCVs, 1, CreateCV("App:1-WorkCV", 12));
	SetMV(clerkGroups, clerkType, workCVs);
	SetMV(workCVs, 2, CreateCV("App:2-WorkCV", 12));
	SetMV(clerkGroups, clerkType, workCVs);
	SetMV(workCVs, 3, CreateCV("App:3-WorkCV", 12));
	SetMV(clerkGroups, clerkType, workCVs);
	SetMV(workCVs, 4, CreateCV("App:4-WorkCV", 12));
	SetMV(clerkGroups, clerkType, workCVs);

	SetMV(bribeCVs, 0, CreateCV("App:0-BribeCV", 13));
	SetMV(clerkGroups, clerkType, bribeCVs);
	SetMV(bribeCVs, 1, CreateCV("App:1-BribeCV", 13));
	SetMV(clerkGroups, clerkType, bribeCVs);
	SetMV(bribeCVs, 2, CreateCV("App:2-BribeCV", 13));
	SetMV(clerkGroups, clerkType, bribeCVs);
	SetMV(bribeCVs, 3, CreateCV("App:3-BribeCV", 13));
	SetMV(clerkGroups, clerkType, bribeCVs);
	SetMV(bribeCVs, 4, CreateCV("App:4-BribeCV", 13));
	SetMV(clerkGroups, clerkType, bribeCVs);

	SetMV(breakCVs, 0, CreateCV("App:0-BreakCV", 13));
	SetMV(clerkGroups, clerkType, breakCVs);
	SetMV(breakCVs, 1, CreateCV("App:1-BreakCV", 13));
	SetMV(clerkGroups, clerkType, breakCVs);
	SetMV(breakCVs, 2, CreateCV("App:2-BreakCV", 13));
	SetMV(clerkGroups, clerkType, breakCVs);
	SetMV(breakCVs, 3, CreateCV("App:3-BreakCV", 13));
	SetMV(clerkGroups, clerkType, breakCVs);
	SetMV(breakCVs, 4, CreateCV("App:4-BreakCV", 13));
	SetMV(clerkGroups, clerkType, breakCVs);

	/*	
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
	clerkGroups[clerkType].breakCVs[4] = CreateCV("App:4-BreakCV", 13);*/
}

void InitializePictureClerkData ()
{	enum persontype clerkType = PICTURE;

	InitializeClerkData(numPicClerks, clerkType);

	SetMV(clerkType, numClerks, numAppClerks);
	SetMV(clerkType, moneyLock, CreateLock("PictureClerks-MoneyLock", 23));
	SetMV(clerkType, groupMoney, 0);
	SetMV(clerkType, lineLock, CreateLock("PictureClerks-LineLock", 22));

	/*clerkGroups[clerkType][numClerks] = numAppClerks;
	clerkGroups[clerkType].numClerks = numAppClerks;
	clerkGroups[clerkType].moneyLock = CreateLock("PictureClerks-MoneyLock", 23);
	clerkGroups[clerkType].groupMoney = 0;
	clerkGroups[clerkType].lineLock = CreateLock("PictureClerks-LineLock", 22);*/

	SetMV(lineCVs, 0, CreateCV("Pic:0-LineCV", 12));
	SetMV(clerkGroups, clerkType, lineCVs);
	SetMV(lineCVs, 1, CreateCV("Pic:1-LineCV", 12));
	SetMV(clerkGroups, clerkType, lineCVs);
	SetMV(lineCVs, 2, CreateCV("Pic:2-LineCV", 12));
	SetMV(clerkGroups, clerkType, lineCVs);
	SetMV(lineCVs, 3, CreateCV("Pic:3-LineCV", 12));
	SetMV(clerkGroups, clerkType, lineCVs);
	SetMV(lineCVs, 4, CreateCV("Pic:4-LineCV", 12));
	SetMV(clerkGroups, clerkType, lineCVs);

	SetMV(bribeLineCVs, 0, CreateCV("Pic:0-BribeLineCV", 17));
	SetMV(clerkGroups, clerkType, bribeLineCVs);
	SetMV(bribeLineCVs, 1, CreateCV("Pic:1-BribeLineCV", 17));
	SetMV(clerkGroups, clerkType, bribeLineCVs);
	SetMV(bribeLineCVs, 2, CreateCV("Pic:2-BribeLineCV", 17));
	SetMV(clerkGroups, clerkType, bribeLineCVs);
	SetMV(bribeLineCVs, 3, CreateCV("Pic:3-BribeLineCV", 17));
	SetMV(clerkGroups, clerkType, bribeLineCVs);
	SetMV(bribeLineCVs, 4, CreateCV("Pic:4-BribeLineCV", 17));
	SetMV(clerkGroups, clerkType, bribeLineCVs);

	SetMV(senatorLineCVs, 0, CreateCV("Pic:0-SenatorLineCV", 19));
	SetMV(clerkGroups, clerkType, senatorLineCVs);
	SetMV(senatorLineCVs, 1, CreateCV("Pic:1-SenatorLineCV", 19));
	SetMV(clerkGroups, clerkType, senatorLineCVs);
	SetMV(senatorLineCVs, 2, CreateCV("Pic:2-SenatorLineCV", 19));
	SetMV(clerkGroups, clerkType, senatorLineCVs);
	SetMV(senatorLineCVs, 3, CreateCV("Pic:3-SenatorLineCV", 19));
	SetMV(clerkGroups, clerkType, senatorLineCVs);
	SetMV(senatorLineCVs, 4, CreateCV("Pic:4-SenatorLineCV", 19));
	SetMV(clerkGroups, clerkType, senatorLineCVs);

	SetMV(clerkLocks, 0, CreateCV("Pic:0-ClerkLock", 15));
	SetMV(clerkGroups, clerkType, clerkLocks);
	SetMV(clerkLocks, 1, CreateCV("Pic:1-ClerkLock", 15));
	SetMV(clerkGroups, clerkType, clerkLocks);
	SetMV(clerkLocks, 2, CreateCV("Pic:2-ClerkLock", 15));
	SetMV(clerkGroups, clerkType, clerkLocks);
	SetMV(clerkLocks, 3, CreateCV("Pic:3-ClerkLock", 15));
	SetMV(clerkGroups, clerkType, clerkLocks);
	SetMV(clerkLocks, 4, CreateCV("Pic:4-ClerkLock", 15));
	SetMV(clerkGroups, clerkType, clerkLocks);

	SetMV(workCVs, 0, CreateCV("Pic:0-WorkCV", 12));
	SetMV(clerkGroups, clerkType, workCVs);
	SetMV(workCVs, 1, CreateCV("Pic:1-WorkCV", 12));
	SetMV(clerkGroups, clerkType, workCVs);
	SetMV(workCVs, 2, CreateCV("Pic:2-WorkCV", 12));
	SetMV(clerkGroups, clerkType, workCVs);
	SetMV(workCVs, 3, CreateCV("Pic:3-WorkCV", 12));
	SetMV(clerkGroups, clerkType, workCVs);
	SetMV(workCVs, 4, CreateCV("Pic:4-WorkCV", 12));
	SetMV(clerkGroups, clerkType, workCVs);

	SetMV(bribeCVs, 0, CreateCV("Pic:0-BribeCV", 13));
	SetMV(clerkGroups, clerkType, bribeCVs);
	SetMV(bribeCVs, 1, CreateCV("Pic:1-BribeCV", 13));
	SetMV(clerkGroups, clerkType, bribeCVs);
	SetMV(bribeCVs, 2, CreateCV("Pic:2-BribeCV", 13));
	SetMV(clerkGroups, clerkType, bribeCVs);
	SetMV(bribeCVs, 3, CreateCV("Pic:3-BribeCV", 13));
	SetMV(clerkGroups, clerkType, bribeCVs);
	SetMV(bribeCVs, 4, CreateCV("Pic:4-BribeCV", 13));
	SetMV(clerkGroups, clerkType, bribeCVs);

	SetMV(breakCVs, 0, CreateCV("Pic:0-BreakCV", 13));
	SetMV(clerkGroups, clerkType, breakCVs);
	SetMV(breakCVs, 1, CreateCV("Pic:1-BreakCV", 13));
	SetMV(clerkGroups, clerkType, breakCVs);
	SetMV(breakCVs, 2, CreateCV("Pic:2-BreakCV", 13));
	SetMV(clerkGroups, clerkType, breakCVs);
	SetMV(breakCVs, 3, CreateCV("Pic:3-BreakCV", 13));
	SetMV(clerkGroups, clerkType, breakCVs);
	SetMV(breakCVs, 4, CreateCV("Pic:4-BreakCV", 13));
	SetMV(clerkGroups, clerkType, breakCVs);



/*
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
	clerkGroups[clerkType].breakCVs[4] = CreateCV("Pic:4-BreakCV", 13);*/
}

/******************************************/
/* 			 System Job Data 			  */
/******************************************/


int ssn;
int clerkID;
enum persontype type;

int numSystemJobs = 50;
int jobs;

jobs = CreateMV("jobs", sizeof("jobs"), numSystemJobs);

int systemJobFindLock; /* Used to synchronize search for available system jobs */
int filingPictureLock;
int filingApplicationLock;
int certifyingPassportLock;

void InitializeSystemJobs ()
{
	int jobID;

	for (jobID = 0; jobID < numSystemJobs; jobID++)
	{
		SetMV(jobData, ssn, -1);
		SetMV(jobData, clerkID, -1);
		SetMV(jobData, type, CUSTOMER);

		SetMV(jobs, jobID, jobData);
		/*jobs[jobID].ssn = -1;
		jobs[jobID].clerkID = -1;
		jobs[jobID].type = CUSTOMER;*/
	}

	systemJobFindLock = CreateLock("SystemJobFindLock", 17);
	filingApplicationLock = CreateLock("FilingApplicationLock", 21);
	filingPictureLock = CreateLock("FilingPictureLock", 17);
	certifyingPassportLock = CreateLock("CertifyingPassportLock", 22);
}