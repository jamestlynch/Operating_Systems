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

int clerkType_appClerks = 0;
int clerkType_pictureClerks = 1;
int clerkType_passportClerks = 2;
int clerkType_cashiers = 3;

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

void InitializeLocks (int locks[5], int numLocks, char lockNames[5], int lockNameLength)
{
	int i;

	for (i = 0; i < numLocks; i++)
	{
		/*locks[i] = CreateLock(lockNames[i], lockNameLength);*/
		WriteInt(i);
		Write(lockNames[i], lockNameLength, 1);
	}
}

void InitializeCVs (int conditions[5], int numCVs, char cvNames[5], int cvNameLength)
{
	int i;

	for (i = 0; i < numCVs; i++)
	{
		/*conditions[i] = CreateCV(cvNames[i], cvNameLength);*/
	}
}

void InitializeApplicationClerkData ()
{
	int clerkType = clerkType_appClerks;
	int clerkCount = clerkCounts[clerkType];
	char *synchStructNames[5];
	int synchStructNameLength;

	InitializeClerkData(appclerks, clerkCount);

	synchStructNames[0] = "App:0-ClerkLock";
	synchStructNames[1] = "App:1-ClerkLock";
	synchStructNames[2] = "App:2-ClerkLock";
	synchStructNames[3] = "App:3-ClerkLock";
	synchStructNames[4] = "App:4-ClerkLock";
	synchStructNameLength = 15;
	InitializeLocks(appclerkLocks, clerkCount, *synchStructNames, synchStructNameLength);

	synchStructNames[0] = "App:0-LineCV";
	synchStructNames[1] = "App:1-LineCV";
	synchStructNames[2] = "App:2-LineCV";
	synchStructNames[3] = "App:3-LineCV";
	synchStructNames[4] = "App:4-LineCV";
	synchStructNameLength = 12;
	InitializeCVs(appclerkLineCVs, clerkCount, *synchStructNames, synchStructNameLength);

	synchStructNames[0] = "App:0-BribeLineCV";
	synchStructNames[1] = "App:1-BribeLineCV";
	synchStructNames[2] = "App:2-BribeLineCV";
	synchStructNames[3] = "App:3-BribeLineCV";
	synchStructNames[4] = "App:4-BribeLineCV";
	synchStructNameLength = 17;
	InitializeCVs(appclerkBribeLineCVs, clerkCount, *synchStructNames, synchStructNameLength);

	synchStructNames[0] = "App:0-SenatorLineCV";
	synchStructNames[1] = "App:1-SenatorLineCV";
	synchStructNames[2] = "App:2-SenatorLineCV";
	synchStructNames[3] = "App:3-SenatorLineCV";
	synchStructNames[4] = "App:4-SenatorLineCV";
	synchStructNameLength = 19;
	InitializeCVs(appclerkSenatorLineCVs, clerkCount, *synchStructNames, synchStructNameLength);

	synchStructNames[0] = "App:0-WorkCV";
	synchStructNames[1] = "App:1-WorkCV";
	synchStructNames[2] = "App:2-WorkCV";
	synchStructNames[3] = "App:3-WorkCV";
	synchStructNames[4] = "App:4-WorkCV";
	synchStructNameLength = 12;
	InitializeCVs(appclerkWorkCVs, clerkCount, *synchStructNames, synchStructNameLength);

	synchStructNames[0] = "App:0-BreakCV";
	synchStructNames[1] = "App:1-BreakCV";
	synchStructNames[2] = "App:2-BreakCV";
	synchStructNames[3] = "App:3-BreakCV";
	synchStructNames[4] = "App:4-BreakCV";
	synchStructNameLength = 13;
	InitializeCVs(appclerkBreakCVs, clerkCount, *synchStructNames, synchStructNameLength);

	clerkGroupData[clerkType][index_clerksData] = (unsigned int)appclerks;
	clerkGroupData[clerkType][index_clerksCount] = (unsigned int)clerkCount;
	clerkGroupData[clerkType][index_clerksMoney] = 0;
	clerkGroupData[clerkType][index_lineLock] = CreateLock("AppClerks-LineLock", 18);
	clerkGroupData[clerkType][index_moneyLock] = CreateLock("AppClerks-MoneyLock", 19);
	clerkGroupData[clerkType][index_clerkLocks] = (unsigned int)appclerkLocks;
	clerkGroupData[clerkType][index_lineCVs] = (unsigned int)appclerkLineCVs;
	clerkGroupData[clerkType][index_bribeLineCVs] = (unsigned int)appclerkBribeLineCVs;
	clerkGroupData[clerkType][index_senatorLineCVs] = (unsigned int)appclerkSenatorLineCVs;
	clerkGroupData[clerkType][index_clerkWorkCVs] = (unsigned int)appclerkWorkCVs;
	clerkGroupData[clerkType][index_breakCVs] = (unsigned int)appclerkBreakCVs;
}

