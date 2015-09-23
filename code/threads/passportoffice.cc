#include "copyright.h"
#include "system.h"
#ifdef CHANGED
#include "synch.h"
#endif
#include <stdio.h>

#ifdef CHANGED

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define WHITE   "\033[37m"
#define ANSI_COLOR_RESET   "\x1b[0m"

enum ClerkStatus {AVAILABLE, BUSY, ONBREAK};

const int MoneyOptions[4] = {100, 600, 1100, 1600};
char ** ClerkTypes;


struct CustomerData 
{

    bool turnedInApplication;// = false;
    bool acceptedPicture;// = false;
    bool gotPassport;// = false;
    bool applicationFiled;
    bool photoFiled;
    bool passportCertified;
    bool passportRecorded;
    
    CustomerData()
    {
        //initialize money value of
        applicationFiled = false; //this needs to become true when gotPassport=true, acceptedPicture=true, and yield happens for a random amt of time  
        turnedInApplication = false;
        photoFiled = false;
        passportCertified = false;
        passportRecorded = false;
        acceptedPicture = false;
        gotPassport = false;
    }
};

struct ClerkData 
{
    int lineCount;
    int bribeLineCount;
    bool isBeingBribed;
    int bribeMoney;
    int currentCustomer;
    ClerkStatus state;

    ClerkData() 
    {
        lineCount = 0;
        bribeLineCount = 0;
        bribeMoney = 0;
        isBeingBribed = false;
        currentCustomer = -1;
        state = AVAILABLE;
    }
};

struct FilingJob
{
    int ssn;
    int lineNumber;
    FilingJob(int tempssn, int templineNumber)
    {
        ssn = tempssn;
        lineNumber = templineNumber;
    }
};

Condition ** appClerkCV;
Condition ** picClerkCV;
Condition ** passportClerkCV;
Condition ** cashierCV;

/* CONDITION VARIABLES FOR WAITING ON CLERKS' LINE */
Condition ** appClerkLineCV;
Condition ** picClerkLineCV;
Condition ** passportClerkLineCV;
Condition ** cashierLineCV;

/* CONDITION VARIABLES FOR WAITING ON CLERKS' BRIBE LINE */
Condition ** appClerkBribeLineCV;
Condition ** picClerkBribeLineCV;
Condition ** passportClerkBribeLineCV;
Condition ** cashierBribeLineCV;

/* CONDITION VARIABLES FOR BRIBING */
Condition ** appClerkBribeCV;
Condition ** picClerkBribeCV;
Condition ** passportClerkBribeCV;
Condition ** cashierBribeCV;

/* LOCKS ON CLERK */
Lock ** appClerkLock;
Lock ** picClerkLock;
Lock ** passportClerkLock;
Lock ** cashierLock;

/* LOCKS ON LINE */
Lock appLineLock("ApplicationLineLock");
Lock picLineLock("PictureLineLock");
Lock passportLineLock("PassportLineLock");
Lock cashierLineLock("CashierLineLock");
Lock filingPictureLock("FilingPictureLock");
Lock filingApplicationLock("FilingApplicationLock");
Lock certifyingPassportLock("CertifyingPassportLock");

Condition ** appClerkBreakCV;
Condition ** picClerkBreakCV;
Condition ** passportClerkBreakCV;
Condition ** cashierBreakCV;

CustomerData * customerData;
ClerkData * appClerkData;
ClerkData * passportClerkData;
ClerkData * picClerkData;
ClerkData * cashierData;

int numCustomers = 3;
int numAppClerks = 1;
int numPicClerks = 1;
int numPassportClerks = 1;
int numCashiers = 1;
int numSenators = 1;

int numCustomersFinished = 0;

Semaphore * customerSemaphore;

/********************/
/***** MONITORS *****/
/********************/
struct LineDecisionMonitor 
{
    Lock * lineLock;
    Condition ** lineCV;
    Condition ** bribeLineCV;
    Condition ** bribeCV;
    Condition ** breakCV;
    Condition ** clerkCV;
    Lock ** clerkLock;
    ClerkData * clerkData;
    int numClerks;

    LineDecisionMonitor() {}
};

LineDecisionMonitor * lineDecisionMonitors;


/*********************/
/******* CLERK *******/
/*********************/
void AcceptBribe(int clerkType, int lineNumber)
{
    ClerkData * clerkData = lineDecisionMonitors[clerkType].clerkData;
    Condition ** bribeCV = lineDecisionMonitors[clerkType].bribeCV;
    Lock ** clerkLock = lineDecisionMonitors[clerkType].clerkLock;
    Lock * lineLock = lineDecisionMonitors[clerkType].lineLock;

    lineLock->Acquire();
    bribeCV[lineNumber]->Signal(lineLock);//wake up next customer on my line

    bribeCV[lineNumber]->Wait(lineLock);
    clerkData[lineNumber].bribeMoney += 500;
    printf(GREEN  "%s %d has received $500 from Customer %d"  ANSI_COLOR_RESET  "\n", ClerkTypes[clerkType], lineNumber, clerkData[lineNumber].currentCustomer);
    bribeCV[lineNumber]->Signal(lineLock);
    clerkData[lineNumber].isBeingBribed = false;
    clerkData[lineNumber].currentCustomer = -1; //set current customer back to -1
    lineLock->Release();
}

void Clerk(int clerkType, int lineNumber, VoidFunctionPtr interaction)
{
    Lock * lineLock = lineDecisionMonitors[clerkType].lineLock;
    ClerkData * clerkData = lineDecisionMonitors[clerkType].clerkData;
    Condition ** lineCV = lineDecisionMonitors[clerkType].lineCV;
    Condition ** bribeLineCV = lineDecisionMonitors[clerkType].bribeLineCV;
    Condition ** breakCV = lineDecisionMonitors[clerkType].breakCV;

    lineLock->Acquire();
    interaction(lineNumber);
    //ApplicationClerkToCustomer(lineNumber);
    while (true)
    {
        lineLock->Acquire();

        if(clerkData[lineNumber].isBeingBribed)
        {
            AcceptBribe(clerkType, lineNumber);
        }
        else if(clerkData[lineNumber].bribeLineCount > 0)
        {
            printf(RED  "%s %d has signalled a Customer to come to their counter"  ANSI_COLOR_RESET  "\n", ClerkTypes[clerkType], lineNumber);
            bribeLineCV[lineNumber]->Signal(lineLock);
            clerkData[lineNumber].state = BUSY;
            interaction(lineNumber);
            //ApplicationClerkToCustomer(lineNumber);   
        }
        else if (clerkData[lineNumber].lineCount > 0) 
        {
            // wake up next customer on may line
            printf(GREEN  "%s %d has signalled a Customer to come to their counter"  ANSI_COLOR_RESET  "\n", ClerkTypes[clerkType], lineNumber);
            lineCV[lineNumber]->Signal(lineLock);
            clerkData[lineNumber].state = BUSY;
            interaction(lineNumber);
            //ApplicationClerkToCustomer(lineNumber);
        }
        else
        {
            // nobody is waiting –> Go on break.
            clerkData[lineNumber].state = ONBREAK;
            printf(GREEN  "%s %d is going on break."  ANSI_COLOR_RESET  "\n", ClerkTypes[clerkType], lineNumber);
            breakCV[lineNumber]->Wait(lineLock);
            printf(GREEN  "%s %d is coming off break."  ANSI_COLOR_RESET  "\n", ClerkTypes[clerkType], lineNumber);
            // Go on break.
        }
    }
}



