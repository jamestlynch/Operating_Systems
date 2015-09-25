// threadtest.cc 
//  Simple test case for the threads assignment.
//
//  Create two threads, and have them context switch
//  back and forth between themselves by calling Thread::Yield, 
//  to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
//#include "passportoffice.cc"
#ifdef CHANGED
#include "synch.h"
#endif

#include <stdio.h>

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define WHITE   "\033[37m"
#define ANSI_COLOR_RESET   "\x1b[0m"


//----------------------------------------------------------------------
// SimpleThread
//  Loop 5 times, yielding the CPU to another ready thread 
//  each iteration.
//
//  "which" is simply a number identifying the thread, for debugging
//  purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
    printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest
//  Set up a ping-pong between two threads, by forking a thread 
//  to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest()
{
    DEBUG('t', "Entering SimpleTest");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}

#ifdef CHANGED
// --------------------------------------------------
// Test Suite
// --------------------------------------------------


// --------------------------------------------------
// Test 1 - see TestSuite() for details
// --------------------------------------------------
Semaphore t1_s1("t1_s1",0);       // To make sure t1_t1 acquires the
                                  // lock before t1_t2
Semaphore t1_s2("t1_s2",0);       // To make sure t1_t2 Is waiting on the 
                                  // lock before t1_t3 releases it
Semaphore t1_s3("t1_s3",0);       // To make sure t1_t1 does not release the
                                  // lock before t1_t3 tries to acquire it
Semaphore t1_done("t1_done",0);   // So that TestSuite knows when Test 1 is
                                  // done
Lock t1_l1("t1_l1");          // the lock tested in Test 1

// --------------------------------------------------
// t1_t1() -- test1 thread 1
//     This is the rightful lock owner
// --------------------------------------------------
void t1_t1() {
    t1_l1.Acquire();
    t1_s1.V();  // Allow t1_t2 to try to Acquire Lock
 
    printf ("%s: Acquired Lock %s, waiting for t3\n",currentThread->getName(),
        t1_l1.getName());
    t1_s3.P();
    printf ("%s: working in CS\n",currentThread->getName());
    for (int i = 0; i < 1000000; i++) ;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
        t1_l1.getName());
    printf("release\n");
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t2() -- test1 thread 2
//     This thread will wait on the held lock.
// --------------------------------------------------
void t1_t2() {

    t1_s1.P();  // Wait until t1 has the lock
    t1_s2.V();  // Let t3 try to acquire the lock
    printf("%s: trying to acquire lock %s\n",currentThread->getName(),
       t1_l1.getName());
    t1_l1.Acquire();

    printf ("%s: Acquired Lock %s, working in CS\n",currentThread->getName(),
        t1_l1.getName());
    for (int i = 0; i < 10; i++)
    ;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
        t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t3() -- test1 thread 3
//     This thread will try to release the lock illegally
// --------------------------------------------------
void t1_t3() {

    t1_s2.P();  // Wait until t2 is ready to try to acquire the lock

    t1_s3.V();  // Let t1 do it's stuff
    for ( int i = 0; i < 3; i++ ) {
    printf("%s: Trying to release Lock %s\n",currentThread->getName(),
           t1_l1.getName());
    t1_l1.Release();
    }
}

// --------------------------------------------------
// Test 2 - see TestSuite() for details
// --------------------------------------------------
Lock t2_l1("t2_l1");        // For mutual exclusion
Condition t2_c1("t2_c1");   // The condition variable to test
Semaphore t2_s1("t2_s1",0); // To ensure the Signal comes before the wait
Semaphore t2_done("t2_done",0);     // So that TestSuite knows when Test 2 is
                                  // done

// --------------------------------------------------
// t2_t1() -- test 2 thread 1
//     This thread will signal a variable with nothing waiting
// --------------------------------------------------
void t2_t1() {
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
       t2_l1.getName(), t2_c1.getName());
    t2_c1.Signal(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
       t2_l1.getName());
    t2_l1.Release();
    t2_s1.V();  // release t2_t2
    t2_done.V();
}

// --------------------------------------------------
// t2_t2() -- test 2 thread 2
//     This thread will wait on a pre-signalled variable
// --------------------------------------------------
void t2_t2() {
    t2_s1.P();  // Wait for t2_t1 to be done with the lock
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
       t2_l1.getName(), t2_c1.getName());
    t2_c1.Wait(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
       t2_l1.getName());
    t2_l1.Release();
}
// --------------------------------------------------
// Test 3 - see TestSuite() for details
// --------------------------------------------------
Lock t3_l1("t3_l1");        // For mutual exclusion
Condition t3_c1("t3_c1");   // The condition variable to test
Semaphore t3_s1("t3_s1",0); // To ensure the Signal comes before the wait
Semaphore t3_done("t3_done",0); // So that TestSuite knows when Test 3 is
                                // done

// --------------------------------------------------
// t3_waiter()
//     These threads will wait on the t3_c1 condition variable.  Only
//     one t3_waiter will be released
// --------------------------------------------------
void t3_waiter() {
    t3_l1.Acquire();
    t3_s1.V();      // Let the signaller know we're ready to wait
       // printf("CV NAME: %s \n", t3_c1.getName());

    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
       t3_l1.getName(), t3_c1.getName());
    t3_c1.Wait(&t3_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t3_c1.getName());
    t3_l1.Release();
    t3_done.V();
}


// --------------------------------------------------
// t3_signaller()
//     This threads will signal the t3_c1 condition variable.  Only
//     one t3_signaller will be released
// --------------------------------------------------
void t3_signaller() {

    // Don't signal until someone's waiting
    
    for ( int i = 0; i < 5 ; i++ ) 
    t3_s1.P();
    t3_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
       t3_l1.getName(), t3_c1.getName());
    t3_c1.Signal(&t3_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t3_l1.getName());
    t3_l1.Release();
    t3_done.V();
}
 
// --------------------------------------------------
// Test 4 - see TestSuite() for details
// --------------------------------------------------
Lock t4_l1("t4_l1");        // For mutual exclusion
Condition t4_c1("t4_c1");   // The condition variable to test
Semaphore t4_s1("t4_s1",0); // To ensure the Signal comes before the wait
Semaphore t4_done("t4_done",0); // So that TestSuite knows when Test 4 is
                                // done