/*void InitializePictureClerkData ()
{
	int picclerkGroupData = clerkGroupData[clerkType_pictureClerks];
	int clerkCount = clerkCounts[clerkType_pictureClerks];

	char synchStructNames[5][20];
	int synchStructNameLengths[5];

	InitializeClerkData(picclerks, clerkCount);

	synchStructNames[0] = "Pic:0-ClerkLock";
	synchStructNames[1] = "Pic:1-ClerkLock";
	synchStructNames[2] = "Pic:2-ClerkLock";
	synchStructNames[3] = "Pic:3-ClerkLock";
	synchStructNames[4] = "Pic:4-ClerkLock";
	synchStructNameLengths = { 15, 15, 15, 15, 15 };
	InitializeLocks(picclerkLocks, clerkCount, synchStructNames, synchStructNameLengths);

	synchStructNames = { "Pic:0-LineCV", "Pic:1-LineCV", "Pic:2-LineCV", "Pic:3-LineCV", "Pic:4-LineCV" };
	synchStructNameLengths = { 12, 12, 12, 12, 12 };
	InitializeCVs(picclerkLineCVs, clerkCount, synchStructNames, synchStructNameLengths);

	synchStructNames = { "Pic:0-BribeLineCV", "Pic:1-BribeLineCV", "Pic:2-BribeLineCV", "Pic:3-BribeLineCV", "Pic:4-BribeLineCV" };
	synchStructNameLengths = { 17, 17, 17, 17, 17 };
	InitializeCVs(picclerkBribeLineCVs, clerkCount);

	synchStructNames = { "Pic:0-SenatorLineCV", "Pic:1-SenatorLineCV", "Pic:2-SenatorLineCV", "Pic:3-SenatorLineCV", "Pic:4-SenatorLineCV" };
	synchStructNameLengths = { 19, 19, 19, 19, 19 };
	InitializeCVs(picclerkSenatorLineCVs, clerkCount, synchStructNames, synchStructNameLengths);

	synchStructNames = { "Pic:0-WorkCV", "Pic:1-WorkCV", "Pic:2-WorkCV", "Pic:3-WorkCV", "Pic:4-WorkCV" };
	synchStructNameLengths = { 12, 12, 12, 12, 12 };
	InitializeCVs(picclerkWorkCVs, clerkCount, synchStructNames, synchStructNameLengths);

	synchStructNames = { "Pic:0-BreakCV", "Pic:1-BreakCV", "Pic:2-BreakCV", "Pic:3-BreakCV", "Pic:4-BreakCV" };
	synchStructNameLengths = { 13, 13, 13, 13, 13 };
	InitializeCVs(picclerkBreakCVs, clerkCount, synchStructNames, synchStructNameLengths);

	picclerkGroupData[index_clerksData] = picclerks;
	picclerkGroupData[index_clerksCount] = clerkCount;
	picclerkGroupData[index_clerksMoney] = 0;
	picclerkGroupData[index_lineLock] = CreateLock("PicClerks-LineLock", 18);
	picclerkGroupData[index_moneyLock] = CreateLock("PicClerks-MoneyLock", 19);
	picclerkGroupData[index_clerkLocks] = picclerksLocks;
	picclerkGroupData[index_lineCVs] = picclerksLineCVs;
	picclerkGroupData[index_bribeLineCVs] = picclerksBribeLineCVs;
	picclerkGroupData[index_senatorLineCVs] = picclerksSenatorLineCVs;
	picclerkGroupData[index_clerkWorkCVs] = picclerksWorkCVs;
	picclerkGroupData[index_breakCVs] = picclerksBreakCVs;
}

void InitializePassportClerkData ()
{
	int passportclerkGroupData = clerkGroupData[clerkType_passportClerks];
	int clerkCount = clerkCounts[clerkType_passportClerks];

	char synchStructNames[5][20];
	int synchStructNameLengths[5];

	InitializeClerkData(passportclerks, clerkCount);

	synchStructNames = { "Pas:0-ClerkLock", "Pas:1-ClerkLock", "Pas:2-ClerkLock", "Pas:3-ClerkLock", "Pas:4-ClerkLock" };
	synchStructNameLengths = { 15, 15, 15, 15, 15 };
	InitializeLocks(passportclerkLocks, clerkCount, synchStructNames, synchStructNameLengths);

	synchStructNames = { "Pas:0-LineCV", "Pas:1-LineCV", "Pas:2-LineCV", "Pas:3-LineCV", "Pas:4-LineCV" };
	synchStructNameLengths = { 12, 12, 12, 12, 12 };
	InitializeCVs(passportclerkLineCVs, clerkCount, synchStructNames, synchStructNameLengths);

	synchStructNames = { "Pas:0-BribeLineCV", "Pas:1-BribeLineCV", "Pas:2-BribeLineCV", "Pas:3-BribeLineCV", "Pas:4-BribeLineCV" };
	synchStructNameLengths = { 17, 17, 17, 17, 17 };
	InitializeCVs(passportclerkBribeLineCVs, clerkCount);

	synchStructNames = { "Pas:0-SenatorLineCV", "Pas:1-SenatorLineCV", "Pas:2-SenatorLineCV", "Pas:3-SenatorLineCV", "Pas:4-SenatorLineCV" };
	synchStructNameLengths = { 19, 19, 19, 19, 19 };
	InitializeCVs(passportclerkSenatorLineCVs, clerkCount, synchStructNames, synchStructNameLengths);

	synchStructNames = { "Pas:0-WorkCV", "Pas:1-WorkCV", "Pas:2-WorkCV", "Pas:3-WorkCV", "Pas:4-WorkCV" };
	synchStructNameLengths = { 12, 12, 12, 12, 12 };
	InitializeCVs(passportclerkWorkCVs, clerkCount, synchStructNames, synchStructNameLengths);

	synchStructNames = { "Pas:0-BreakCV", "Pas:1-BreakCV", "Pas:2-BreakCV", "Pas:3-BreakCV", "Pas:4-BreakCV" };
	synchStructNameLengths = { 13, 13, 13, 13, 13 };
	InitializeCVs(passportclerkBreakCVs, clerkCount, synchStructNames, synchStructNameLengths);

	passportclerkGroupData[index_clerksData] = passportclerks;
	passportclerkGroupData[index_clerksCount] = clerkCount;
	passportclerkGroupData[index_clerksMoney] = 0;
	passportclerkGroupData[index_lineLock] = CreateLock("PasClerks-LineLock", 18);
	passportclerkGroupData[index_moneyLock] = CreateLock("PasClerks-MoneyLock", 19);
	passportclerkGroupData[index_clerkLocks] = passportclerksLocks;
	passportclerkGroupData[index_lineCVs] = passportclerksLineCVs;
	passportclerkGroupData[index_bribeLineCVs] = passportclerksBribeLineCVs;
	passportclerkGroupData[index_senatorLineCVs] = passportclerksSenatorLineCVs;
	passportclerkGroupData[index_clerkWorkCVs] = passportclerksWorkCVs;
	passportclerkGroupData[index_breakCVs] = passportclerksBreakCVs;
}

void InitializeCashierData ()
{
	int cashierGroupData = clerkGroupData[clerkType_cashiers];
	int clerkCount = clerkCounts[clerkType_cashiers];

	char synchStructNames[5][20];
	int synchStructNameLengths[5];

	InitializeClerkData(cashiers, clerkCount);

	synchStructNames = { "Csh:0-ClerkLock", "Csh:1-ClerkLock", "Csh:2-ClerkLock", "Csh:3-ClerkLock", "Csh:4-ClerkLock" };
	synchStructNameLengths = { 15, 15, 15, 15, 15 };
	InitializeLocks(cashierLocks, clerkCount, synchStructNames, synchStructNameLengths);

	synchStructNames = { "Csh:0-LineCV", "Csh:1-LineCV", "Csh:2-LineCV", "Csh:3-LineCV", "Csh:4-LineCV" };
	synchStructNameLengths = { 12, 12, 12, 12, 12 };
	InitializeCVs(cashierLineCVs, clerkCount, synchStructNames, synchStructNameLengths);

	synchStructNames = { "Csh:0-BribeLineCV", "Csh:1-BribeLineCV", "Csh:2-BribeLineCV", "Csh:3-BribeLineCV", "Csh:4-BribeLineCV" };
	synchStructNameLengths = { 17, 17, 17, 17, 17 };
	InitializeCVs(cashierBribeLineCVs, clerkCount);

	synchStructNames = { "Csh:0-SenatorLineCV", "Csh:1-SenatorLineCV", "Csh:2-SenatorLineCV", "Csh:3-SenatorLineCV", "Csh:4-SenatorLineCV" };
	synchStructNameLengths = { 19, 19, 19, 19, 19 };
	InitializeCVs(cashierSenatorLineCVs, clerkCount, synchStructNames, synchStructNameLengths);

	synchStructNames = { "Csh:0-WorkCV", "Csh:1-WorkCV", "Csh:2-WorkCV", "Csh:3-WorkCV", "Csh:4-WorkCV" };
	synchStructNameLengths = { 12, 12, 12, 12, 12 };
	InitializeCVs(cashierWorkCVs, clerkCount, synchStructNames, synchStructNameLengths);

	synchStructNames = { "Csh:0-BreakCV", "Csh:1-BreakCV", "Csh:2-BreakCV", "Csh:3-BreakCV", "Csh:4-BreakCV" };
	synchStructNameLengths = { 13, 13, 13, 13, 13 };
	InitializeCVs(cashierBreakCVs, clerkCount, synchStructNames, synchStructNameLengths);

	cashierGroupData[index_clerksData] = cashiers;
	cashierGroupData[index_clerksCount] = clerkCount;
	cashierGroupData[index_clerksMoney] = 0;
	cashierGroupData[index_lineLock] = CreateLock("CshClerks-LineLock", 18);
	cashierGroupData[index_moneyLock] = CreateLock("CshClerks-MoneyLock", 19);
	cashierGroupData[index_clerkLocks] = cashiersLocks;
	cashierGroupData[index_lineCVs] = cashiersLineCVs;
	cashierGroupData[index_bribeLineCVs] = cashiersBribeLineCVs;
	cashierGroupData[index_senatorLineCVs] = cashiersSenatorLineCVs;
	cashierGroupData[index_clerkWorkCVs] = cashiersWorkCVs;
	cashierGroupData[index_breakCVs] = cashiersBreakCVs;
}
*/

void InitializeData ()
{
	InitializeCustomerData();
	InitializeSenatorData();
	/*InitializeApplicationClerkData();*/
	/*InitializePictureClerkData();*/
	/*InitializePassportClerkData();*/
	/*InitializeCashierData();*/
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