/*
    Simulates customer behavior:
    –   Whether to pick application or picture clerk
*/
void CustomerToApplicationClerk(int ssn, int myLine)
{

    appClerkLock[myLine]->Acquire();
    //Give my data to my clerk
    appClerkData[myLine].currentCustomer = ssn;
    printf(GREEN  "Customer %d has given SSN %d to ApplicationClerk %d."  ANSI_COLOR_RESET  "\n", ssn, ssn, myLine);
    //task is give my data to the clerk using customerData[5]
    appClerkCV[myLine]->Signal(appClerkLock[myLine]);
    //wait for clerk to do their job
    appClerkCV[myLine]->Wait(appClerkLock[myLine]);
    //Read my data
    appClerkCV[myLine]->Signal(appClerkLock[myLine]);
    appClerkLock[myLine]->Release();
}

/*

customer at clerk desk calls V() so that it can acquire clerk's lock
customer is working with clerk
senator enters: grabs every available semaphore slot  –––  can this get interrupted?
    lock
    for i numClerks:
        sem.v()
    release 
    
*/


void fileApplication(FilingJob* jobPointer) 
{

    jobPointer = (FilingJob*)jobPointer;

    int filingTime = (rand() % 80) + 20;
    for (int i = 0; i < filingTime; i++)
    {
        currentThread->Yield();
    }
    
    filingApplicationLock.Acquire();
    customerData[jobPointer->ssn].applicationFiled = true;
    printf(GREEN  "ApplicationClerk %d has recorded a completed application for Customer %d"  ANSI_COLOR_RESET  "\n", jobPointer->lineNumber, jobPointer->ssn);
    filingApplicationLock.Release();

}



void ApplicationClerkToCustomer(int lineNumber)
{
    appClerkLock[lineNumber]->Acquire(); // acquire the lock for my line to pause time.
    appLineLock.Release(); // clerk must know a customer left before starting over
    appClerkCV[lineNumber]->Wait(appClerkLock[lineNumber]);
    printf(GREEN  "ApplicationClerk %d has received SSN %d from Customer %d"  ANSI_COLOR_RESET  "\n", lineNumber, appClerkData[lineNumber].currentCustomer, appClerkData[lineNumber].currentCustomer);
    // do my job - customer nowwaiting
    currentThread->Yield();
    FilingJob * applicationFiling = new FilingJob(appClerkData[lineNumber].currentCustomer, lineNumber);
    Thread * t = new Thread("ApplicationFilingThread");
    t->Fork((VoidFunctionPtr)fileApplication, (int)applicationFiling); //think about where this should go!
    appClerkCV[lineNumber]->Signal(appClerkLock[lineNumber]);
    appClerkData[lineNumber].currentCustomer = -1; //set current customer back to -1
    appClerkCV[lineNumber]->Wait(appClerkLock[lineNumber]);
    appClerkLock[lineNumber]->Release();
}



void ApplicationClerk(int lineNumber)
{
    appLineLock.Acquire();
    ApplicationClerkToCustomer(lineNumber);
    while (true)
    {
        if(appClerkData[lineNumber].isBeingBribed)
        {
            AcceptBribe(0, lineNumber);
            continue;
        }

        appLineLock.Acquire();
        //if (ClerkBribeLineCount[myLine] > 0)
        //clerkBribeLineCV[myLine]->Signal(applicationClerksLineLock);
        //appClerkData[lineNumber].state = BUSY;
        //else
        
        if(appClerkData[lineNumber].bribeLineCount > 0)
        {
            printf(RED  "ApplicationClerk %d has signalled a Customer to come to their counter"  ANSI_COLOR_RESET  "\n", lineNumber);
            appClerkBribeLineCV[lineNumber]->Signal(&appLineLock);
            appClerkData[lineNumber].state = BUSY;
            ApplicationClerkToCustomer(lineNumber);   
        }
        else if (appClerkData[lineNumber].lineCount > 0) 
        {
            // wake up next customer on may line
            printf(GREEN  "ApplicationClerk %d has signalled a Customer to come to their counter"  ANSI_COLOR_RESET  "\n", lineNumber);
            appClerkLineCV[lineNumber]->Signal(&appLineLock);
            appClerkData[lineNumber].state = BUSY;
            ApplicationClerkToCustomer(lineNumber);
        }
        else
        {
            // nobody is waiting –> Go on break.
            appClerkData[lineNumber].state = ONBREAK;
            printf(GREEN  "ApplicationClerk %d is going on break."  ANSI_COLOR_RESET  "\n", lineNumber);
            appClerkBreakCV[lineNumber]->Wait(&appLineLock);
            printf(GREEN  "ApplicationClerk %d is coming off break."  ANSI_COLOR_RESET  "\n", lineNumber);
            // Go on break.
        }
    }
}





























void filePicture(FilingJob* jobPointer) 
{
    jobPointer = (FilingJob*)jobPointer;

    int filingTime = (rand() % 80) + 20;
    for (int i = 0; i < filingTime; i++) { currentThread->Yield(); }

    filingPictureLock.Acquire();
    customerData[jobPointer->ssn].photoFiled = true;
    printf(GREEN  "PictureClerk %d has recorded a filed picture for Customer %d"  ANSI_COLOR_RESET  "\n", jobPointer->lineNumber, jobPointer->ssn);
    filingPictureLock.Release();
}

