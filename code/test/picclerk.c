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

void Clerk()
{

	int ssn;
	struct Person clerk;
	enum clerkinteraction interaction;
	AcquireLock(paramLock);

	ssn = threadParam;
	ReleaseLock(paramLock);

	clerk = people[ssn];

	while (numCustomersFinished < (numCustomers + numSenators))
	{
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
	}

	Exit(0);
}
int main()
{
	Write("Inside application clerk", sizeof("Inside application clerk"), 1);

	Exit(0);
}