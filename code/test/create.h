#include "syscall.h"
/*
	0=false
	1=true

	0=applicaiton
	1=picture
	2=passport
	3=cashier

customerData function: {turnedInApplication, acceptedPassport, gotPassport, applicationFiled, pictureFiled, passportCertified, passportRecorded;*/

int numCustomers=5;
int numCustomersFinished=0;
int numSenators=0;
int numSenatorsFinished=0;

int customers = CreateMV("numCustomers", sizeof("numCustomers"), numCustomers);
int senators= CreateMV("numSenators", sizeof("numSenators"), numSenators);

int moneyOptions[4] = {100, 600, 1100, 1600};

int turnedInApplication;
int acceptedPassport;
int gotPassport;
int applicationFiled;
int pictureFiled;
int passportCertified;
int passportRecorded;

enum persontype { APPLICATION, PICTURE, PASSPORT, CASHIER, CUSTOMER, SENATOR, MANAGER };

/* MV */
int customerData;
int customers;
int peopleData;
int people;

int id;
int money;
enum persontype type;


/* Senators are stored inside of customers array. See above. */

int isSenatorPresent = 0; /* Used to determine whether or not customers should wait for senators to leave PPOffice */

int senatorIndoorLock; /* Synchronizes isSenatorPresent */
int senatorIndoorCV; /* Wait on this whenever a senator is inside PPOffice */


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

		SetMV(customers, ssn, customerData); /*customers[ssn] = customerData;*/

		peopleData = CreateMV("peopleData", sizeof("peopleData"), 2);

		SetMV(peopleData, id, ssn);
		SetMV(peopleData, money, 100);
		SetMV(peopleData, type, CUSTOMER);
		SetMV(people, ssn, peopleData);
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

	int numClerks=5;

	int moneyLock[1];
	int groupMoney[1];
	
	int lineLock;
	int lineCVs[5];
	int bribeLineCVs[5];
	int senatorLineCVs[5];

	int clerkLocks[5];
	int workCVs[5];
	int bribeCVs[5];
	int breakCVs[5];


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
	int clerkID;
	int ssn;
	int ssnOffset;

	clerks = CreateMV("clerks", sizeof("clerks"), numClerks);
	clerkGroups= CreateMV("clerkGroups", sizeof("clerkGroups"), 4);

	
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


		peopleData = CreateMV("peopleData", sizeof("peopleData"), 2);

		SetMV(peopleData, id, clerkID);
		SetMV(peopleData, money, 0);
		SetMV(peopleData, type, clerkType);

		SetMV(people, ssn, peopleData);
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

	/*clerkGroups[clerkType].clerks[i].state
	int appclerkdata = GetMV(clerkGroups, clerkType);
	int appclerks = GetMV(appclerkdata, clerks);
	int clerk = GetMV(clerks, i);
	SetMV(clerk, state, BUSY);

	clerkGroups[clerkType].clerks[clerkID].senatorLineLength;
	*/


	 appclerkdata = GetMV(clerkGroups, clerkType);
	 line= GetMV(appclerkdata, lineCVs);
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


	/*clerkGroups[clerkType].lineCVs[0]=createCV("App:0-LineCV", 12)*/

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
{	
	enum persontype clerkType = PICTURE;

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
}
void InitiializePassportData(){
	enum persontype clerkType = PASSPORT;

	InitializeClerkData(numPicClerks, clerkType);

	SetMV(clerkType, numClerks, numAppClerks);
	SetMV(clerkType, moneyLock, CreateLock("PassportClerks-MoneyLock", 24));
	SetMV(clerkType, groupMoney, 0);
	SetMV(clerkType, lineLock, CreateLock("PassportClerks-LineLock", 23));

}
void InitializeCashierData(){
	enum persontype clerkType = CASHIER;

	InitializeClerkData(numCashiers, clerkType);

	SetMV(clerkType, numClerks, numAppClerks);
	SetMV(clerkType, moneyLock, CreateLock("Cashiers-MoneyLock", 18));
	SetMV(clerkType, groupMoney, 0);
	SetMV(clerkType, lineLock, CreateLock("Cashiers-LineLock", 17));



}


/******************************************/
/* 			 System Job Data 			  */
/******************************************/


int ssn;
int clerkID;
enum persontype type;

int numSystemJobs = 50;
int jobs = CreateMV("jobs", sizeof("jobs"), numSystemJobs);
int jobData= CreateMV("jobData", sizeof("jobData"), 2);

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