void CustomerToPictureClerk(int ssn, int myLine)
{
    bool acceptedPicture = false;

    //customer just got to window, wake up, wait to take picture
    picClerkLock[myLine]->Acquire();//simulating the line
    picClerkCV[myLine]->Signal(picClerkLock[myLine]);//take my picture
    picClerkData[myLine].currentCustomer = ssn;
    printf(GREEN  "Customer %d has given SSN %d to PictureClerk %d."  ANSI_COLOR_RESET  "\n", ssn, ssn, myLine);
    picClerkCV[myLine]->Wait(picClerkLock[myLine]); //waiting for you to take my picture
    
    int randomVal = (rand() % 100 + 1);
    if ( randomVal < 50 )
    {
        currentThread->Yield();
        acceptedPicture = true;
        printf(GREEN  "Customer %d does like their picture from PictureClerk %d."  ANSI_COLOR_RESET  "\n", ssn, myLine);
        currentThread->Yield();
        FilingJob * pictureFiling = new FilingJob(ssn, myLine);
        Thread * t = new Thread("PictureFilingThread");
        t->Fork((VoidFunctionPtr)filePicture, (int)pictureFiling);
        
        int filingTime = (rand() % 80) + 20;
        for (int i = 0; i < filingTime; i++) { currentThread->Yield(); }
    }
    else
    {
        printf(GREEN  "Customer %d does not like their picture from PictureClerk %d."  ANSI_COLOR_RESET  "\n", ssn, myLine);
    }
    picClerkCV[myLine]->Signal(picClerkLock[myLine]); //leaving
    picClerkLock[myLine]->Release();
    
    if (!acceptedPicture)
    {
        picLineLock.Acquire();
        // ApplicationClerk is not available, so wait in line
        picClerkData[myLine].lineCount++; // Join the line
        printf(GREEN  "Customer %d has gotten in regular line for PictureClerk %d."  ANSI_COLOR_RESET  "\n", ssn, myLine);
        picClerkLineCV[myLine]->Wait(&picLineLock); // Waiting in line
        // Reacquires lock after getting woken up inside Wait.
        picClerkData[myLine].lineCount--; // Leaving the line
        picLineLock.Release();
        CustomerToPictureClerk(ssn, myLine);
    }
}

void PictureClerkToCustomer(int lineNumber)
{
    // TODO: TRANSFER DATA BETWEEN PIC CLERK AND CUSTOMER
    picClerkLock[lineNumber]->Acquire(); // acquire the lock for my line to pause time.
    picLineLock.Release(); //clerk must know a customer left before starting over
    picClerkCV[lineNumber]->Wait(picClerkLock[lineNumber]);
    printf(GREEN  "PictureClerk %d has received SSN %d from Customer %d"  ANSI_COLOR_RESET  "\n", lineNumber, picClerkData[lineNumber].currentCustomer, picClerkData[lineNumber].currentCustomer);
    picClerkCV[lineNumber]->Signal(picClerkLock[lineNumber]);
    printf(GREEN  "PictureClerk %d has taken a picture of Customer %d."  ANSI_COLOR_RESET  "\n", lineNumber, picClerkData[lineNumber].currentCustomer);
    picClerkData[lineNumber].currentCustomer = -1;
    picClerkCV[lineNumber]->Wait(picClerkLock[lineNumber]);
    picClerkLock[lineNumber]->Release();

}
void PictureClerk(int lineNumber)
{
    picLineLock.Acquire();
    PictureClerkToCustomer(lineNumber);
    while (true)
    {
        if(picClerkData[lineNumber].isBeingBribed)
        {
            AcceptBribe(1, lineNumber);
            continue;
        }

        picLineLock.Acquire();
        //if (ClerkBribeLineCount[myLine] > 0)
        //clerkBribeLineCV[myLine]->Signal(applicationClerksLineLock);
        //picClerkData[lineNumber].state = BUSY;
        /*else*/

        if(picClerkData[lineNumber].bribeLineCount > 0)
        {
            picClerkBribeLineCV[lineNumber]->Signal(&picLineLock);//wake up next customer on my line
            printf(GREEN  "PictureClerk %d has signalled a Customer to come to their counter."  ANSI_COLOR_RESET  "\n", lineNumber);
            picClerkData[lineNumber].state = BUSY;
            PictureClerkToCustomer(lineNumber);
        }
        else if (picClerkData[lineNumber].lineCount > 0) 
        {
            picClerkLineCV[lineNumber]->Signal(&picLineLock);//wake up next customer on my line
            printf(GREEN  "PictureClerk %d has signalled a Customer to come to their counter."  ANSI_COLOR_RESET  "\n", lineNumber);
            picClerkData[lineNumber].state = BUSY;
            PictureClerkToCustomer(lineNumber);

        }
        else
        { 
            // Nobody is waiting –> Go on break.
            picClerkData[lineNumber].state = ONBREAK;
            printf(GREEN  "PictureClerk %d is going on break."  ANSI_COLOR_RESET  "\n", lineNumber);
            picClerkBreakCV[lineNumber]->Wait(&picLineLock);
            printf(GREEN  "PictureClerk %d is coming off break."  ANSI_COLOR_RESET  "\n", lineNumber);
        }
    }
}




























void certifyPassport(FilingJob * certifyingJobPointer){
    certifyingJobPointer = (FilingJob *) certifyingJobPointer;

    int filingTime = (rand() % 80) + 20;
    for (int i = 0; i < filingTime; i++)
    {
        currentThread->Yield();
    }

    certifyingPassportLock.Acquire();
    customerData[certifyingJobPointer->ssn].passportCertified = true;
    printf(GREEN  "PassportClerk %d has recorded Customer %d passport documentation."  ANSI_COLOR_RESET  "\n", certifyingJobPointer->lineNumber, certifyingJobPointer->ssn);
    certifyingPassportLock.Release();
}

