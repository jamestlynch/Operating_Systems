/******************************************/
/* 			   Customer Data 			  */
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
int customers[50][7]; /* Array of customers, indexed by ssn (Must support at least 50 customers) */
int numSenators = 10;
int senators[10][7]; /* Array of customers, indexed by ssn (Must support at least 50 customers) */

void CreateCustomerData ()
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

void CreateSenatorData ()
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

int numAppClerks = 5;

void InitializeData ()
{
	CreateCustomerData();
	CreateSenatorData();
}

int main() 
{
	InitializeData();
}