// --------------------------------------------------
// t4_waiter()
//     These threads will wait on the t4_c1 condition variable.  All
//     t4_waiters will be released
// --------------------------------------------------
void t4_waiter() {
    t4_l1.Acquire();
    t4_s1.V();      // Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
       t4_l1.getName(), t4_c1.getName());
    t4_c1.Wait(&t4_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t4_c1.getName());
    t4_l1.Release();
    t4_done.V();
}


// --------------------------------------------------
// t2_signaller()
//     This thread will broadcast to the t4_c1 condition variable.
//     All t4_waiters will be released
// --------------------------------------------------
void t4_signaller() {

    // Don't broadcast until someone's waiting
    
    for ( int i = 0; i < 5 ; i++ ) 
    t4_s1.P();
    t4_l1.Acquire();
    printf("%s: Lock %s acquired, broadcasting %s\n",currentThread->getName(),
       t4_l1.getName(), t4_c1.getName());
    t4_c1.Broadcast(&t4_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t4_l1.getName());
    t4_l1.Release();
    t4_done.V();
}
// --------------------------------------------------
// Test 5 - see TestSuite() for details
// --------------------------------------------------
Lock t5_l1("t5_l1");        // For mutual exclusion
Lock t5_l2("t5_l2");        // Second lock for the bad behavior
Condition t5_c1("t5_c1");   // The condition variable to test
Semaphore t5_s1("t5_s1",0); // To make sure t5_t2 acquires the lock after
                                // t5_t1

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a condition under t5_l1
// --------------------------------------------------
void t5_t1() {
    t5_l1.Acquire();
    t5_s1.V();  // release t5_t2
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
       t5_l1.getName(), t5_c1.getName());
    t5_c1.Wait(&t5_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
       t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a t5_c1 condition under t5_l2, which is
//     a Fatal error
// --------------------------------------------------
void t5_t2() {
    t5_s1.P();  // Wait for t5_t1 to get into the monitor
    t5_l1.Acquire();
    t5_l2.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
       t5_l2.getName(), t5_c1.getName());
    t5_c1.Signal(&t5_l2);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
       t5_l2.getName());
    t5_l2.Release();
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
       t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// TestSuite()
//     This is the main thread of the test suite.  It runs the
//     following tests:
//
//       1.  Show that a thread trying to release a lock it does not
//       hold does not work
//
//       2.  Show that Signals are not stored -- a Signal with no
//       thread waiting is ignored
//
//       3.  Show that Signal only wakes 1 thread
//
//       4.  Show that Broadcast wakes all waiting threads
//
//       5.  Show that Signalling a thread waiting under one lock
//       while holding another is a Fatal error
//
//     Fatal errors terminate the thread in question.
// --------------------------------------------------
void TestSuite() {
    Thread *t;
    char *name;
    int i;
    
    // Test 1

    printf("Starting Test 1\n");

    t = new Thread("t1_t1");
    t->Fork((VoidFunctionPtr)t1_t1,0);

    t = new Thread("t1_t2");
    t->Fork((VoidFunctionPtr)t1_t2,0);

    t = new Thread("t1_t3");
    t->Fork((VoidFunctionPtr)t1_t3,0);

    // Wait for Test 1 to complete
    for (  i = 0; i < 2; i++ )
    t1_done.P();

    // Test 2

    printf("Starting Test 2.  Note that it is an error if thread t2_t2\n");
    printf("completes\n");

    t = new Thread("t2_t1");
    t->Fork((VoidFunctionPtr)t2_t1,0);

    t = new Thread("t2_t2");
    t->Fork((VoidFunctionPtr)t2_t2,0);

    // Wait for Test 2 to complete
    t2_done.P();

    // Test 3

    printf("Starting Test 3\n");

    for (  i = 0 ; i < 5 ; i++ ) {
    name = new char [20];
    sprintf(name,"t3_waiter%d",i);
    t = new Thread(name);
    t->Fork((VoidFunctionPtr)t3_waiter,0);
    }
    t = new Thread("t3_signaller");
    t->Fork((VoidFunctionPtr)t3_signaller,0);

    // Wait for Test 3 to complete
    for (  i = 0; i < 2; i++ )
    t3_done.P();

    // Test 4

    printf("Starting Test 4\n");

    for (  i = 0 ; i < 5 ; i++ ) {
    name = new char [20];
    sprintf(name,"t4_waiter%d",i);
    t = new Thread(name);
    t->Fork((VoidFunctionPtr)t4_waiter,0);
    }
    t = new Thread("t4_signaller");
    t->Fork((VoidFunctionPtr)t4_signaller,0);

    // Wait for Test 4 to complete
    for (  i = 0; i < 6; i++ )
    t4_done.P();

    // Test 5

    printf("Starting Test 5.  Note that it is an error if thread t5_t1\n");
    printf("completes\n");

    t = new Thread("t5_t1");
    t->Fork((VoidFunctionPtr)t5_t1,0);

    t = new Thread("t5_t2");
    t->Fork((VoidFunctionPtr)t5_t2,0);

}

enum ClerkStatus {AVAILABLE, BUSY, ONBREAK};

const int MoneyOptions[4] = {100, 600, 1100, 1600};
char ** ClerkTypes;

bool runningSimulation = false;


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
    int isBeingBribed;
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

struct ManagerData
{
    int appClerkMoney;
    int picClerkMoney;
    int passportClerkMoney;
    int cashierMoney;
    int totalMoney;

    ManagerData()
    {
        appClerkMoney = 0;
        picClerkMoney = 0;
        passportClerkMoney = 0;
        cashierMoney = 0;
        totalMoney = 0;
    }
};

ManagerData managerData;

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

Lock appMoneyLock("ApplicationMoneyLock");
Lock picMoneyLock("PictureMoneyLock");
Lock passportMoneyLock("PassportMoneyLock");
Lock cashierMoneyLock("CashierMoneyLock");

Semaphore * customersFinished;

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

Semaphore CustomersFinished("CustomersFinished", 0);


/********************/
/***** MONITORS *****/
/********************/
struct ClerkGroupData 
{
    Lock * lineLock;
    Condition ** lineCV;
    Condition ** bribeLineCV;
    Condition ** bribeCV;
    Condition ** breakCV;
    Condition ** clerkCV;
    Lock ** clerkLock;
    Lock * moneyLock;
    ClerkData * clerkData;
    
    int numClerks;
    int totalMoney;

    ClerkGroupData() {}
};