void CustomerToPassportClerk(int ssn, int myLine)
{
    // TODO: TRANSMIT DATA FROM CUSTOMER TO PASSPORT CLERK
    passportClerkLock[myLine]->Acquire();//simulating the line
    passportClerkCV[myLine]->Signal(passportClerkLock[myLine]);
    passportClerkData[myLine].currentCustomer = ssn;
    printf(GREEN  "Customer %d has given SSN %d to PassportClerk %d."  ANSI_COLOR_RESET  "\n", ssn, ssn, myLine);
    passportClerkCV[myLine]->Wait(passportClerkLock[myLine]);
        
    //customer pays the passport clerk $100

    filingApplicationLock.Acquire();
    filingPictureLock.Acquire();

    if(!customerData[ssn].applicationFiled || !customerData[ssn].photoFiled){
        printf("PassportClerk %d has determined that Customer %d does not have both their application and picture completed\n", myLine, ssn);
        filingApplicationLock.Release();
        filingPictureLock.Release();

        //customer went to counter too soon,
        //wait an arbitrary amount of time
        int punishmentTime = (rand() % 900) + 100;
        for (int i = 0; i < punishmentTime; i++) { currentThread->Yield(); }


        //go to the back of the line and wait again
        //passportClerkLock[myLine]->Release();
        passportLineLock.Acquire();
        passportClerkCV[myLine]->Signal(passportClerkLock[myLine]); //leaving
        passportClerkLock[myLine]->Release();
        passportClerkData[myLine].lineCount++; // rejoin the line
        printf(GREEN  "Customer %d has gone to PassportClerk %d too soon. They are going to the back of the line."  ANSI_COLOR_RESET  "\n", ssn, myLine);
        passportClerkLineCV[myLine]->Wait(&passportLineLock); // Waiting in line
        // Reacquires lock after getting woken up inside Wait.
        passportClerkData[myLine].lineCount--; // Leaving the line, going to the counter
        passportLineLock.Release();
        CustomerToPassportClerk(ssn, myLine);
        return;
    }

    filingApplicationLock.Release();
    filingPictureLock.Release();
            //thread yield until passportcertification
            //customer leaves co
            //customer gets on line for cashier
  
    // TODO: Move the job creation below inside of Passport Clerk

    // Create a FilingJob in the system for certifying the Customer's passport.
    char * name = new char [40];
    sprintf(name, "PassportCertifyingThread-PC%d-C%d", myLine, ssn);
    Thread * t = new Thread(name);
    FilingJob * passportCertifyingJob = new FilingJob(ssn, myLine);
    t->Fork((VoidFunctionPtr)certifyPassport, (int)passportCertifyingJob); //think about where this should go!

    passportClerkCV[myLine]->Signal(passportClerkLock[myLine]); //leaving
    passportClerkLock[myLine]->Release();
}

void PassportClerkToCustomer(int lineNumber)
{
    passportClerkLock[lineNumber]->Acquire();
    passportLineLock.Release();
    passportClerkCV[lineNumber]->Wait(passportClerkLock[lineNumber]); // Wait for a customer to signal he's ready for me to do work
    printf(GREEN  "PassportClerk %d has received SSN %d from Customer %d"  ANSI_COLOR_RESET  "\n", lineNumber, passportClerkData[lineNumber].currentCustomer, passportClerkData[lineNumber].currentCustomer);
    
    currentThread->Yield(); // One moment, I need to file in the system

    passportClerkCV[lineNumber]->Signal(passportClerkLock[lineNumber]); // Signal the customer that I have done my work
    passportClerkCV[lineNumber]->Wait(passportClerkLock[lineNumber]); // Wait for customer to tell me he is leaving
    passportClerkData[lineNumber].currentCustomer = -1;
    passportClerkLock[lineNumber]->Release();
}

void PassportClerk(int lineNumber)
{
    passportLineLock.Acquire();
    PassportClerkToCustomer(lineNumber);
    while (true)
    {
        if(passportClerkData[lineNumber].isBeingBribed)
        {
            AcceptBribe(2, lineNumber);
            continue;
        }

        passportLineLock.Acquire();
        //if (ClerkBribeLineCount[myLine] > 0)
        //clerkBribeLineCV[myLine]->Signal(applicationClerksLineLock);
        //passportClerkData[lineNumber].state = BUSY;
        /*else*/

        if(passportClerkData[lineNumber].bribeLineCount > 0)
        {
            passportClerkBribeLineCV[lineNumber]->Signal(&passportLineLock);//wake up next customer on my line
            printf(GREEN  "PassportClerk %d has signalled a Customer to come to their counter."  ANSI_COLOR_RESET  "\n", lineNumber);
            passportClerkData[lineNumber].state = BUSY;
            PassportClerkToCustomer(lineNumber);
        }
        else if (passportClerkData[lineNumber].lineCount > 0) 
        {
            passportClerkLineCV[lineNumber]->Signal(&passportLineLock);//wake up next customer on my line
            printf(GREEN  "PassportClerk %d has signalled a Customer to come to their counter."  ANSI_COLOR_RESET  "\n", lineNumber);
            passportClerkData[lineNumber].state = BUSY;
            PassportClerkToCustomer(lineNumber);
        }
        else
        { 
            // Nobody is waiting –> Go on break.
            passportClerkData[lineNumber].state = ONBREAK;
            printf(GREEN  "PassportClerk %d is going on break."  ANSI_COLOR_RESET  "\n", lineNumber);
            passportClerkBreakCV[lineNumber]->Wait(&passportLineLock);
            printf(GREEN  "PassportClerk %d is coming off break."  ANSI_COLOR_RESET  "\n", lineNumber);
        }
    }
}




























void CustomerToCashier(int ssn, int& money, int myLine)
{
    cashierLock[myLine]->Acquire(); // Get access to the clerk's counter
    cashierCV[myLine]->Signal(cashierLock[myLine]); // At the cashier's counter, let him know I'm ready to do work
    cashierData[myLine].currentCustomer = ssn; // Give clerk my SSN
    printf(GREEN  "Customer %d has given SSN %d to Cashier %d."  ANSI_COLOR_RESET  "\n", ssn, ssn, myLine);
    cashierCV[myLine]->Wait(cashierLock[myLine]); // Wait for cashier to respond after doing work
    
    // Cashier did his work, "checking" if you've been certified...
    certifyingPassportLock.Acquire();

    if(!customerData[ssn].passportCertified){ // If you weren't, go back to end of line
        certifyingPassportLock.Release();
        
        // Customer went to cashier too soon, "punish" -> wait an arbitrary amount of time
        int punishmentTime = (rand() % 900) + 100;
        for (int i = 0; i < punishmentTime; i++) { currentThread->Yield(); }
        
        // Go to the back of the line and wait again
        cashierLineLock.Acquire();
        cashierCV[myLine]->Signal(cashierLock[myLine]); //leaving
        cashierLock[myLine]->Release();
        cashierData[myLine].lineCount++; // rejoin the line
        printf(GREEN  "Customer %d has gone to Cashier %d too soon. They are going to the back of the line."  ANSI_COLOR_RESET  "\n", ssn, myLine);
        //cashierCV[myLine]->Signal(cashierLock[myLine]);
        cashierLineCV[myLine]->Wait(&cashierLineLock); // Waiting in line
        cashierData[myLine].lineCount--; // Leaving the line, going to the counter
        cashierLineLock.Release();
        
        CustomerToCashier(ssn, money, myLine); // Try again...
        return;
    }

    // I was certified, so now I pay.
    certifyingPassportLock.Release();

    money -= 100;
    // TODO: Increment cashier's money
    cashierData[myLine].bribeMoney += 100;
    printf(GREEN  "Cashier %d has taken $100 from Customer %d."  ANSI_COLOR_RESET  "\n", myLine, cashierData[myLine].currentCustomer);

    customerData[ssn].gotPassport = true;

    cashierCV[myLine]->Signal(cashierLock[myLine]); // Got my passport from the Cashier, so now I'm leaving
    cashierLock[myLine]->Release();
}

