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
int *customers[50][7]; /* Array of customers, indexed by ssn (Must support at least 50 customers) */

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

void InitializeData ()
{

	CreateCustomerData();
}

int main() 
{
	InitializeData();
}