ClerkGroupData * clerkGroupData;

/*********************/
/******* CLERK *******/
/*********************/
struct ClerkFunctionStruct
{
    int clerkType;
    int lineNumber;
    VoidFunctionPtr interaction;

    ClerkFunctionStruct(int inClerkType, int inLineNumber, VoidFunctionPtr inInteraction)
    {
        clerkType = inClerkType;
        lineNumber = inLineNumber;
        interaction = inInteraction;
    }
};

void AcceptBribe(int clerkType, int lineNumber)
{
    ClerkData * clerkData = clerkGroupData[clerkType].clerkData;
    Condition ** bribeCV = clerkGroupData[clerkType].bribeCV;
    Condition ** clerkCV = clerkGroupData[clerkType].clerkCV;
    Lock ** clerkLock = clerkGroupData[clerkType].clerkLock;
    Lock * lineLock = clerkGroupData[clerkType].lineLock;

    Lock * moneyLock = clerkGroupData[clerkType].moneyLock;

    bribeCV[lineNumber]->Signal(lineLock);// wake up next customer on my line

    clerkLock[lineNumber]->Acquire();
    lineLock->Release();

    clerkCV[lineNumber]->Wait(clerkLock[lineNumber]);

    //clerkData[lineNumber].bribeMoney += 500;

    moneyLock->Acquire();
    clerkGroupData[clerkType].totalMoney += 500;
    moneyLock->Release();

    printf(GREEN  "%s %d has received $500 from Customer %d"  ANSI_COLOR_RESET  "\n", ClerkTypes[clerkType], lineNumber, clerkData[lineNumber].currentCustomer);
    clerkCV[lineNumber]->Signal(clerkLock[lineNumber]);
    //clerkData[lineNumber].isBeingBribed = false;
    clerkData[lineNumber].currentCustomer = -1; // set current customer back to -1
    clerkLock[lineNumber]->Release();
}