/*void recordCompletion(int ssn, int lineNumber){
    int filingTime = (rand() % 1000) + 100;
    for (int i = 0; i < filingTime; i++)
    {
        currentThread->Yield();
    }
    CustomerData[ssn].passportCompleted=true;
}*/
void CashierToCustomer(int lineNumber)
{
    cashierLock[lineNumber]->Acquire(); // acquire the lock for my line to pause time.
    cashierLineLock.Release(); //clerk must know a customer left before starting over
    cashierCV[lineNumber]->Wait(cashierLock[lineNumber]);
    printf(GREEN  "Cashier %d has received SSN %d from Customer %d"  ANSI_COLOR_RESET  "\n", lineNumber, cashierData[lineNumber].currentCustomer, cashierData[lineNumber].currentCustomer);
    
    cashierCV[lineNumber]->Signal(cashierLock[lineNumber]);
    cashierCV[lineNumber]->Wait(cashierLock[lineNumber]);
    cashierData[lineNumber].currentCustomer = -1;
    cashierLock[lineNumber]->Release();
}

void Cashier(int lineNumber){
    cashierLineLock.Acquire();
    CashierToCustomer(lineNumber);
    while (true)
    {
        if(cashierData[lineNumber].isBeingBribed)
        {
            AcceptBribe(3, lineNumber);
            continue;
        }

        cashierLineLock.Acquire();
        //if (ClerkBribeLineCount[myLine] > 0)
        //clerkBribeLineCV[myLine]->Signal(applicationClerksLineLock);
        //cashierData[lineNumber].state = BUSY;

        if (cashierData[lineNumber].bribeLineCount > 0) 
        {
            cashierBribeLineCV[lineNumber]->Signal(&cashierLineLock);//wake up next customer on my line
            printf(GREEN  "Cashier %d has signalled a Customer to come to their counter."  ANSI_COLOR_RESET  "\n", lineNumber);
            cashierData[lineNumber].state = BUSY;
            CashierToCustomer(lineNumber);
        }
        else if (cashierData[lineNumber].lineCount > 0) 
        {
            cashierLineCV[lineNumber]->Signal(&cashierLineLock);//wake up next customer on my line
            printf(GREEN  "Cashier %d has signalled a Customer to come to their counter."  ANSI_COLOR_RESET  "\n", lineNumber);
            cashierData[lineNumber].state = BUSY;
            CashierToCustomer(lineNumber);
        }
        else
        { 
            // nobody is waiting
            cashierData[lineNumber].state = ONBREAK;
            printf(GREEN  "Cashier %d is going on break."  ANSI_COLOR_RESET  "\n", lineNumber);
            cashierBreakCV[lineNumber]->Wait(&cashierLineLock);
            printf(GREEN  "Cashier %d is coming off break."  ANSI_COLOR_RESET  "\n", lineNumber);
            // Go on break.
        }
    }
}


































int ManageClerk(int clerkType)
{
    Lock * lineLock = lineDecisionMonitors[clerkType].lineLock;
    ClerkData * clerkData = lineDecisionMonitors[clerkType].clerkData;
    Condition ** breakCV = lineDecisionMonitors[clerkType].breakCV;
    int numClerks = lineDecisionMonitors[clerkType].numClerks;

    int clerkMoney = 0;
    int wakeUpClerk = 0;
    int clerksOnBreak = 0;

    lineLock->Acquire();
    for(int i = 0; i < numClerks; i++) 
    {
        clerkMoney += clerkData[i].bribeMoney;

        if(clerkData[i].lineCount >= 3 && clerkData[i].state != ONBREAK) 
        {
            wakeUpClerk++;
        }

        if(clerkData[i].state == ONBREAK)
        {
            clerksOnBreak++;
        }
    }

    for(int i = 0; i < numClerks; i++) 
    {
        // If all clerks are on break, but they have people in their line, wake up that clerk
        if(clerksOnBreak == numClerks && clerkData[i].lineCount > 0)
        {
            clerkData[i].state = AVAILABLE;
            breakCV[i]->Signal(lineLock);
            printf(GREEN  "Manager has woken up %s %d."  ANSI_COLOR_RESET  "\n", ClerkTypes[clerkType], i);
            continue;
        }

        // If a clerk is on break, and another clerk has 3+ people in their line, wake up that clerk
        if(clerkData[i].state == ONBREAK && wakeUpClerk > 0) 
        {
            wakeUpClerk--;

            clerkData[i].state = AVAILABLE;
            breakCV[i]->Signal(lineLock);
            printf(GREEN  "Manager has woken up %s %d."  ANSI_COLOR_RESET  "\n", ClerkTypes[clerkType], i);
        }
    }
    lineLock->Release();

    return clerkMoney;
}

void Manager()
{
    // Wakes up the clerks when there are >3 people waiting
    // Counts each clerks money
    while(true) 
    {
        int appClerkMoney = ManageClerk(0);
        int picClerkMoney = ManageClerk(1);
        int passportClerkMoney = ManageClerk(2);
        int cashierMoney = ManageClerk(3);
        int totalMoney = appClerkMoney + picClerkMoney + passportClerkMoney + cashierMoney;

        printf(GREEN  "Manager has counted a total of $%d for ApplicationClerks."  ANSI_COLOR_RESET  "\n", appClerkMoney);
        printf(GREEN  "Manager has counted a total of $%d for PictureClerks."  ANSI_COLOR_RESET  "\n", picClerkMoney);
        printf(GREEN  "Manager has counted a total of $%d for PassportClerks."  ANSI_COLOR_RESET  "\n", passportClerkMoney);
        printf(GREEN  "Manager has counted a total of $%d for Cashier."  ANSI_COLOR_RESET  "\n", cashierMoney);
        printf(GREEN  "Manager has counted a total of $%d for the passport office."  ANSI_COLOR_RESET  "\n", totalMoney);

        if(numCustomersFinished == numCustomers)
        {
            return;
        }

        for(int i = 0; i < 1000; i++)
        {
            currentThread->Yield();
        }
    }
}
































void Senator()
{

}

