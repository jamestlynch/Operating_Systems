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

const int moneyOptions[4] = {100, 600, 1100, 1600};



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
    int bribeMoney;
    int currentCustomer;
    ClerkStatus state;

    ClerkData() 
    {
        lineCount = 0;
        bribeLineCount = 0;
        bribeMoney = 0;
        currentCustomer = -1;
        state = AVAILABLE;
    }
};

struct ManagerData 
{
    int money;// = 0;
};
struct FilingJob
{
    int ssn;
    int lineNumber;
    FilingJob(int tempssn, int templineNumber)
    {
        ssn=tempssn;
        lineNumber=templineNumber;
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

Condition ** appClerkBreakCV;
Condition ** picClerkBreakCV;
Condition ** passportClerkBreakCV;
Condition ** cashierBreakCV;

CustomerData * customerData;
ClerkData * appClerkData;
ClerkData * passportClerkData;
ClerkData * picClerkData;
ClerkData * cashierData;
ManagerData managerData;

int numCustomers = 4;
int numAppClerks = 1;
int numPicClerks = 1;
int numPassportClerks = 1;
int numCashiers = 1;
int numSenators = 1;



// Monitors

struct LineDecisionMonitor {
    Lock * lineLock;
    Condition ** lineCV;
    Condition ** breakCV;
    ClerkData * clerkData;
    int numClerks;

    LineDecisionMonitor() {}
};

LineDecisionMonitor * lineDecisionMonitors;



























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
    printf(MAGENTA  "filingTime = %d" ANSI_COLOR_RESET "\n", filingTime);
    for (int i = 0; i < filingTime; i++)
    {
        currentThread->Yield();
        //printf(MAGENTA  "FilingApplication back in scheduler %s"  ANSI_COLOR_RESET  "\n", currentThread->getName());
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
        appLineLock.Acquire();
        //if (ClerkBribeLineCount[myLine] > 0)
        //clerkBribeLineCV[myLine]->Signal(applicationClerksLineLock);
        //appClerkData[lineNumber].state = BUSY;
        //else
        if (appClerkData[lineNumber].lineCount > 0) 
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
    printf(MAGENTA  "filingTime = %d" ANSI_COLOR_RESET "\n", filingTime);
    for (int i = 0; i < filingTime; i++)
    {
        currentThread->Yield();
        //printf(MAGENTA  "FilingPicture back in scheduler %s"  ANSI_COLOR_RESET  "\n", currentThread->getName());
    }
    filingPictureLock.Acquire();
    customerData[jobPointer->ssn].photoFiled = true;
    printf(GREEN  "PictureClerk %d has recorded a filed picture for Customer %d"  ANSI_COLOR_RESET  "\n", jobPointer->lineNumber, jobPointer->ssn);
    filingPictureLock.Release();
}

void CustomerToPictureClerk(int ssn, int myLine)
{
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
        customerData[ssn].acceptedPicture = true;
        printf(GREEN  "Customer %d does like their picture from PictureClerk %d."  ANSI_COLOR_RESET  "\n", ssn, myLine);
        currentThread->Yield();
        FilingJob * pictureFiling = new FilingJob(ssn, myLine);
        Thread * t = new Thread("PictureFilingThread");
        t->Fork((VoidFunctionPtr)filePicture, (int)pictureFiling);
        int filingTime = (rand() % 80) + 20;
        //printf(MAGENTA  "filingTime = %d" ANSI_COLOR_RESET "\n", filingTime);
        for (int i = 0; i < filingTime; i++)
        {
            currentThread->Yield();
            //printf(MAGENTA  "FilingApplication back in scheduler %s"  ANSI_COLOR_RESET  "\n", currentThread->getName());
        }
    }
    else
    {
        printf(GREEN  "Customer %d does not like their picture from PictureClerk %d."  ANSI_COLOR_RESET  "\n", ssn, myLine);
    }
    picClerkCV[myLine]->Signal(picClerkLock[myLine]); //leaving
    picClerkLock[myLine]->Release();
    
    if (!customerData[ssn].acceptedPicture)
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
        picLineLock.Acquire();
        //if (ClerkBribeLineCount[myLine] > 0)
        //clerkBribeLineCV[myLine]->Signal(applicationClerksLineLock);
        picClerkData[lineNumber].state = BUSY;
        /*else*/

        if (picClerkData[lineNumber].lineCount > 0) 
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
        printf(MAGENTA  "release 1"  ANSI_COLOR_RESET  "\n");
        filingApplicationLock.Release();
        filingPictureLock.Release();
        //customer went to counter too soon,
        //wait an arbitrary amount of time
        int filingTime = (rand() % 80) + 20;
        //printf(MAGENTA  "filingTime = %d" ANSI_COLOR_RESET "\n", filingTime);
        for (int i = 0; i < filingTime; i++)
        {
            currentThread->Yield();
            //printf(MAGENTA  "FilingApplication back in scheduler %s"  ANSI_COLOR_RESET  "\n", currentThread->getName());
        }


        //go to the back of the line and wait again
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

    printf(MAGENTA  "release 2"  ANSI_COLOR_RESET  "\n");
    filingApplicationLock.Release();
    filingPictureLock.Release();
            //thread yield until passportcertification
            //customer leaves co
            //customer gets on line for cashier
  
    passportClerkCV[myLine]->Signal(passportClerkLock[myLine]); //leaving
    printf(MAGENTA  "%s done with passport clerk"  ANSI_COLOR_RESET  "\n", currentThread->getName());
    passportClerkLock[myLine]->Release();
}



/*void certifyApplication(int ssn, int lineNumber){
    int filingTime = (rand() % 1000) + 100;
    for (int i = 0; i < filingTime; i++)
    {
        currentThread->Yield();
    }
    CustomerData[ssn].passportCertified=true;
}*/

void PassportClerkToCustomer(int lineNumber)
{
    passportClerkLock[lineNumber]->Acquire(); // acquire the lock for my line to pause time.
    passportLineLock.Release(); //clerk must know a customer left before starting over
    passportClerkCV[lineNumber]->Wait(passportClerkLock[lineNumber]);
    printf(GREEN  "PassportClerk %d has received SSN %d from Customer %d"  ANSI_COLOR_RESET  "\n", lineNumber, passportClerkData[lineNumber].currentCustomer, passportClerkData[lineNumber].currentCustomer);
    passportClerkCV[lineNumber]->Signal(passportClerkLock[lineNumber]);
    passportClerkData[lineNumber].currentCustomer = -1;
    passportClerkCV[lineNumber]->Wait(passportClerkLock[lineNumber]);
    passportClerkLock[lineNumber]->Release();

}
void PassportClerk(int lineNumber){
    passportLineLock.Acquire();
    PassportClerkToCustomer(lineNumber);
    printf(MAGENTA  "PassportClerk checks his lines."  ANSI_COLOR_RESET  "\n");
    while (true)
        {
            passportLineLock.Acquire();
            //if (ClerkBribeLineCount[myLine] > 0)
            //clerkBribeLineCV[myLine]->Signal(applicationClerksLineLock);
            passportClerkData[lineNumber].state = BUSY;
            /*else*/

            if (passportClerkData[lineNumber].lineCount > 0) 
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




























void CustomerToCashier(int ssn, int myLine)
{
    //int ssn = 0;

    cashierLock[myLine]->Acquire();//simulating the line
    cashierCV[myLine]->Signal(cashierLock[myLine]);

    cashierData[myLine].currentCustomer = ssn;
    printf(GREEN  "Customer %d has given SSN %d to Cashier %d."  ANSI_COLOR_RESET  "\n", ssn, ssn, myLine);

    cashierCV[myLine]->Wait(cashierLock[myLine]);

    //MAKE SURE MONEY IS DECREMENTED AT RIGHT TIME.
     
    if(!customerData[ssn].passportCertified){
            //customer went to cashier too soon,
            //wait an arbitrary amount of time
            
            //TO DO: PUNISHMENT OF 100 to 1000 currentThread->Yield() calls
            //go to the back of the line and wait again
            cashierLineLock.Acquire();
            cashierData[myLine].lineCount++; // rejoin the line
            printf(GREEN  "Customer %d has gone to Cashier %d too soon. They are going to the back of the line."  ANSI_COLOR_RESET  "\n", ssn, myLine);
            cashierLineCV[myLine]->Wait(&cashierLineLock); // Waiting in line
            // Reacquires lock after getting woken up inside Wait.
            cashierData[myLine].lineCount--; // Leaving the line, going to the counter
            cashierLineLock.Release();
            CustomerToCashier(ssn, myLine);
            }
            customerData[ssn].money -= 100;
            //thread yield until passportcertification
            //customer leaves counter
            //customer gets on line for cashier
            cashierCV[myLine]->Signal(cashierLock[myLine]); //leaving
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
void CashierToCustomer(int lineNumber){
    cashierLock[lineNumber]->Acquire(); // acquire the lock for my line to pause time.
    cashierLineLock.Release(); //clerk must know a customer left before starting over
    cashierCV[lineNumber]->Wait(cashierLock[lineNumber]);
    printf(GREEN  "Cashier %d has received SSN %d from Customer %d"  ANSI_COLOR_RESET  "\n", lineNumber, cashierData[lineNumber].currentCustomer, cashierData[lineNumber].currentCustomer);
    printf(GREEN  "Cashier %d has taken $100 from Customer %d."  ANSI_COLOR_RESET  "\n", lineNumber, cashierData[lineNumber].currentCustomer);
    cashierCV[lineNumber]->Signal(cashierLock[lineNumber]);
    cashierData[lineNumber].currentCustomer = -1;
    cashierCV[lineNumber]->Wait(cashierLock[lineNumber]);
    cashierLock[lineNumber]->Release();
}
void Cashier(int lineNumber){
    cashierLineLock.Acquire();
    CashierToCustomer(lineNumber);
    while (true)
    {
        cashierLineLock.Acquire();
        //if (ClerkBribeLineCount[myLine] > 0)
        //clerkBribeLineCV[myLine]->Signal(applicationClerksLineLock);
        cashierData[lineNumber].state = BUSY;

        if (cashierData[lineNumber].lineCount > 0) 
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
            switch(clerkType) 
            {
                case 0: printf(GREEN  "Manager has woken up ApplicationClerk %d."  ANSI_COLOR_RESET  "\n", i); break;
                case 1: printf(GREEN  "Manager has woken up PictureClerk %d."  ANSI_COLOR_RESET  "\n", i); break;
                case 2: printf(GREEN  "Manager has woken up PassportClerk %d."  ANSI_COLOR_RESET  "\n", i); break;
                case 3: printf(GREEN  "Manager has woken up Cashier %d"  ANSI_COLOR_RESET  "\n", i); break;
            }
            continue;
        }

        // If a clerk is on break, and another clerk has 3+ people in their line, wake up that clerk
        if(clerkData[i].state == ONBREAK && wakeUpClerk > 0) 
        {
            wakeUpClerk--;

            clerkData[i].state = AVAILABLE;
            breakCV[i]->Signal(lineLock);
            switch(clerkType) 
            {
                case 0: printf(GREEN  "Manager has woken up ApplicationClerk %d."  ANSI_COLOR_RESET  "\n", i); break;
                case 1: printf(GREEN  "Manager has woken up PictureClerk %d."  ANSI_COLOR_RESET  "\n", i); break;
                case 2: printf(GREEN  "Manager has woken up PassportClerk %d."  ANSI_COLOR_RESET  "\n", i); break;
                case 3: printf(GREEN  "Manager has woken up Cashier %d"  ANSI_COLOR_RESET  "\n", i); break;
            }
        }
    }
    lineLock->Release();

    return clerkMoney;
}

// TODO: add a method for each lock that exists between passport clerks and X
// TODO: Change clerksLineLock to clerkLineLock[i]
void Manager()
{
    // Wakes up the clerks when there are >3 people waiting
    // Counts each clerks money
    while(true) 
    {
        int appClerkMoney = ManageClerk(0);
        int picClerkMoney = ManageClerk(1);
        int passportClerkMoney = ManageClerk(2);
        int cashierMoney = 0; //ManageClerk(3);
        int totalMoney = appClerkMoney + picClerkMoney + passportClerkMoney + cashierMoney;

        printf(GREEN  "Manager has counted a total of $%d for ApplicationClerks."  ANSI_COLOR_RESET  "\n", appClerkMoney);
        printf(GREEN  "Manager has counted a total of $%d for PictureClerks."  ANSI_COLOR_RESET  "\n", picClerkMoney);
        printf(GREEN  "Manager has counted a total of $%d for PassportClerks."  ANSI_COLOR_RESET  "\n", passportClerkMoney);
        printf(GREEN  "Manager has counted a total of $%d for Cashier."  ANSI_COLOR_RESET  "\n", cashierMoney);
        printf(GREEN  "Manager has counted a total of $%d for the passport office."  ANSI_COLOR_RESET  "\n", totalMoney);

        currentThread->Yield();
    }
}
































void Senator()
{

}

void getInput()
{
    



    printf(MAGENTA  "Number of Customers = %d"  ANSI_COLOR_RESET  "\n", numCustomers);
    printf(MAGENTA  "Number of ApplicationClerks = %d"  ANSI_COLOR_RESET  "\n", numAppClerks);
    printf(MAGENTA  "Number of PictureClerks = %d"  ANSI_COLOR_RESET  "\n", numPicClerks);
    printf(MAGENTA  "Number of PassportClerks = %d"  ANSI_COLOR_RESET  "\n", numPassportClerks);
    printf(MAGENTA  "Number of Cashiers = %d"  ANSI_COLOR_RESET  "\n", numCashiers);
    printf(MAGENTA  "Number of Senators = %d"  ANSI_COLOR_RESET  "\n", numSenators);
}






























void DecideLine(int ssn, int money, int clerkType) 
{
    Lock * lineLock = lineDecisionMonitors[clerkType].lineLock;
    ClerkData * clerkData = lineDecisionMonitors[clerkType].clerkData;
    Condition ** lineCV = lineDecisionMonitors[clerkType].lineCV;
    Condition ** bribeLineCV = lineDecisionMonitors[clerkType].lineCV;
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
    if (clerkData[currentLine].state == BUSY || clerkData[currentLine].state == ONBREAK)// ApplicationClerk is not available, so wait in line
    {
        // Decide if we want to bribe the clerk
        if(clerkData[currentLine].lineCount >= 1 && money >= 600)
        {
            clerkData[currentLine].bribeLineCount++;

            switch(clerkType) 
            {
                case 0: printf(GREEN  "Customer %d has gotten in bribe line for ApplicationClerk %d."  ANSI_COLOR_RESET  "\n", ssn, currentLine); break;
                case 1: printf(GREEN  "Customer %d has gotten in bribe line for PictureClerk %d."  ANSI_COLOR_RESET  "\n", ssn, currentLine); break;
                case 2: printf(GREEN  "Customer %d has gotten in bribe line for PassportClerk %d."  ANSI_COLOR_RESET  "\n", ssn, currentLine); break;
                case 3: printf(GREEN  "Customer %d has gotten in bribe line for Cashier %d."  ANSI_COLOR_RESET  "\n", ssn, currentLine); break;
            }

            bribeLineCV[currentLine]->Wait(lineLock);
            clerkData[currentLine].bribeLineCount--;

        }
        else 
        {
            // Join the line
            clerkData[currentLine].lineCount++; 

            switch(clerkType) 
            {
                case 0: printf(GREEN  "Customer %d has gotten in regular line for ApplicationClerk %d."  ANSI_COLOR_RESET  "\n", ssn, currentLine); break;
                case 1: printf(GREEN  "Customer %d has gotten in regular line for PictureClerk %d."  ANSI_COLOR_RESET  "\n", ssn, currentLine); break;
                case 2: printf(GREEN  "Customer %d has gotten in regular line for PassportClerk %d."  ANSI_COLOR_RESET  "\n", ssn, currentLine); break;
                case 3: printf(GREEN  "Customer %d has gotten in regular line for Cashier %d."  ANSI_COLOR_RESET  "\n", ssn, currentLine); break;
            }

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
        case 3: CustomerToCashier(ssn, currentLine); break;
    }
}

void Customer(int ssn) 
{
    int RandIndex = rand() % 4;
    int money = moneyOptions[RandIndex];

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
    //DecideLine(ssn, 3); // clerkType = 3 = Cashier
    //Leave();
}

/*
    semaphore(numClerks)

    semaphore.V();
    
    while(true) {
        for(int i = 0; i < numClerks; i++) {
            semaphore.V();
        }

        customer(senatorID);
    }
    semaphore.V(numCler)
*/


































void Part2()
{
    getInput();

    Thread *t;
    char *name;

    appClerkCV = new Condition*[numAppClerks];
    picClerkCV = new Condition*[numPicClerks];
    passportClerkCV = new Condition*[numPassportClerks];
    
    appClerkLineCV = new Condition*[numAppClerks];
    picClerkLineCV = new Condition*[numPicClerks];
    passportClerkLineCV = new Condition*[numPassportClerks];
    cashierLineCV = new Condition*[numCashiers];

    appClerkBribeLineCV = new Condition*[numAppClerks];
    picClerkBribeLineCV = new Condition*[numPicClerks];
    passportClerkBribeLineCV = new Condition*[numPassportClerks];
    cashierBribeLineCV = new Condition*[numCashiers];
    
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
    lineDecisionMonitors[0].breakCV = appClerkBreakCV;
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
    lineDecisionMonitors[1].breakCV = picClerkBreakCV;
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
    lineDecisionMonitors[2].breakCV = passportClerkBreakCV;
    lineDecisionMonitors[2].clerkData = passportClerkData;
    lineDecisionMonitors[2].numClerks = numPassportClerks;

    //  ================================================
    //      Cashiers
    //  ================================================

    // for(int i = 0; i < numCashiers; i++) 
    // {
    //     // Create Locks and CVs tied to Clerk
    //     name = new char [40];
    //     sprintf(name, "LineCV-Cashier-%d", i);
    //     cashierLineCV[i] = new Condition(name);
        
    //     name = new char [40];
    //     sprintf(name, "Lock-Cashier-%d", i);
    //     cashierLock[i] = new Lock(name);
        
    //     name = new char [40];
    //     sprintf(name, "WorkCV-CashierCV-%d", i);
    //     cashierCV[i] = new Condition(name);
            
    //     name = new char [40];
    //     sprintf(name, "BreakCV-Cashier-%d", i);
    //     cashierBreakCV[i] = new Condition(name);

    //     // Create clerks
    //     name = new char [40];
    //     sprintf(name,"Cashier-%d",i);
    //     t = new Thread(name);
    //     t->Fork((VoidFunctionPtr)Cashier, i);
    // }

    // lineDecisionMonitors[3].lineLock = &cashierLineLock;
    // lineDecisionMonitors[3].lineCV = cashierLineCV;
    // lineDecisionMonitors[3].bribeLineCV = cashierBribeLineCV;
    // lineDecisionMonitors[3].breakCV = cashierBreakCV;
    // lineDecisionMonitors[3].clerkData = cashierData;
    // lineDecisionMonitors[3].numClerks = numCashiers;

    //  ================================================
    //      Managers
    //  ================================================


   // t = new Thread("Manager");
    //t->Fork((VoidFunctionPtr)Manager, 0);
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
bribe lines
yields- implement for all clerk processing
passport
cashier
write all the tests
on break
user input menu
write up

*/