void Clerk(ClerkFunctionStruct * clerkFunctionStruct)
{
    clerkFunctionStruct = (ClerkFunctionStruct*)clerkFunctionStruct;

    int clerkType = clerkFunctionStruct->clerkType;
    int lineNumber = clerkFunctionStruct->lineNumber;
    VoidFunctionPtr interaction = clerkFunctionStruct->interaction;

    Lock * lineLock = clerkGroupData[clerkType].lineLock;
    ClerkData * clerkData = clerkGroupData[clerkType].clerkData;
    Condition ** lineCV = clerkGroupData[clerkType].lineCV;
    Condition ** bribeLineCV = clerkGroupData[clerkType].bribeLineCV;
    Condition ** breakCV = clerkGroupData[clerkType].breakCV;

    lineLock->Acquire();

    interaction(lineNumber);
    while (true)
    {
        lineLock->Acquire();
        if(clerkData[lineNumber].isBeingBribed > 0)
        {
            printf("being bribed\n");
            AcceptBribe(clerkType, lineNumber);
        }
        else if(clerkData[lineNumber].bribeLineCount > 0)
        {
            printf(GREEN  "%s %d has signalled a Customer to come to their counter"  ANSI_COLOR_RESET  "\n", ClerkTypes[clerkType], lineNumber);
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



/*********************************/
/******* APPLICATION CLERK *******/
/*********************************/
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

void FileApplication(FilingJob* jobPointer) 
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
    t->Fork((VoidFunctionPtr)FileApplication, (int)applicationFiling); //think about where this should go!
    appClerkCV[lineNumber]->Signal(appClerkLock[lineNumber]);
    appClerkData[lineNumber].currentCustomer = -1; //set current customer back to -1
    appClerkCV[lineNumber]->Wait(appClerkLock[lineNumber]);
    appClerkLock[lineNumber]->Release();
}

























/*********************************/
/******* PICTURE CLERK ***********/
/*********************************/
void FilePicture(FilingJob* jobPointer) 
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
        t->Fork((VoidFunctionPtr)FilePicture, (int)pictureFiling);
        
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

























/******************************/
/******* PASSPORT CLERK *******/
/******************************/
void CertifyPassport(FilingJob * certifyingJobPointer)
{
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
    t->Fork((VoidFunctionPtr)CertifyPassport, (int)passportCertifyingJob); //think about where this should go!

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

























/***********************/
/******* CASHIER *******/
/***********************/
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
    //cashierData[myLine].bribeMoney += 100;

    cashierMoneyLock.Acquire();
    clerkGroupData[3].totalMoney += 100;
    cashierMoneyLock.Release();

    printf(GREEN  "Cashier %d has taken $100 from Customer %d."  ANSI_COLOR_RESET  "\n", myLine, cashierData[myLine].currentCustomer);

    customerData[ssn].gotPassport = true;

    cashierCV[myLine]->Signal(cashierLock[myLine]); // Got my passport from the Cashier, so now I'm leaving
    cashierLock[myLine]->Release();
}

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






























/***********************/
/******* MANAGER *******/
/***********************/
int ManageClerk(int clerkType)
{
    Lock * lineLock = clerkGroupData[clerkType].lineLock;
    Lock ** clerkLock = clerkGroupData[clerkType].clerkLock;
    ClerkData * clerkData = clerkGroupData[clerkType].clerkData;
    Condition ** breakCV = clerkGroupData[clerkType].breakCV;
    int numClerks = clerkGroupData[clerkType].numClerks;

    Lock * moneyLock = clerkGroupData[clerkType].moneyLock;

    int clerkMoney = 0;
    int wakeUpClerk = 0;
    int clerksOnBreak = 0;

    lineLock->Acquire();

    moneyLock->Acquire();
    clerkMoney = clerkGroupData[clerkType].totalMoney;
    moneyLock->Release();

    for(int i = 0; i < numClerks; i++) 
    {
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
        managerData.appClerkMoney = ManageClerk(0);
        //currentThread->Yield();
        managerData.picClerkMoney= ManageClerk(1);
        //currentThread->Yield();
        managerData.passportClerkMoney = ManageClerk(2);
        //currentThread->Yield();
        managerData.cashierMoney = ManageClerk(3);
        //currentThread->Yield();
        managerData.totalMoney = managerData.appClerkMoney + managerData.picClerkMoney + managerData.passportClerkMoney + managerData.cashierMoney;

        printf(GREEN  "Manager has counted a total of $%d for ApplicationClerks."  ANSI_COLOR_RESET  "\n", managerData.appClerkMoney);
        printf(GREEN  "Manager has counted a total of $%d for PictureClerks."  ANSI_COLOR_RESET  "\n", managerData.picClerkMoney);
        printf(GREEN  "Manager has counted a total of $%d for PassportClerks."  ANSI_COLOR_RESET  "\n", managerData.passportClerkMoney);
        printf(GREEN  "Manager has counted a total of $%d for Cashier."  ANSI_COLOR_RESET  "\n", managerData.cashierMoney);
        printf(GREEN  "Manager has counted a total of $%d for the passport office."  ANSI_COLOR_RESET  "\n", managerData.totalMoney);

        if(numCustomersFinished == numCustomers)
        {
            return;
        }

        //for(int i = 0; i < 100; i++)
        {
            currentThread->Yield();
        }
    }
}

void Senator()
{

}




/************************/
/******* CUSTOMER *******/
/************************/
int DecideLine(int ssn, int& money, int clerkType) 
{
    Lock * lineLock = clerkGroupData[clerkType].lineLock;
    ClerkData * clerkData = clerkGroupData[clerkType].clerkData;
    Condition ** lineCV = clerkGroupData[clerkType].lineCV;
    Condition ** bribeLineCV = clerkGroupData[clerkType].bribeLineCV;
    Condition ** bribeCV = clerkGroupData[clerkType].bribeCV;
    Condition ** clerkCV = clerkGroupData[clerkType].clerkCV;
    Lock ** clerkLock = clerkGroupData[clerkType].clerkLock;
    int numClerks = clerkGroupData[clerkType].numClerks;

    // CS: Need to check the state of all application clerks' lines without them changing
    lineLock->Acquire();
    
    int currentLine = -1; // No line yet
    int currentLineSize = 1000; // Larger (bc we're finding shortest line) than the number customers created
    
    // What if everyone's on break?
    int shortestLine = -1; // Store the shortest line    //(Once a single line has >= 3 Customers, Manager wakes up an ApplicationClerk)
    int shortestLineSize = 1000; // Larger than any line could possibly be because we are searching for shortest line.



    for (int i = 0; i < numClerks; i++) //number of clerks
    {
        if (clerkData[i].lineCount == 0 && clerkData[i].state == AVAILABLE) {
            currentLine = i;
            currentLineSize = clerkData[i].lineCount;
            break;
        }

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
            clerkData[currentLine].isBeingBribed++;
            bribeCV[currentLine]->Wait(lineLock);
            clerkData[currentLine].isBeingBribed--;
            lineLock->Release();

            clerkLock[currentLine]->Acquire();

            clerkData[currentLine].currentCustomer = ssn;
            money -= 500;
            clerkCV[currentLine]->Signal(clerkLock[currentLine]);
            clerkCV[currentLine]->Wait(clerkLock[currentLine]);
            printf(GREEN  "Customer %d has gotten in bribe line for %s %d."  ANSI_COLOR_RESET  "\n", ssn, ClerkTypes[clerkType], currentLine);
            clerkLock[currentLine]->Release();

            lineLock->Acquire();
            clerkData[currentLine].bribeLineCount++;
            bribeLineCV[currentLine]->Wait(lineLock);
            printf("Customer %d \n", ssn);
            clerkData[currentLine].bribeLineCount--;

        }
        else 
        {
            // Join the line
            clerkData[currentLine].lineCount++; 
            printf(GREEN  "Customer %d has gotten in a line for %s %d."  ANSI_COLOR_RESET  "\n", ssn, ClerkTypes[clerkType], currentLine);

            lineCV[currentLine]->Wait(lineLock); // Waiting in line (Reacquires lock after getting woken up inside Wait.)
            
            clerkData[currentLine].lineCount--; // Leaving the line
        }
    } 
    else // Line was empty to begin with. Clerk is available
    { 
        clerkData[currentLine].state = BUSY;
    }
    lineLock->Release();
    
    return currentLine;
}

void Leave(int ssn)
{
    numCustomersFinished++;
    printf(GREEN  "Customer %d is leaving the Passport Office."  ANSI_COLOR_RESET  "\n", ssn);
    CustomersFinished.V();
}

void Customer(int ssn) 
{
    int RandIndex = rand() % 4;
    int money = MoneyOptions[RandIndex];
    int currentLine = -1;

    int randomVal = (rand() % 100 + 1);
    if (randomVal < 50)
    {
        currentLine = DecideLine(ssn, money, 0); // clerkType = 0 = ApplicationClerk
        CustomerToApplicationClerk(ssn, currentLine);
        currentLine = DecideLine(ssn, money, 1); // clerkType = 1 = PictureClerk
        CustomerToPictureClerk(ssn, currentLine);
    } 
    else 
    {
        currentLine = DecideLine(ssn, money, 1); // clerkType = 1 = PictureClerk
        CustomerToPictureClerk(ssn, currentLine);
        currentLine = DecideLine(ssn, money, 0); // clerkType = 0 = ApplicationClerk
        CustomerToApplicationClerk(ssn, currentLine);
    }

    currentLine = DecideLine(ssn, money, 2); // clerkType = 2 = PassportClerk
    CustomerToPassportClerk(ssn, currentLine);
    currentLine = DecideLine(ssn, money, 3); // clerkType = 3 = Cashier
    CustomerToCashier(ssn, money, currentLine);
    Leave(ssn);
}























/******************************/
/******* INITIALIZATION *******/
/******************************/
void GetInput()
{
    bool invalidInput = false;
    char * inputPointer, input[100];

    printf(WHITE  "\n\nWelcome to the Passport Office"  ANSI_COLOR_RESET  "\n");
    
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

void InitializeData()
{

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
    clerkGroupData = new ClerkGroupData [4];

    ClerkTypes = new char*[4];

    ClerkTypes[0] = "ApplicationClerk";
    ClerkTypes[1] = "PictureClerk";
    ClerkTypes[2] = "PassportClerk";
    ClerkTypes[3] = "Cashier";
}

void CleanUpData()
{
    Condition ** tempCV;

    tempCV = appClerkCV;
    delete tempCV;
    appClerkCV = NULL;

    tempCV = picClerkCV;
    delete tempCV;
    picClerkCV = NULL;

    tempCV = passportClerkCV;
    delete tempCV;
    passportClerkCV = NULL;

    tempCV = cashierCV;
    delete tempCV;
    cashierCV = NULL;

    tempCV = appClerkLineCV;
    delete tempCV;
    appClerkLineCV = NULL;

    tempCV = picClerkLineCV;
    delete tempCV;
    picClerkLineCV = NULL;

    tempCV = passportClerkLineCV;
    delete tempCV;
    passportClerkLineCV = NULL;

    tempCV = cashierLineCV;
    delete tempCV;
    cashierLineCV = NULL;

    tempCV = appClerkBribeLineCV;
    delete tempCV;
    appClerkBribeLineCV = NULL;

    tempCV = picClerkBribeLineCV;
    delete tempCV;
    picClerkBribeLineCV = NULL;

    tempCV = passportClerkBribeLineCV;
    delete tempCV;
    passportClerkBribeLineCV = NULL;

    tempCV = cashierBribeLineCV;
    delete tempCV;
    cashierBribeLineCV = NULL;

    tempCV = cashierBribeLineCV;
    delete tempCV;
    cashierBribeLineCV = NULL;

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
    clerkGroupData = new ClerkGroupData [4];

    ClerkTypes = new char*[4];

    ClerkTypes[0] = "ApplicationClerk";
    ClerkTypes[1] = "PictureClerk";
    ClerkTypes[2] = "PassportClerk";
    ClerkTypes[3] = "Cashier";
}

void InitializeAppClerks () 
{
    Thread * t;
    char * name;

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


        ClerkFunctionStruct * clerkFunctionStruct = new ClerkFunctionStruct(0, i, (VoidFunctionPtr) ApplicationClerkToCustomer);

        // Create clerks
        name = new char [40];
        sprintf(name, "ApplicationClerk-%d", i);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)Clerk, (int) clerkFunctionStruct);
    }

    clerkGroupData[0].lineLock = &appLineLock;
    clerkGroupData[0].lineCV = appClerkLineCV;
    clerkGroupData[0].bribeLineCV = appClerkBribeLineCV;
    clerkGroupData[0].bribeCV = appClerkBribeCV;
    clerkGroupData[0].breakCV = appClerkBreakCV;
    clerkGroupData[0].clerkCV = appClerkCV;
    clerkGroupData[0].clerkLock = appClerkLock;
    clerkGroupData[0].moneyLock = &appMoneyLock;
    clerkGroupData[0].clerkData = appClerkData;
    clerkGroupData[0].numClerks = numAppClerks;
}