void getInput()
{
    bool invalidInput = false;
    char * inputPointer, input[100];

    printf(WHITE  "\n\nhello to the Passport Office"  ANSI_COLOR_RESET  "\n");
    
    printf(WHITE  "How many Customers? [1-50]\t\t"  ANSI_COLOR_RESET);
    while (fgets(input, sizeof(input), stdin)) {
        numCustomers = strtol(input, &inputPointer, 10); // Base-10
        if ((inputPointer == input || *inputPointer != '\n') || !(numCustomers <= 50 && numCustomers >= 1)) { // strtol did not advance pointer to an int
            printf(RED  "Please enter an integer between 1 and 50."  ANSI_COLOR_RESET  "\n");
            printf(WHITE  "How many Customers? [1-50]\t"  ANSI_COLOR_RESET);
        } else break;
    }

    printf(WHITE  "How many Application Clerks? [1-5]\t"  ANSI_COLOR_RESET);
    while (fgets(input, sizeof(input), stdin)) {
        numAppClerks = strtol(input, &inputPointer, 10); // Base-10
        if ((inputPointer == input || *inputPointer != '\n') || !(numAppClerks <= 5 && numAppClerks >= 1)) { // strtol did not advance pointer to an int
            printf(RED  "Please enter an integer between 1 and 5."  ANSI_COLOR_RESET  "\n");
            printf(WHITE  "How many Application Clerks? [1-5]\t"  ANSI_COLOR_RESET);
        } else break;
    }

    printf(WHITE  "How many Picture Clerks? [1-5]\t"  ANSI_COLOR_RESET);
    while (fgets(input, sizeof(input), stdin)) {
        numPicClerks = strtol(input, &inputPointer, 10); // Base-10
        if ((inputPointer == input || *inputPointer != '\n') || !(numPicClerks <= 5 && numPicClerks >= 1)) { // strtol did not advance pointer to an int
            printf(RED  "Please enter an integer between 1 and 5."  ANSI_COLOR_RESET  "\n");
            printf(WHITE  "How many Picture Clerks? [1-5]\t"  ANSI_COLOR_RESET);
        } else break;
    }

    printf(WHITE  "How many Passport Clerks? [1-5]\t"  ANSI_COLOR_RESET);
    while (fgets(input, sizeof(input), stdin)) {
        numPassportClerks = strtol(input, &inputPointer, 10); // Base-10
        if ((inputPointer == input || *inputPointer != '\n') || !(numPassportClerks <= 5 && numPassportClerks >= 1)) { // strtol did not advance pointer to an int
            printf(RED  "Please enter an integer between 1 and 5."  ANSI_COLOR_RESET  "\n");
            printf(WHITE  "How many Passport Clerks? [1-5]\t"  ANSI_COLOR_RESET);
        } else break;
    }

    printf(WHITE  "How many Cashiers? [1-5]\t\t"  ANSI_COLOR_RESET);
    while (fgets(input, sizeof(input), stdin)) {
        numCashiers = strtol(input, &inputPointer, 10); // Base-10
        if ((inputPointer == input || *inputPointer != '\n') || !(numCashiers <= 5 && numCashiers >= 1)) { // strtol did not advance pointer to an int
            printf(RED  "Please enter an integer between 1 and 5."  ANSI_COLOR_RESET  "\n");
            printf(WHITE  "How many Cashiers? [1-5]\t\t"  ANSI_COLOR_RESET);
        } else break;
    }

    printf(WHITE  "How many Senators? [0-10]\t\t"  ANSI_COLOR_RESET);
    while (fgets(input, sizeof(input), stdin)) {
        numSenators = strtol(input, &inputPointer, 10); // Base-10
        if ((inputPointer == input || *inputPointer != '\n') || !(numSenators <= 10 && numSenators >= 0)) { // strtol did not advance pointer to an int
            printf(RED  "Please enter an integer between 0 and 10."  ANSI_COLOR_RESET  "\n");
            printf(WHITE  "How many Senators? [0-10]\t"  ANSI_COLOR_RESET);
        } else break;
    }

    printf(MAGENTA  "Number of Customers = %d"  ANSI_COLOR_RESET  "\n", numCustomers);
    printf(MAGENTA  "Number of ApplicationClerks = %d"  ANSI_COLOR_RESET  "\n", numAppClerks);
    printf(MAGENTA  "Number of PictureClerks = %d"  ANSI_COLOR_RESET  "\n", numPicClerks);
    printf(MAGENTA  "Number of PassportClerks = %d"  ANSI_COLOR_RESET  "\n", numPassportClerks);
    printf(MAGENTA  "Number of Cashiers = %d"  ANSI_COLOR_RESET  "\n", numCashiers);
    printf(MAGENTA  "Number of Senators = %d"  ANSI_COLOR_RESET  "\n", numSenators);
}






























void DecideLine(int ssn, int& money, int clerkType) 
{
    Lock * lineLock = lineDecisionMonitors[clerkType].lineLock;
    ClerkData * clerkData = lineDecisionMonitors[clerkType].clerkData;
    Condition ** lineCV = lineDecisionMonitors[clerkType].lineCV;
    Condition ** bribeLineCV = lineDecisionMonitors[clerkType].bribeLineCV;
    Condition ** bribeCV = lineDecisionMonitors[clerkType].bribeCV;
    Lock ** clerkLock = lineDecisionMonitors[clerkType].clerkLock;
    int numClerks = lineDecisionMonitors[clerkType].numClerks;

    // CS: Need to check the state of all application clerks' lines without them changing
    lineLock->Acquire();
    
    int currentLine = -1; // No line yet
    int currentLineSize = 1000; // Larger (bc we're finding shortest line) than the number customers created
    
    // What if everyone's on break?
    int shortestLine = -1; // Store the shortest line    //(Once a single line has >= 3 Customers, Manager wakes up an ApplicationClerk)
    int shortestLineSize = 1000; // Larger than any line could possibly be because we are searching for shortest line.

    for (int i = 0; i < numClerks; i++) //number of clerks
    {
        // Pick the shortest line with a clerk not on break
        if (clerkData[i].lineCount < currentLineSize && clerkData[i].state != ONBREAK)
        {
            currentLine = i;
            currentLineSize = clerkData[i].lineCount;
        }

        // What if everyones on break?
        if (clerkData[i].lineCount < shortestLineSize) 
        {
            shortestLine = i;
            shortestLineSize = clerkData[i].lineCount;
        }
    }

    // What if everyones on break?
    // Join the longest line and wait for Manager to wake up an Application Clerk (once this line gets at least 3 Customers)
    // ^^^ Actually just pick the shortest because assignment says to
    if (currentLine == -1) // If this is the last ApplicationClerk(number of clerks -1) and we haven't picked a line
    {
        currentLine = shortestLine; // Join the shortest line
        currentLineSize = clerkData[currentLine].lineCount;
    }

    // I've selected a line...
    if (clerkData[currentLine].state != AVAILABLE)// ApplicationClerk is not available, so wait in line
    {
        // Decide if we want to bribe the clerk
        if(clerkData[currentLine].lineCount >= 1 && money >= 600 && clerkData[currentLine].state != ONBREAK)
        {
            clerkData[currentLine].isBeingBribed = true;
            bribeCV[currentLine]->Wait(lineLock);
            clerkData[currentLine].currentCustomer = ssn;
            money -= 500;
            bribeCV[currentLine]->Signal(lineLock);
            bribeCV[currentLine]->Wait(lineLock);
            printf(GREEN  "Customer %d has gotten in bribe line for %s %d."  ANSI_COLOR_RESET  "\n", ssn, ClerkTypes[clerkType], currentLine);

            clerkData[currentLine].bribeLineCount++;
            bribeLineCV[currentLine]->Wait(lineLock);
            clerkData[currentLine].bribeLineCount--;

        }
        else 
        {
            // Join the line
            clerkData[currentLine].lineCount++; 
            printf(GREEN  "Customer %d has gotten in no line for %s %d."  ANSI_COLOR_RESET  "\n", ssn, ClerkTypes[clerkType], currentLine);

            lineCV[currentLine]->Wait(lineLock); // Waiting in line (Reacquires lock after getting woken up inside Wait.)
            clerkData[currentLine].lineCount--; // Leaving the line
        }
    } 
    else // Line was empty to begin with. Clerk is available
    { 
        clerkData[currentLine].state = BUSY;
    }
    lineLock->Release();
    
    switch(clerkType) 
    {
        case 0: CustomerToApplicationClerk(ssn, currentLine); break;
        case 1: CustomerToPictureClerk(ssn, currentLine); break;
        case 2: CustomerToPassportClerk(ssn, currentLine); break;
        case 3: CustomerToCashier(ssn, money, currentLine); break;
    }
}

