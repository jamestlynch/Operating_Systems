/*	passportoffice.c
 */

/* ========================================================================================================================================= */
/*																																			 */
/*		DATA SETUP																															 */
/*																																			 */
/* ========================================================================================================================================= */

/******************************************/
/* 		  Customer / Senator Data 		  */
/******************************************/

int moneyOptions[4] = {100, 600, 1100, 1600};

int index_turnedInApplication = 0;
int index_acceptedPassport = 1;
int index_gotPassport = 2;
int index_applicationFiled = 3;
int index_photoFiled = 4;
int index_passportCertified = 5;
int index_passportRecorded = 6;

int numCustomerDataEntries = 7;

int customerData[7] = {
	0, /* turnedInApplication */
	0, /* acceptedPassport */
	0, /* gotPassport */
	0, /* applicationFiled */
	0, /* photoFiled */
	0, /* passportCertified */
	0, /* passportRecorded */
};

int numCustomers = 50;
int customers[50][7];

int numSenators = 10;
int senators[10][7];

void InitializeCustomerData ()
{
	int ssn, i;

	for (ssn = 0; ssn < numCustomers; ssn++)
	{
		for (i = 0; i < numCustomerDataEntries; i++)
		{
			customers[ssn][i] = customerData[i];
		}
	}
}

void InitializeSenatorData ()
{
	int ssn, i;

	for (ssn = numCustomers; ssn < (numCustomers + numSenators); ssn++)
	{
		for (i = 0; i < numCustomerDataEntries; i++)
		{
			senators[ssn][i] = customerData[i];
		}
	}
}

/******************************************/
/* 			    Clerk Data 				  */
/******************************************/

int state_clerkAvailable = 0;
int state_clerkBusy = 1;
int state_clerkOnBreak = 2;

int index_state = 0;
int index_currentCustomer = 1;
int index_money = 2;
int index_lineCount = 3;
int index_bribeLineCount = 4;
int index_senatorLineCount = 5;
int index_isBeingBribed = 6;
int index_customerLikedPhoto = 7;
int index_customerAppReadyToCertify = 8;
int index_customerAppReadyForPayment = 9;

int numClerkDataEntries = 10;

int clerkData[] = {
	0, /* state */
	-1, /* currentCustomer */
	0, /* money */
	0, /* lineCount */
	0, /* bribeLineCount */
	0, /* senatorLineCount */
	0, /* isBeingBribed */
	0, /* customerLikedPhoto */
	0, /* customerAppReadyToCertify */
	0 /* customerAppReadyForPayment */
};

int clerkType_appclerk = 0;
int clerkType_picclerk = 1;
int clerkType_passportclerk = 2;
int clerkType_cashier = 3;

int clerkCounts [4] = {
	5, /* clerkType_appClerks */
	5, /* clerkType_pictureClerks */
	5, /* clerkType_passportClerks */
	5  /* clerkType_cashiers */
};

int appclerks[5][10];
int appclerkLocks[5];
int appclerkLineCVs[5];
int appclerkBribeLineCVs[5];
int appclerkSenatorLineCVs[5];
int appclerkWorkCVs[5];
int appclerkBreakCVs[5];

int picclerks[5][10];
int picclerkLocks[5];
int picclerkLineCVs[5];
int picclerkBribeLineCVs[5];
int picclerkSenatorLineCVs[5];
int picclerkWorkCVs[5];
int picclerkBreakCVs[5];

int passportclerks[5][10];
int passportclerkLocks[5];
int passportclerkLineCVs[5];
int passportclerkBribeLineCVs[5];
int passportclerkSenatorLineCVs[5];
int passportclerkWorkCVs[5];
int passportclerkBreakCVs[5];

int cashiers[5][10];
int cashierLocks[5];
int cashierLineCVs[5];
int cashierBribeLineCVs[5];
int cashierSenatorLineCVs[5];
int cashierWorkCVs[5];
int cashierBreakCVs[5];

int index_clerksData = 0;
int index_clerksCount = 1;
int index_clerksMoney = 2;
int index_lineLock = 3;
int index_moneyLock = 4;
int index_clerkLocks = 5;
int index_lineCVs = 6;
int index_bribeLineCVs = 7;
int index_senatorLineCVs = 8;
int index_clerkWorkCVs = 9;
int index_breakCVs = 10;