void CleanUpAppClerks()
{
    Thread * t;
    char * name;

    Condition * tempCV;
    Lock * tempLock;

    for (int i = 0; i < numAppClerks; i++)
    {
        tempCV = appClerkLineCV[i];
        delete tempCV;
        appClerkLineCV[i] = NULL;

        tempCV = appClerkBribeLineCV[i];
        delete tempCV;
        appClerkBribeLineCV[i] = NULL;

        tempCV = appClerkBribeCV[i];
        delete tempCV;
        appClerkBribeCV[i] = NULL;

        tempLock = appClerkLock[i];
        delete tempLock;
        appClerkLock[i] = NULL;
        
        tempCV = appClerkCV[i];
        delete tempCV;
        appClerkCV[i] = NULL;

        tempCV = appClerkBreakCV[i];
        delete tempCV;
        appClerkBreakCV[i] = NULL;
    }

    clerkGroupData[0].lineLock = NULL;
    clerkGroupData[0].lineCV = NULL;
    clerkGroupData[0].bribeLineCV = NULL;
    clerkGroupData[0].bribeCV = NULL;
    clerkGroupData[0].breakCV = NULL;
    clerkGroupData[0].clerkCV = NULL;
    clerkGroupData[0].clerkLock = NULL;
    clerkGroupData[0].moneyLock = NULL;
    clerkGroupData[0].clerkData = NULL;
    clerkGroupData[0].numClerks = 0;
}

void InitializePicClerks () 
{
    Thread * t;
    char * name;

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

        ClerkFunctionStruct * clerkFunctionStruct = new ClerkFunctionStruct(1, i, (VoidFunctionPtr) PictureClerkToCustomer);

        // Create clerks
        name = new char [40];
        sprintf(name, "PictureClerk-%d",i);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)Clerk, (int) clerkFunctionStruct);
    }


    clerkGroupData[1].lineLock = &picLineLock;
    clerkGroupData[1].lineCV = picClerkLineCV;
    clerkGroupData[1].bribeLineCV = picClerkBribeLineCV;
    clerkGroupData[1].bribeCV = picClerkBribeCV;
    clerkGroupData[1].breakCV = picClerkBreakCV;
    clerkGroupData[1].clerkCV = picClerkCV;
    clerkGroupData[1].clerkLock = picClerkLock;
    clerkGroupData[1].moneyLock = &picMoneyLock;
    clerkGroupData[1].clerkData = picClerkData;
    clerkGroupData[1].numClerks = numPicClerks;
}

void CleanUpPicClerks()
{
    Thread * t;
    char * name;

    Condition * tempCV;
    Lock * tempLock;

    for (int i = 0; i < numPicClerks; i++)
    {
        tempCV = picClerkLineCV[i];
        delete tempCV;
        picClerkLineCV[i] = NULL;

        tempCV = picClerkBribeLineCV[i];
        delete tempCV;
        picClerkBribeLineCV[i] = NULL;

        tempCV = picClerkBribeCV[i];
        delete tempCV;
        picClerkBribeCV[i] = NULL;

        tempLock = picClerkLock[i];
        delete tempLock;
        picClerkLock[i] = NULL;
        
        tempCV = picClerkCV[i];
        delete tempCV;
        picClerkCV[i] = NULL;

        tempCV = picClerkBreakCV[i];
        delete tempCV;
        picClerkBreakCV[i] = NULL;
    }

    clerkGroupData[1].lineLock = NULL;
    clerkGroupData[1].lineCV = NULL;
    clerkGroupData[1].bribeLineCV = NULL;
    clerkGroupData[1].bribeCV = NULL;
    clerkGroupData[1].breakCV = NULL;
    clerkGroupData[1].clerkCV = NULL;
    clerkGroupData[1].clerkLock = NULL;
    clerkGroupData[1].moneyLock = NULL;
    clerkGroupData[1].clerkData = NULL;
    clerkGroupData[1].numClerks = 0;
}