void Leave(int ssn)
{
    numCustomersFinished++;
    printf(GREEN  "Customer %d is leaving the Passport Office."  ANSI_COLOR_RESET  "\n", ssn);
}


void Customer(int ssn) 
{
    int RandIndex = rand() % 4;
    int money = MoneyOptions[RandIndex];
    int randomVal = (rand() % 100 + 1);
    if (randomVal < 50)
    {
        DecideLine(ssn, money, 0); // clerkType = 0 = ApplicationClerk
        DecideLine(ssn, money, 1); // clerkType = 1 = PictureClerk
    } 
    else 
    {
        DecideLine(ssn, money, 1); // clerkType = 1 = PictureClerk
        DecideLine(ssn, money, 0); // clerkType = 0 = ApplicationClerk
    }

    DecideLine(ssn, money, 2); // clerkType = 2 = PassportClerk
    DecideLine(ssn, money, 3); // clerkType = 3 = Cashier
    Leave(ssn);
}





ClerkData * testClerkData = new ClerkData[5];

void shortestLine() {
    Thread * t;
    char * name;

    // Initialize lines with lineCounts
    for (int i = 0; i < 5; i++) {
        testClerkData[i].lineCount = rand() % 10;
    }

    for(int i = 0; i < numCustomers; i++) 
    {
        name = new char [40];
        sprintf(name, "Customer-%d", i);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)Customer, i);
    }
}


// Customers do not leave until they are given passport
// Senator




