int numClerkGroupDataEntries = 11;

int clerkGroupData[4][11];

void InitializeClerkData (int clerks[][10], int numClerks)
{
	int clerkType, clerkID, i;

	for (clerkID = 0; clerkID < numClerks; clerkID++)
	{
		for (i = 0; i < numClerkDataEntries; i++)
		{
			clerks[clerkID][i] = clerkData[i];
		}
	}
}

void InitializeApplicationClerkData ()
{
	int clerkType = clerkType_appclerk;
	int clerkCount = clerkCounts[clerkType];

	InitializeClerkData(appclerks, clerkCount);

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

	appclerkBreakCVs[0] = CreateCV("App:0-BreakCV", 13);
	appclerkBreakCVs[1] = CreateCV("App:1-BreakCV", 13);
	appclerkBreakCVs[2] = CreateCV("App:2-BreakCV", 13);
	appclerkBreakCVs[3] = CreateCV("App:3-BreakCV", 13);
	appclerkBreakCVs[4] = CreateCV("App:4-BreakCV", 13);

	clerkGroupData[clerkType][index_clerksData] = (unsigned int)appclerks;
	clerkGroupData[clerkType][index_clerksCount] = (unsigned int)clerkCount;
	clerkGroupData[clerkType][index_clerksMoney] = 0;
	clerkGroupData[clerkType][index_lineLock] = CreateLock("ApplicationClerks-LineLock", 26);
	clerkGroupData[clerkType][index_moneyLock] = CreateLock("ApplicationClerks-MoneyLock", 27);
	clerkGroupData[clerkType][index_clerkLocks] = (unsigned int)appclerkLocks;
	clerkGroupData[clerkType][index_lineCVs] = (unsigned int)appclerkLineCVs;
	clerkGroupData[clerkType][index_bribeLineCVs] = (unsigned int)appclerkBribeLineCVs;
	clerkGroupData[clerkType][index_senatorLineCVs] = (unsigned int)appclerkSenatorLineCVs;
	clerkGroupData[clerkType][index_clerkWorkCVs] = (unsigned int)appclerkWorkCVs;
	clerkGroupData[clerkType][index_breakCVs] = (unsigned int)appclerkBreakCVs;
}

void InitializePictureClerkData ()
{
	int clerkType = clerkType_picclerk;
	int clerkCount = clerkCounts[clerkType];

	InitializeClerkData(picclerks, clerkCount);

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

	picclerkBreakCVs[0] = CreateCV("Pic:0-BreakCV", 13);
	picclerkBreakCVs[1] = CreateCV("Pic:1-BreakCV", 13);
	picclerkBreakCVs[2] = CreateCV("Pic:2-BreakCV", 13);
	picclerkBreakCVs[3] = CreateCV("Pic:3-BreakCV", 13);
	picclerkBreakCVs[4] = CreateCV("Pic:4-BreakCV", 13);

	clerkGroupData[clerkType][index_clerksData] = (unsigned int)picclerks;
	clerkGroupData[clerkType][index_clerksCount] = (unsigned int)clerkCount;
	clerkGroupData[clerkType][index_clerksMoney] = 0;
	clerkGroupData[clerkType][index_lineLock] = CreateLock("PictureClerks-LineLock", 22);
	clerkGroupData[clerkType][index_moneyLock] = CreateLock("PictureClerks-MoneyLock", 23);
	clerkGroupData[clerkType][index_clerkLocks] = (unsigned int)picclerkLocks;
	clerkGroupData[clerkType][index_lineCVs] = (unsigned int)picclerkLineCVs;
	clerkGroupData[clerkType][index_bribeLineCVs] = (unsigned int)picclerkBribeLineCVs;
	clerkGroupData[clerkType][index_senatorLineCVs] = (unsigned int)picclerkSenatorLineCVs;
	clerkGroupData[clerkType][index_clerkWorkCVs] = (unsigned int)picclerkWorkCVs;
	clerkGroupData[clerkType][index_breakCVs] = (unsigned int)picclerkBreakCVs;
}