void InitializePassportClerks () 
{
    Thread * t;
    char * name;

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

        ClerkFunctionStruct * clerkFunctionStruct = new ClerkFunctionStruct(2, i, (VoidFunctionPtr) PassportClerkToCustomer);

        // Create clerks
        name = new char [40];
        sprintf(name, "PassportClerk-%d", i);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)Clerk, (int) clerkFunctionStruct);
    }
    clerkGroupData[2].lineLock = &passportLineLock;
    clerkGroupData[2].lineCV = passportClerkLineCV;
    clerkGroupData[2].bribeLineCV = passportClerkBribeLineCV;
    clerkGroupData[2].bribeCV = passportClerkBribeCV;
    clerkGroupData[2].breakCV = passportClerkBreakCV;
    clerkGroupData[2].clerkCV = passportClerkCV;
    clerkGroupData[2].clerkLock = passportClerkLock;
    clerkGroupData[2].moneyLock = &passportMoneyLock;
    clerkGroupData[2].clerkData = passportClerkData;
    clerkGroupData[2].numClerks = numPassportClerks;
}

void CleanUpPassportClerks()
{
    Thread * t;
    char * name;

    Condition * tempCV;
    Lock * tempLock;

    for (int i = 0; i < numPassportClerks; i++)
    {
        tempCV = passportClerkLineCV[i];
        delete tempCV;
        passportClerkLineCV[i] = NULL;

        tempCV = passportClerkBribeLineCV[i];
        delete tempCV;
        passportClerkBribeLineCV[i] = NULL;

        tempCV = passportClerkBribeCV[i];
        delete tempCV;
        passportClerkBribeCV[i] = NULL;

        tempLock = passportClerkLock[i];
        delete tempLock;
        passportClerkLock[i] = NULL;
        
        tempCV = passportClerkCV[i];
        delete tempCV;
        passportClerkCV[i] = NULL;

        tempCV = passportClerkBreakCV[i];
        delete tempCV;
        passportClerkBreakCV[i] = NULL;
    }

    clerkGroupData[2].lineLock = NULL;
    clerkGroupData[2].lineCV = NULL;
    clerkGroupData[2].bribeLineCV = NULL;
    clerkGroupData[2].bribeCV = NULL;
    clerkGroupData[2].breakCV = NULL;
    clerkGroupData[2].clerkCV = NULL;
    clerkGroupData[2].clerkLock = NULL;
    clerkGroupData[2].moneyLock = NULL;
    clerkGroupData[2].clerkData = NULL;
    clerkGroupData[2].numClerks = 0;
}

void InitializeCashiers() 
{
    Thread * t;
    char * name;

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

        ClerkFunctionStruct * clerkFunctionStruct = new ClerkFunctionStruct(3, i, (VoidFunctionPtr) CashierToCustomer);

        // Create clerks
        name = new char [40];
        sprintf(name,"Cashier-%d",i);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)Clerk, (int)clerkFunctionStruct);
    }

    clerkGroupData[3].lineLock = &cashierLineLock;
    clerkGroupData[3].lineCV = cashierLineCV;
    clerkGroupData[3].bribeLineCV = cashierBribeLineCV;
    clerkGroupData[3].bribeCV = cashierBribeCV;
    clerkGroupData[3].breakCV = cashierBreakCV;
    clerkGroupData[3].clerkCV = cashierCV;
    clerkGroupData[3].clerkLock = cashierLock;
    clerkGroupData[3].moneyLock = &cashierMoneyLock;
    clerkGroupData[3].clerkData = cashierData;
    clerkGroupData[3].numClerks = numCashiers;
}

void CleanUpCashiers()
{
    Thread * t;
    char * name;

    Condition * tempCV;
    Lock * tempLock;

    for (int i = 0; i < numCashiers; i++)
    {
        tempCV = cashierLineCV[i];
        delete tempCV;
        cashierLineCV[i] = NULL;

        tempCV = cashierBribeLineCV[i];
        delete tempCV;
        cashierBribeLineCV[i] = NULL;

        tempCV = cashierBribeCV[i];
        delete tempCV;
        cashierBribeCV[i] = NULL;

        tempLock = cashierLock[i];
        delete tempLock;
        cashierLock[i] = NULL;
        
        tempCV = cashierCV[i];
        delete tempCV;
        cashierCV[i] = NULL;

        tempCV = cashierBreakCV[i];
        delete tempCV;
        cashierBreakCV[i] = NULL;
    }

    clerkGroupData[3].lineLock = NULL;
    clerkGroupData[3].lineCV = NULL;
    clerkGroupData[3].bribeLineCV = NULL;
    clerkGroupData[3].bribeCV = NULL;
    clerkGroupData[3].breakCV = NULL;
    clerkGroupData[3].clerkCV = NULL;
    clerkGroupData[3].clerkLock = NULL;
    clerkGroupData[3].moneyLock = NULL;
    clerkGroupData[3].clerkData = NULL;
    clerkGroupData[3].numClerks = 0;
}

void InitializeManager()
{
    Thread * t;

    t = new Thread("Manager");
    t->Fork((VoidFunctionPtr)Manager, 0);
}

void InitializeCustomers()
{
    Thread * t;
    char * name;

    for(int i = 0; i < numCustomers; i++) 
    {
        name = new char [40];
        sprintf(name, "Customer-%d", i);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)Customer, i);
    }
}























/***********************/
/******* TESTING *******/
/***********************/


/***********************/
/*  SHORTEST LINE TEST */
/*   Customers always take the shortest line, but no 2 customers */ 
/*   ever choose the same shortest line at the same time. */
/***********************/

struct DecideLineParams {
    int ssn;
    int money;
    int clerkType;

    DecideLineParams(int customerSSN, int customerMoney, int lineClerkType) {
        ssn = customerSSN;
        money = customerMoney;
        clerkType = lineClerkType;
    }
};


void ShortestLineTest_PrintResults(int numDecisionsSoFar, int numLines) {
    currentThread->Yield();

    printf(YELLOW  "\tResults of ShortestLineTest after "  MAGENTA  "%d "  YELLOW "Customers:\n"  ANSI_COLOR_RESET, numDecisionsSoFar);
    printf(YELLOW  "\t\tLine\tCount\tBribe\tState\n"  ANSI_COLOR_RESET);
    for (int i = 0; i < numLines; i++) {
        printf(MAGENTA  "\t\t  %d\t  %d\t  %d\t  %d\n"  ANSI_COLOR_RESET, i, clerkGroupData[0].clerkData[i].lineCount, clerkGroupData[0].clerkData[i].bribeLineCount, (int)clerkGroupData[0].clerkData[i].state);
    }
}