void Part2()
{
    getInput();

    Thread *t;
    char *name;

    appClerkCV = new Condition*[numAppClerks];
    picClerkCV = new Condition*[numPicClerks];
    passportClerkCV = new Condition*[numPassportClerks];
    cashierCV = new Condition*[numCashiers];
    
    appClerkLineCV = new Condition*[numAppClerks];
    picClerkLineCV = new Condition*[numPicClerks];
    passportClerkLineCV = new Condition*[numPassportClerks];
    cashierLineCV = new Condition*[numCashiers];

    appClerkBribeLineCV = new Condition*[numAppClerks];
    picClerkBribeLineCV = new Condition*[numPicClerks];
    passportClerkBribeLineCV = new Condition*[numPassportClerks];
    cashierBribeLineCV = new Condition*[numCashiers];

    appClerkBribeCV = new Condition*[numAppClerks];
    picClerkBribeCV = new Condition*[numPicClerks];
    passportClerkBribeCV = new Condition*[numPassportClerks];
    cashierBribeCV = new Condition*[numCashiers];
    
    appClerkLock = new Lock*[numAppClerks];
    picClerkLock = new Lock*[numPicClerks];
    passportClerkLock = new Lock*[numPassportClerks];
    cashierLock = new Lock*[numCashiers];

    
    customerData = new CustomerData[numCustomers];
    appClerkData = new ClerkData[numAppClerks];
    passportClerkData = new ClerkData[numPassportClerks];
    picClerkData = new ClerkData[numPicClerks];
    cashierData = new ClerkData[numCashiers];

    appClerkBreakCV = new Condition*[numAppClerks];
    picClerkBreakCV = new Condition*[numPicClerks];
    passportClerkBreakCV = new Condition*[numPassportClerks];
    cashierBreakCV = new Condition*[numCashiers];

    // Used to reference clerk data when making decision about which line to get in. 4 types of clerk
    lineDecisionMonitors = new LineDecisionMonitor [4];

    ClerkTypes = new char*[4];

    ClerkTypes[0] = "fuckboy";
    ClerkTypes[1] = "fuckboy2";
    ClerkTypes[2] = "PassportClerk";
    ClerkTypes[3] = "Cashier";

    //  POLISH: If we have time, below could be done in two nested for loops.

    //  ================================================
    //      Application Clerks
    //  ================================================

    for (int i = 0; i < numAppClerks; i++)
    {
        // Create Locks and CVs tied to Clerk
        name = new char [40];
        sprintf(name, "LineCV-ApplicationClerk-%d", i);
        appClerkLineCV[i] = new Condition(name);

        name = new char [40];
        sprintf(name, "BribeLineCV-ApplicationClerk-%d", i);
        appClerkBribeLineCV[i] = new Condition(name);

        name = new char [40];
        sprintf(name, "BribeCV-ApplicationClerk-%d", i);
        appClerkBribeCV[i] = new Condition(name);
        
        name = new char [40];
        sprintf(name, "Lock-ApplicationClerk-%d", i);
        appClerkLock[i] = new Lock(name);
        
        name = new char [40];
        sprintf(name, "WorkCV-ApplicationClerk-%d", i);
        appClerkCV[i] = new Condition(name);
        
        name = new char [40];
        sprintf(name, "BreakCV-ApplicationClerk-%d", i);
        appClerkBreakCV[i] = new Condition(name);

        // Create clerks
        name = new char [40];
        sprintf(name, "ApplicationClerk-%d", i);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)ApplicationClerk, i);
    }

    lineDecisionMonitors[0].lineLock = &appLineLock;
    lineDecisionMonitors[0].lineCV = appClerkLineCV;
    lineDecisionMonitors[0].bribeLineCV = appClerkBribeLineCV;
    lineDecisionMonitors[0].bribeCV = appClerkBribeCV;
    lineDecisionMonitors[0].breakCV = appClerkBreakCV;
    lineDecisionMonitors[0].clerkCV = appClerkCV;
    lineDecisionMonitors[0].clerkLock = appClerkLock;
    lineDecisionMonitors[0].clerkData = appClerkData;
    lineDecisionMonitors[0].numClerks = numAppClerks;


    //  ================================================
    //      Picture Clerks
    //  ================================================


    for(int i = 0; i < numPicClerks; i++)
    {
        // Create Locks and CVs tied to Clerk
        name = new char [40];
        sprintf(name, "LineCV-PictureClerk-%d", i);
        picClerkLineCV[i] = new Condition(name);
        
        name = new char [40];
        sprintf(name, "BribeLineCV-PictureClerk-%d", i);
        picClerkBribeLineCV[i] = new Condition(name);

        name = new char [40];
        sprintf(name, "BribeCV-PictureClerk-%d", i);
        picClerkBribeCV[i] = new Condition(name);

        name = new char [40];
        sprintf(name, "Lock-PictureClerk-%d", i);
        picClerkLock[i] = new Lock(name);
        
        name = new char [40];
        sprintf(name, "WorkCV-PictureClerk-%d", i);
        picClerkCV[i] = new Condition(name);
        
        name = new char [40];
        sprintf(name, "BreakCV-PictureClerk-%d", i);
        picClerkBreakCV[i] = new Condition(name);

        // Create clerks
        name = new char [40];
        sprintf(name, "PictureClerk-%d",i);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)PictureClerk, i);
    }


    lineDecisionMonitors[1].lineLock = &picLineLock;
    lineDecisionMonitors[1].lineCV = picClerkLineCV;
    lineDecisionMonitors[1].bribeLineCV = picClerkBribeLineCV;
    lineDecisionMonitors[1].bribeCV = picClerkBribeCV;
    lineDecisionMonitors[1].breakCV = picClerkBreakCV;
    lineDecisionMonitors[1].clerkCV = picClerkCV;
    lineDecisionMonitors[1].clerkLock = picClerkLock;
    lineDecisionMonitors[1].clerkData = picClerkData;
    lineDecisionMonitors[1].numClerks = numPicClerks;

    //  ================================================
    //      Passport Clerks
    //  ================================================

    for(int i = 0; i < numPassportClerks; i++)
    {
        // Create Locks and CVs tied to Clerk
        name = new char [40];
        sprintf(name, "LineCV-PassportClerk-%d", i);
        passportClerkLineCV[i] = new Condition(name);

        name = new char [40];
        sprintf(name, "BribeLineCV-PassportClerk-%d", i);
        passportClerkBribeLineCV[i] = new Condition(name);
        
        name = new char [40];
        sprintf(name, "BribeCV-PassportClerk-%d", i);
        passportClerkBribeCV[i] = new Condition(name);

        name = new char [40];
        sprintf(name, "Lock-PassportClerk-%d", i);
        passportClerkLock[i] = new Lock(name);
        
        name = new char [40];
        sprintf(name, "WorkCV-PassportClerkCV-%d", i);
        passportClerkCV[i] = new Condition(name);
        
        name = new char [40];
        sprintf(name, "BreakCV-PassportClerkCV-%d", i);
        passportClerkBreakCV[i] = new Condition(name);

        // Create clerks
        name = new char [40];
        sprintf(name, "PassportClerk-%d",i);
        t = new Thread(name);

        t->Fork((VoidFunctionPtr)PassportClerk, i);
    }

    lineDecisionMonitors[2].lineLock = &passportLineLock;
    lineDecisionMonitors[2].lineCV = passportClerkLineCV;
    lineDecisionMonitors[2].bribeLineCV = passportClerkBribeLineCV;
    lineDecisionMonitors[2].bribeCV = passportClerkBribeCV;
    lineDecisionMonitors[2].breakCV = passportClerkBreakCV;
    lineDecisionMonitors[2].clerkCV = passportClerkCV;
    lineDecisionMonitors[2].clerkLock = passportClerkLock;
    lineDecisionMonitors[2].clerkData = passportClerkData;
    lineDecisionMonitors[2].numClerks = numPassportClerks;

    //  ================================================
    //      Cashiers
    //  ================================================

    for(int i = 0; i < numCashiers; i++) 
    {
        // Create Locks and CVs tied to Clerk
        name = new char [40];
        sprintf(name, "LineCV-Cashier-%d", i);
        cashierLineCV[i] = new Condition(name);

        name = new char [40];
        sprintf(name, "BribeLineCV-Cashier-%d", i);
        cashierBribeLineCV[i] = new Condition(name);

        name = new char [40];
        sprintf(name, "BribeCV-Cashier-%d", i);
        cashierBribeCV[i] = new Condition(name);
        
        name = new char [40];
        sprintf(name, "Lock-Cashier-%d", i);
        cashierLock[i] = new Lock(name);
        
        name = new char [40];
        sprintf(name, "WorkCV-CashierCV-%d", i);
        cashierCV[i] = new Condition(name);
            
        name = new char [40];
        sprintf(name, "BreakCV-Cashier-%d", i);
        cashierBreakCV[i] = new Condition(name);

        // Create clerks
        name = new char [40];
        sprintf(name,"Cashier-%d",i);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)Cashier, i);
    }

    lineDecisionMonitors[3].lineLock = &cashierLineLock;
    lineDecisionMonitors[3].lineCV = cashierLineCV;
    lineDecisionMonitors[3].bribeLineCV = cashierBribeLineCV;
    lineDecisionMonitors[3].bribeCV = cashierBribeCV;
    lineDecisionMonitors[3].breakCV = cashierBreakCV;
    lineDecisionMonitors[3].clerkCV = cashierCV;
    lineDecisionMonitors[3].clerkLock = cashierLock;
    lineDecisionMonitors[3].clerkData = cashierData;
    lineDecisionMonitors[3].numClerks = numCashiers;

    //  ================================================
    //      Managers
    //  ================================================


    t = new Thread("Manager");
    t->Fork((VoidFunctionPtr)Manager, 0);
    //  ================================================
    //      Customers
    //  ================================================


    for(int i = 0; i < numCustomers; i++) 
    {
        name = new char [40];
        sprintf(name, "Customer-%d", i);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)Customer, i);
    }
}
#endif
/*

TO DO
senators

cashier

write all the tests


write up

*/