void InitializePassportClerkData ()
{
	int clerkType = clerkType_passportclerk;
	int clerkCount = clerkCounts[clerkType];

	InitializeClerkData(passportclerks, clerkCount);

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

	passportclerkBreakCVs[0] = CreateCV("Pas:0-BreakCV", 13);
	passportclerkBreakCVs[1] = CreateCV("Pas:1-BreakCV", 13);
	passportclerkBreakCVs[2] = CreateCV("Pas:2-BreakCV", 13);
	passportclerkBreakCVs[3] = CreateCV("Pas:3-BreakCV", 13);
	passportclerkBreakCVs[4] = CreateCV("Pas:4-BreakCV", 13);

	clerkGroupData[clerkType][index_clerksData] = (unsigned int)passportclerks;
	clerkGroupData[clerkType][index_clerksCount] = (unsigned int)clerkCount;
	clerkGroupData[clerkType][index_clerksMoney] = 0;
	clerkGroupData[clerkType][index_lineLock] = CreateLock("PassportClerks-LineLock", 23);
	clerkGroupData[clerkType][index_moneyLock] = CreateLock("PassportClerks-MoneyLock", 24);
	clerkGroupData[clerkType][index_clerkLocks] = (unsigned int)passportclerkLocks;
	clerkGroupData[clerkType][index_lineCVs] = (unsigned int)passportclerkLineCVs;
	clerkGroupData[clerkType][index_bribeLineCVs] = (unsigned int)passportclerkBribeLineCVs;
	clerkGroupData[clerkType][index_senatorLineCVs] = (unsigned int)passportclerkSenatorLineCVs;
	clerkGroupData[clerkType][index_clerkWorkCVs] = (unsigned int)passportclerkWorkCVs;
	clerkGroupData[clerkType][index_breakCVs] = (unsigned int)passportclerkBreakCVs;
}

void InitializeCashierData ()
{
	int clerkType = clerkType_cashier;
	int clerkCount = clerkCounts[clerkType];

	InitializeClerkData(cashiers, clerkCount);

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

	cashierBreakCVs[0] = CreateCV("Csh:0-BreakCV", 13);
	cashierBreakCVs[1] = CreateCV("Csh:1-BreakCV", 13);
	cashierBreakCVs[2] = CreateCV("Csh:2-BreakCV", 13);
	cashierBreakCVs[3] = CreateCV("Csh:3-BreakCV", 13);
	cashierBreakCVs[4] = CreateCV("Csh:4-BreakCV", 13);

	clerkGroupData[clerkType][index_clerksData] = (unsigned int)cashiers;
	clerkGroupData[clerkType][index_clerksCount] = (unsigned int)clerkCount;
	clerkGroupData[clerkType][index_clerksMoney] = 0;
	clerkGroupData[clerkType][index_lineLock] = CreateLock("Cashiers-LineLock", 17);
	clerkGroupData[clerkType][index_moneyLock] = CreateLock("Cashiers-MoneyLock", 18);
	clerkGroupData[clerkType][index_clerkLocks] = (unsigned int)cashierLocks;
	clerkGroupData[clerkType][index_lineCVs] = (unsigned int)cashierLineCVs;
	clerkGroupData[clerkType][index_bribeLineCVs] = (unsigned int)cashierBribeLineCVs;
	clerkGroupData[clerkType][index_senatorLineCVs] = (unsigned int)cashierSenatorLineCVs;
	clerkGroupData[clerkType][index_clerkWorkCVs] = (unsigned int)cashierWorkCVs;
	clerkGroupData[clerkType][index_breakCVs] = (unsigned int)cashierBreakCVs;
}

void InitializeData ()
{
	InitializeCustomerData();
	InitializeSenatorData();
	InitializeApplicationClerkData();
	InitializePictureClerkData();
	InitializePassportClerkData();
	InitializeCashierData();
}

/* ========================================================================================================================================= */
/*																																			 */
/*		DATA SETUP																															 */
/*																																			 */
/* ========================================================================================================================================= */

int main() 
{
	InitializeData();
}