void ShortestLineTest_Customer(DecideLineParams* decideLineParamsPointer) {
    decideLineParamsPointer = (DecideLineParams *) decideLineParamsPointer;
    int ssn = decideLineParamsPointer->ssn;
    int money = decideLineParamsPointer->money;
    int clerkType = decideLineParamsPointer->clerkType;

    // Select your line, if it returns that means customer went directly to the counter.
    int myLine = DecideLine(ssn, money, clerkType);

    printf(GREEN  "Customer %d went directly to the counter for ApplicationClerk %d.\n"  ANSI_COLOR_RESET, ssn, myLine);
}

void ShortestLineTest(int numLineDecisions, int defaultMoney, int numLines, int defaultLineCount, bool useRandomClerkStates, bool useRandomLineCounts, bool useRandomMoney, ClerkStatus defaultStatus) {
    printf(WHITE  "\n\nShortest Line Test"  ANSI_COLOR_RESET  "\n");
    
    printf(YELLOW  "Assumptions about Customer's Line Decisions:\n"  ANSI_COLOR_RESET);
    printf(CYAN  "\t1.  Customers always go straight to the counter if the Clerk is (i) AVAILABLE and (ii) its line is empty (it always should be if he is AVAILABLE).\n"  ANSI_COLOR_RESET);
    printf(CYAN  "\t2.  Customers decide their clerk based ONLY on the length of the regular line.\n"  ANSI_COLOR_RESET);
    printf(CYAN  "\t3.  Customers always bribe if they have enough money (>= $600).\n"  ANSI_COLOR_RESET);
    printf(CYAN  "\t4.  Customers will join a long bribe line if the clerk's regular line is shortest. (Restatement of Assumption 2)\n\n"  ANSI_COLOR_RESET);

    InitializeData();
    InitializeAppClerks();
    
    int clerkType = 0;
    int clerkState = (int)defaultStatus;
    numAppClerks = numLines;

    // Initialize lines with lineCounts
    for (int i = 0; i < numLines; i++) 
    {

        if (useRandomClerkStates) { clerkState = rand() % 3; }
        clerkGroupData[clerkType].clerkData[i].state = (ClerkStatus)clerkState;

        if (useRandomLineCounts) 
        { 
            if (clerkGroupData[clerkType].clerkData[i].state == AVAILABLE) {
                clerkGroupData[clerkType].clerkData[i].lineCount = 0;
            } else {
                clerkGroupData[clerkType].clerkData[i].lineCount = rand() % 10;
            }
        }

        else { clerkGroupData[clerkType].clerkData[i].lineCount = defaultLineCount; }
    }



    printf(MAGENTA  "\nInitial line conditions: \n"  ANSI_COLOR_RESET);
    printf(YELLOW  "\tNumber of customers: "  MAGENTA  "%d\n"  ANSI_COLOR_RESET, numLineDecisions);
    printf(YELLOW  "\tNumber of lines: "  MAGENTA  "%d\n"  ANSI_COLOR_RESET, numLines);
    ShortestLineTest_PrintResults(0, numLines);

    for (int i = 0; i < numLineDecisions; i++) 
    {

        int money = defaultMoney;

        if (useRandomMoney) {
            int RandIndex = rand() % 4;
            money = MoneyOptions[RandIndex];
        }

        char * name = new char [40];
        sprintf(name, "Customer-%d", i);
        Thread * t = new Thread(name);

        DecideLineParams * decideLineParams = new DecideLineParams(i, money, clerkType);
        t->Fork((VoidFunctionPtr)ShortestLineTest_Customer, (int)decideLineParams);
    
        // Print results with every 5 customers
        ShortestLineTest_PrintResults(i + 1, numLines);
    }
}


/***********************/
/* PASSPORT THIRD TEST */
/*   Customers do not leave until they are given their passport by the Cashier. */ 
/*   The Cashier does not start on another customer until they know that the last */
/*   Customer has left their area. */
/***********************/
Semaphore ClerksGoOnBreak_Semaphore("ClerksGoOnBreak_Semaphore", 0);
Semaphore CashierTest_Semaphore("CashierTest_Semaphore", 0);

//needs to show the cashier yields until the customer signals that it is leaving
//needs to show the customer does not leave the passport office until the cashier gave the passport.
//must show that the customer's app hasn't been filed/the customers picture hasn't been filed.

void CashierTest_Customer(int ssn){
    int RandIndex = rand() % 4;
    int money = MoneyOptions[0];
    int num = DecideLine(ssn, money, 3);
    CustomerToCashier(ssn, money, num);
    Leave(ssn);
    CashierTest_Semaphore.P();

}

void CashierTest(int defaultMoney, int numCashier, int numCustomer, ClerkStatus defaultStatus){
    
    printf(WHITE  "\n\nCashier Test"  ANSI_COLOR_RESET  "\n");
    printf(YELLOW  "\tNumber of customers: "  MAGENTA  "%d"  ANSI_COLOR_RESET  "\n", numCustomer);
    printf(YELLOW  "\tNumber of cashiers: "  MAGENTA  "%d"  ANSI_COLOR_RESET  "\n", numCashier);

    InitializeData();
    InitializeCashiers();


    for (int i = 0; i < numCustomer; i++) 
    {
        char * name = new char [40];
        sprintf(name, "Customer-%d", i);
        Thread * t = new Thread(name);
        t->Fork((VoidFunctionPtr)CashierTest_Customer, i);
        printf("forked thread %d\n", i);
        customerData[i].passportCertified=false;
    }
    for (int i = 0; i < 2000; i++)
    {
        currentThread->Yield();
    }
    for (int i=0; i<numCustomer; i++){
        printf("Setting customer %d passport certified to true.\n", i);
        customerData[i].passportCertified=true;
    }

    CashierTest_Semaphore.P();

    //currentThread->Yield();
}

void ClerksGoOnBreak_Customer(int i)
{
    int money = 100;

    int currentLine = DecideLine(i, money, 0);
    CustomerToApplicationClerk(i, currentLine);
    ClerksGoOnBreak_Semaphore.V();
}

void ClerksGoOnBreak() 
{
    numCustomers = 1;
    numAppClerks = 1;

    InitializeData();
    InitializeAppClerks();

    Thread * t;
    char * name;

    name = new char [40];
    sprintf(name, "Customer-%d", 0);
    t = new Thread(name);
    t->Fork((VoidFunctionPtr)ClerksGoOnBreak_Customer, 0);

    ClerksGoOnBreak_Semaphore.P();

    // Make sure nobody is in line
    ASSERT(appClerkData[0].lineCount == 0);
    // Make sure clerk is on break
    ASSERT(appClerkData[0].state == ONBREAK);
    

    //CleanUpAppClerks();
}

Semaphore ManagerTakesClerkOffBreak_Semaphore("ManagerTakesClerkOffBreak_Semaphore", 0);

void ManagerTakesClerkOffBreak_MultipleCustomers(int i)
{
    int money = 100;
    int currentLine = DecideLine(i, money, 0);
    CustomerToApplicationClerk(i, currentLine);
    ManagerTakesClerkOffBreak_Semaphore.V();
}

void ManagerTakesClerkOffBreak_SingleCustomer(int i)
{
    int money = 100;
    int currentLine = DecideLine(i, money, 0);
    ManagerTakesClerkOffBreak_Semaphore.V();
    CustomerToApplicationClerk(i, currentLine);
}



void ManagerTakesClerkOffBreak()
{
    numCustomers = 6;
    numAppClerks = 2;
    numPicClerks = 1;
    numPassportClerks = 1;
    numCashiers = 1;

    InitializeData();

    InitializeAppClerks();

    InitializePicClerks();

    InitializePassportClerks();

    InitializeCashiers();

    InitializeManager();

    Thread * t;
    char * name;

    name = new char [40];
    sprintf(name, "Customer-%d", 0);
    t = new Thread(name);
    t->Fork((VoidFunctionPtr)ManagerTakesClerkOffBreak_MultipleCustomers, 0);
    
    ManagerTakesClerkOffBreak_Semaphore.P();

    // Make sure nobody is in line 0
    ASSERT(appClerkData[0].lineCount == 0);
    // Make sure clerk 0 is on break
    ASSERT(appClerkData[0].state == ONBREAK);

    for(int i = 1; i < 5; i++)
    {
        name = new char [40];
        sprintf(name, "Customer-%d", i);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)ManagerTakesClerkOffBreak_MultipleCustomers, i);
    }

    ManagerTakesClerkOffBreak_Semaphore.P();

    // Make sure clerk 0 is available
    ASSERT(appClerkData[0].state == AVAILABLE);

    for(int i = 0; i < 3; i++)
    {
        ManagerTakesClerkOffBreak_Semaphore.P();
    }

    ASSERT(appClerkData[0].state == ONBREAK);
    ASSERT(appClerkData[1].state == ONBREAK);

    name = new char [40];
    sprintf(name, "Customer-%d", 0);
    t = new Thread(name);
    t->Fork((VoidFunctionPtr)ManagerTakesClerkOffBreak_SingleCustomer, 0);
    
    ManagerTakesClerkOffBreak_Semaphore.P();

    ASSERT(appClerkData[0].state == BUSY);
    numCustomersFinished = numCustomers;
}

Semaphore ManagerCountsMoney_CashierSemaphore("ManagerCountsMoney_CashierSemaphore", 0);
Semaphore ManagerCountsMoney_AppClerkSemaphore("ManagerCountsMoney_AppClerkSemaphore", 0);

void ManagerCountsMoney_CashierCustomer(int i)
{
    int money = 100;
    int currentLine = DecideLine(i, money, 3);
    CustomerToCashier(i, money, currentLine);
    ManagerCountsMoney_CashierSemaphore.V();
}

void ManagerCountsMoney_AppClerkCustomer(int i)
{
    int money = 600;
    int currentLine = DecideLine(i, money, 0);
    ManagerCountsMoney_AppClerkSemaphore.V();
    CustomerToApplicationClerk(i, currentLine);
    
}


void ManagerCountsMoney()
{
    numCustomers = 4;
    numAppClerks = 1;
    numPicClerks = 1;
    numPassportClerks = 1;
    numCashiers = 1;

    InitializeData();

    InitializeAppClerks();

    InitializePicClerks();

    InitializePassportClerks();

    InitializeCashiers();

    InitializeManager();

    Thread * t;
    char * name;

    customerData[0].passportCertified = true;

    name = new char [40];
    sprintf(name, "Customer-%d", 0);
    t = new Thread(name);
    t->Fork((VoidFunctionPtr)ManagerCountsMoney_CashierCustomer, 0);

    for (int i = 1; i < 4; i++)
    {
        name = new char [40];
        sprintf(name, "Customer-%d", i);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)ManagerCountsMoney_AppClerkCustomer, i);
    }
    
    ManagerCountsMoney_CashierSemaphore.P();
    ASSERT(managerData.cashierMoney == 100);
    ASSERT(managerData.appClerkMoney == 0);

    for(int i = 0; i < 3; i++)
    {
        ManagerCountsMoney_AppClerkSemaphore.P();
    }

    ASSERT(managerData.appClerkMoney == 500);

    numCustomersFinished = numCustomers;
}

void Test2()
{
    //ShortestLineTest(5, false, 100, 3, false, 0, false, AVAILABLE); // 5 Customers, 3 Lines, $100 (no bribes), All clerks begin AVAILABLE

    //ClerksGoOnBreak();
    //ManagerTakesClerkOffBreak();

    ManagerCountsMoney();
}




void Part2()
{
    GetInput();

    InitializeData();

    //  POLISH: If we have time, below could be done in two nested for loops.

    //  ================================================
    //      Application Clerks
    //  ================================================

    InitializeAppClerks();

    //  ================================================
    //      Picture Clerks
    //  ================================================

    InitializePicClerks();
    

    //  ================================================
    //      Passport Clerks
    //  ================================================

    InitializePassportClerks();

    //  ================================================
    //      Cashiers
    //  ================================================

    InitializeCashiers();

    //  ================================================
    //      Managers
    //  ================================================

    InitializeManager();

    //  ================================================
    //      Customers
    //  ================================================

    InitializeCustomers();

    for(int i = 0; i < numCustomers; i++)
    {
        CustomersFinished.P();
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
