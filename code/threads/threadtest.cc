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
const char money[4] = {'100', '600', '1100', '1600'};


struct CustomerData 
{
    int money;
    bool turnedInApplication;// = false;
    bool acceptedPicture;// = false;
    bool gotPassport;// = false;
    bool filed;
    CustomerData(){
        //initialize money value of 
        turnedInApplication=false;
        acceptedPicture=false;
        gotPassport=false;
        filed=false; //this needs to become true when gotPassport=true, acceptedPicture=true, and yield happens for a random amt of time
        int RandIndex = rand() % 4;
        money= money[RandIndex];
    }
};

struct ApplicationClerkData 
{
    int lineCount;// = 0;
    int bribeMoney;
    int currentCustomer;
    ClerkStatus state;

    ApplicationClerkData() 
    {
        //state = AVAILABLE;
        lineCount = 0;
        bribeMoney=0;
        currentCustomer=-1;
        state=AVAILABLE;
    }
};

struct PictureClerkData 
{
    int lineCount;// = 0
    int bribeMoney;
    int currentCustomer;
    ClerkStatus state;
    PictureClerkData()
    {
        lineCount = 0;
        bribeMoney=0;
        currentCustomer=-1;
        state=AVAILABLE;
    }
};

struct PassportClerkData
{
    int lineCount;// = 0;
    int bribeMoney;
    int currentCustomer;
    ClerkStatus state;
    PassportClerkData()
    {
        lineCount = 0;
        bribeMoney=0;
        currentCustomer=-1;
        state=AVAILABLE;
    }
};

struct CashierData
{
    int lineCount;// = 0
    int currentCustomer;
    ClerkStatus state;
    CashierData()
    {
        lineCount = 0;
        bribeMoney=0;
        currentCustomer=-1;
        state=AVAILABLE;
    }
};

struct ManagerData 
{
    int money;// = 0;
};

Condition ** appClerkCV;
Condition ** picClerkCV;
Condition ** passportClerkCV;

/* CONDITION VARIABLES FOR WAITING ON CLERKS' LINE */
Condition ** appClerkLineCV;
Condition ** picClerkLineCV;
Condition ** passportClerkLineCV;
Condition ** cashierLineCV;

/* LOCKS ON CLERK */
Lock ** appClerkLock;
Lock ** picClerkLock;
Lock ** passportClerkLock;

/* LOCKS ON LINE */
Lock appLineLock("applicationLineLock");
Lock picLineLock("applicationLineLock");
Lock passportLineLock("applicationLineLock");
Lock cashierLineLock("cashierLineLock");

Condition ** appClerkBreakCV;
Condition ** picClerkBreakCV;
Condition ** passportClerkBreakCV;

CustomerData * customerData;
ApplicationClerkData * appClerkData;
PassportClerkData * passportClerkData;
PictureClerkData * picClerkData;
CashierData * cashierData;
ManagerData managerData;

int numAppClerks = 1;
int numPicClerks = 5;
int numPassportClerks = 5;
int numCustomers = 2;
int numCashiers = 5;

/*
    Simulates customer behavior:
    –   Whether to pick application or picture clerk
*/
void CustomerToApplicationClerk(int ssn, int myLine)
{
    appClerkLock[myLine]->Acquire();
    //Give my data to my clerk
    printf("Customer %d has given SSN %d to ApplicationClerk %d.\n", ssn, ssn, myLine);
    appClerkData[myLine].currentCustomer = ssn;
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

void DecideApplicationLine(int ssn) 
{
    // TODO: CHANGE THE WAY PEOPLE CHOOSE LINES
    // CS: Need to check the state of all application clerks' lines without them changing
    appLineLock.Acquire();
    
    int myLine = -1; // No line yet
    int lineSize = 1000; // Larger (bc we're finding shortest line) than the number customers created
    
    // What if everyone's on break?
    int longestLine = -1; // Store the longest line (Once a single line has >= 3 Customers, Manager wakes up an ApplicationClerk)
    int longestLineSize = -1; // Smaller than any line could possibly be because we are searching for longest line.

    for (int i = 0; i < numAppClerks; i++) //number of clerks
    {
        // Pick the shortest line with a clerk not on break
        if (appClerkData[i].lineCount < lineSize && appClerkData[i].state != ONBREAK)
        {
            myLine = i;
            lineSize = appClerkData[i].lineCount;
            //even if line size = 0, the clerk could still be busy since being at the counter is not                                                             ‘in line'
        }

        // What if everyones on break?
        // Keep track of the longest line
        /*if (appClerkData[i].lineCount > longestLineSize) 
        {
            longestLine = i;
        }

        // What if everyones on break?
        // Join the longest line and wait for Manager to wake up an Application Clerk (once this line gets at least 3 Customers)
        if (i == 4 && myLine == -1) 
        { // If this is the last ApplicationClerk(number of clerks -1) and we haven't picked a line
            myLine = longestLine; // Join the longest line
            lineSize = appClerkData[i].lineCount;
        }*/
    }

    // I've selected a line...
    //if (appClerkData[myLine].state == BUSY || appClerkData[myLine].state == ONBREAK)// ApplicationClerk is not available, so wait in line
    //{ 
        appClerkData[myLine].lineCount++; // Join the line
        printf("Customer %d has gotten in regular line for ApplicationClerk %d.\n", ssn, myLine);
        appClerkLineCV[myLine]->Wait(&appLineLock); // Waiting in line
        // Reacquires lock after getting woken up inside Wait.
        appClerkData[myLine].lineCount--; // Leaving the line
    /*} 
    else // Line was empty to begin with. Clerk is avail
    { 
        printf("Customer %d has gotten in regular line for ApplicationClerk %d.\n", ssn, myLine);
        appClerkData[myLine].state = BUSY;
    }*/

    appLineLock.Release();
    CustomerToApplicationClerk(ssn, myLine);
}

void ApplicationClerkToCustomer(int lineNumber)
{
    appClerkLock[lineNumber]->Acquire(); // acquire the lock for my line to pause time.
    appLineLock.Release(); // clerk must know a customer left before starting over
    appClerkCV[lineNumber]->Wait(appClerkLock[lineNumber]);
    printf("ApplicationClerk %d has received SSN %d from Customer %d\n", lineNumber, appClerkData[lineNumber].currentCustomer, appClerkData[lineNumber].currentCustomer);
    // do my job - customer nowwaiting
    currentThread->Yield();
    printf("ApplicationClerk %d has recorded a completed application for Customer %d\n", lineNumber, appClerkData[lineNumber].currentCustomer);
    appClerkCV[lineNumber]->Signal(appClerkLock[lineNumber]);
    appClerkCV[lineNumber]->Wait(appClerkLock[lineNumber]);
    appClerkLock[lineNumber]->Release();
}

void ApplicationClerk(int lineNumber)
{
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
            printf("ApplicationClerk %d has signalled a Customer to come to their counter\n", lineNumber);
            appClerkLineCV[lineNumber]->Signal(&appLineLock);
            appClerkData[lineNumber].state = BUSY;
            ApplicationClerkToCustomer(lineNumber);
        }
        else
        {
            printf("ApplicationClerk %d is going on break\n", lineNumber);
            // nobody is waiting
            appClerkData[lineNumber].state = ONBREAK;
            appClerkBreakCV[lineNumber]->Wait(&appLineLock);
            // Go on break.
        }
    }
}

void CustomerToPictureClerk(int ssn, int myLine)
{
    //customer just got to window, wake up, wait to take picture
    picClerkLock[myLine]->Acquire();//simulating the line
    picClerkCV[myLine]->Signal(picClerkLock[myLine]);//take my picture
    picClerkCV[myLine]->Wait(picClerkLock[myLine]); //waiting for you to take my picture
    if ( rand() < .5 )
    {
        customerData[ssn].acceptedPicture = true;
        printf("Customer %d does like their picture from PictureClerk %d.\n", ssn, myLine);
    }
    else
    {
        printf("Customer %d does not like their picture from PictureClerk %d.\n", ssn, myLine);
    }
    picClerkCV[myLine]->Signal(picClerkLock[myLine]); //leaving
    picClerkLock[myLine]->Release();
    
    if (!customerData[ssn].acceptedPicture)
    {
        picLineLock.Acquire();
        // ApplicationClerk is not available, so wait in line
        picClerkData[myLine].lineCount++; // Join the line
        printf("Customer %d has gotten in regular line for PictureClerk %d.\n", ssn, myLine);
        picClerkLineCV[myLine]->Wait(&picLineLock); // Waiting in line
        // Reacquires lock after getting woken up inside Wait.
        picClerkData[myLine].lineCount--; // Leaving the line
        picLineLock.Release();
        CustomerToPictureClerk(ssn, myLine);
    }
}

void DecidePictureLine(int ssn)
{
    picLineLock.Acquire();

    int myLine = -1; // no line yet
    int lineSize = 1000; // bigger (bc we're finding shortest line) than # customers created
    
    // What if everyone's on break?
    int longestLine = -1; // Store the longest line (Once a single line has >= 3 Customers, Manager wakes up an ApplicationClerk)
    int longestLineSize = -1; // Smaller than any line could possibly be because we are searching for longest line.

    for (int i = 0; i < numPicClerks; i++) 
    {

        // Pick the shortest line with a clerk not on break
        if (picClerkData[i].lineCount < lineSize && picClerkData[i].state != ONBREAK)
        {
            myLine = i;
            lineSize = picClerkData[i].lineCount;
            //even if line size = 0, the clerk could still be busy since being at the counter is not                                                             ‘in line'
        }

        // What if everyones on break?
        // Keep track of the longest line
        if (picClerkData[i].lineCount > longestLineSize) 
        {
            longestLine = i;
        }

        // What if everyones on break?
        // Join the longest line and wait for Manager to wake up an Application Clerk (once this line gets at least 3 Customers)
        if (i == 4 && myLine == -1) 
        { // If this is the last ApplicationClerk and we haven't picked a line
            myLine = longestLine; // Join the longest line
            lineSize = picClerkData[i].lineCount;
        }
    }

    // I've selected a line...
    if(picClerkData[myLine].state == BUSY || picClerkData[myLine].state == ONBREAK) 
    { 
        // ApplicationClerk is not available, so wait in line
        picClerkData[myLine].lineCount++; // Join the line
        printf("Customer %d has gotten in regular line for PictureClerk %d.\n", ssn, myLine);
        picClerkLineCV[myLine]->Wait(&picLineLock); // Waiting in line
        // Reacquires lock after getting woken up inside Wait.
        picClerkData[myLine].lineCount--; // Leaving the line
    } 
    else 
    { 
        // Line was empty to begin with. Clerk is avail
        picClerkData[myLine].state = BUSY;
    }

    picLineLock.Release();
    CustomerToPictureClerk(ssn, myLine);
}


void PictureClerkToCustomer(int lineNumber)
{
    // TODO: TRANSFER DATA BETWEEN PIC CLERK AND CUSTOMER
    int ssn = 0;
    picClerkLock[lineNumber]->Acquire(); // acquire the lock for my line to pause time.
    picLineLock.Release(); //clerk must know a customer left before starting over
    picClerkCV[lineNumber]->Wait(picClerkLock[lineNumber]);
    printf("PictureClerk %d has taken a picture of Customer %d.\n", lineNumber, ssn);
    picClerkCV[lineNumber]->Signal(picClerkLock[lineNumber]);
    picClerkCV[lineNumber]->Wait(picClerkLock[lineNumber]);
    picClerkLock[lineNumber]->Release();
}

void PictureClerk(int lineNumber)
{

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
            picClerkData[lineNumber].state = BUSY;
            PictureClerkToCustomer(lineNumber);
        }
        else
        { 
            // nobody is waiting
            picClerkData[lineNumber].state = ONBREAK;
            picClerkBreakCV[lineNumber]->Wait(&picLineLock);
            // Go on break.
        }
    }
}

void CustomerToPassportClerk(int lineNumber)
{
    // TODO: TRANSMIT DATA FROM CUSTOMER TO PASSPORT CLERK
    int ssn = 0;

    passportClerkLock[lineNumber]->Acquire();//simulating the line
    //do {
        passportClerkCV[lineNumber]->Signal(passportClerkLock[lineNumber]);
        passportClerkCV[lineNumber]->Wait(passportClerkLock[lineNumber]);
         if(customerData[ssn].acceptedPicture && customerData[ssn].acceptedPicture){

         }
         else{

         }
        /*if (rand() < .5 )
        {
            customerData[ssn].acceptedPicture = true;
            printf("Customer %d does like their picture from PictureClerk %d.\n", ssn, lineNumber);
        }
        else
        {
            printf("Customer %d does not like their picture from PictureClerk %d.\n", ssn, lineNumber);
        }
    } while(customerData[ssn].acceptedPicture);*/

    passportClerkCV[lineNumber]->Signal(passportClerkLock[lineNumber]); //leaving
    passportClerkLock[lineNumber]->Release();
}

//add a method for each lock that exists between picture clerks and X
void DecidePassportLine(int ssn)
{
    passportLineLock.Acquire();

    int myLine = -1; // no line yet
    int lineSize = 1000;// bigger (bc we're finding shortest line) than # customers created  
    int longestLine = -1; // Store the longest line (Once a single line has >= 3 Customers, Manager wakes up an ApplicationClerk)
    int longestLineSize = -1; // Smaller than any line could possibly be because we are searching for longest line.

    for (int i = 0; i < numPassportClerks; i++) { 
        // Pick the shortest line with a clerk not on break
        if (passportClerkData[i].lineCount < lineSize && passportClerkData[i].state != ONBREAK)
        {
            myLine = i;
            lineSize = passportClerkData[i].lineCount;
               //even if line size = 0, the clerk could still be busy since being at the counter is not                                                             ‘in line'
        }

        // What if everyones on break?
        // Keep track of the longest line
        if (passportClerkData[i].lineCount > longestLineSize) {
            longestLine = i;
        }

        // What if everyones on break?
        // Join the longest line and wait for Manager to wake up an Application Clerk (once this line gets at least 3 Customers)
        if (i == 4 && myLine == -1) { // If this is the last ApplicationClerk and we haven't picked a line
            myLine = longestLine; // Join the longest line
            lineSize = passportClerkData[i].lineCount;
        }
    }
    // I've selected a line...
    if(passportClerkData[myLine].state == BUSY || passportClerkData[myLine].state == ONBREAK) { // ApplicationClerk is not available, so wait in line
        passportClerkData[myLine].lineCount++; // Join the line
        printf("Customer %d has gotten in regular line for ApplicationClerk %d.\n", ssn, myLine);
        passportClerkLineCV[myLine]->Wait(&passportLineLock); // Waiting in line
        // Reacquires lock after getting woken up inside Wait.
        passportClerkData[myLine].lineCount--; // Leaving the line
    } else { // Line was empty to begin with. Clerk is avail
        passportClerkData[myLine].state = BUSY;
    }
    passportLineLock.Release();
    CustomerToPassportClerk(myLine);
}

void CashierToPassportClerk(int ssn, int cashierLineNumber/*, int passportLineNumber*/)
{
    //does the cashier have to acquire the passport lock to check the current customers state??
}

void DecideCashierLine(int ssn){
    cashierLineLock.Acquire();    
    int myLine = -1; // no line yet
    int lineSize = 1000;// bigger (bc we're finding shortest line) than # customers created
    
    // What if everyone's on break?
    int longestLine = -1; // Store the longest line (Once a single line has >= 3 Customers, Manager wakes up an ApplicationClerk)
    int longestLineSize = -1; // Smaller than any line could possibly be because we are searching for longest line.

    for (int i = 0; i < numCashiers; i++) {
        // Pick the shortest line with a clerk not on break
        if (cashierData[i].lineCount < lineSize && cashierData[i].state != ONBREAK)
        {
            myLine = i;
            lineSize = cashierData[i].lineCount;
               //even if line size = 0, the clerk could still be busy since being at the counter is not                                                             ‘in line'
        }
        // What if everyones on break?
        // Keep track of the longest line
        if (cashierData[i].lineCount > longestLineSize) {
            longestLine = i;
        }

        // What if everyones on break?
        // Join the longest line and wait for Manager to wake up an Application Clerk (once this line gets at least 3 Customers)
        if (i == 4 && myLine == -1) { // If this is the last CashierClerk and we haven't picked a line
            myLine = longestLine; // Join the longest line
            lineSize = cashierData[i].lineCount;
        }
    }
    // I've selected a line...
    if(cashierData[myLine].state == BUSY || cashierData[myLine].state == ONBREAK) 
    { // ApplicationClerk is not available, so wait in line
        cashierData[myLine].lineCount++; // Join the line
        printf("Customer %d has gotten in regular line for PictureClerk %d.\n", ssn, myLine);
        cashierLineCV[myLine]->Wait(&cashierLineLock); // Waiting in line
        // Reacquires lock after getting woken up inside Wait.
        cashierData[myLine].lineCount--; // Leaving the line
    } 
    else 
    { // Line was empty to begin with. Clerk is avail
        cashierData[myLine].state = BUSY;
    }

    cashierLineLock.Release();
    CashierToPassportClerk(ssn, myLine);
}

void PassportClerkToCashier(int ssn, int lineNumber, int )
{

}

void CashierToCustomer()
{

}

void CustomerToCashier()
{

}


// TODO: add a method for each lock that exists between passport clerks and X


// TODO: Change clerksLineLock to clerkLineLock[i]
void Manager(){
    // Wakes up the clerks when there are >3 people waiting
    // Counts each clerks money
    int appClerkMoney = 0;
    int picClerkMoney = 0;
    int passportClerkMoney = 0;
    int cashierMoney = 0;
    int totalMoney = 0;

    while(true) {
        appLineLock.Acquire();
        for(int i = 0; i < numAppClerks; i++) 
        {
            appClerkMoney += appClerkData[i].bribeMoney;

            if(appClerkData[i].state == ONBREAK && appClerkData[i].lineCount >= 3) 
            {
                appClerkData[i].state = AVAILABLE;
                appClerkBreakCV[i]->Signal(&appLineLock);
                printf("Manager has woken up an ApplicationClerk\n");
            }
        }
        appLineLock.Release();

        picLineLock.Acquire();
        for(int i = 0; i < numPicClerks; i++) 
        {
            picClerkMoney += picClerkData[i].bribeMoney;

            if(picClerkData[i].state == ONBREAK && picClerkData[i].lineCount >= 3)
            {
                picClerkData[i].state = AVAILABLE;
                picClerkBreakCV[i]->Signal(&picLineLock);
                printf("Manager has woken up an PictureClerk\n");
                printf("ApplicationClerk %d is coming off break\n", i);
            }
        }
        picLineLock.Release();

        passportLineLock.Acquire();
        for(int i = 0; i < numPassportClerks; i++)
        {
            passportClerkMoney += passportClerkData[i].bribeMoney;

            if(passportClerkData[i].state == ONBREAK && passportClerkData[i].lineCount >= 3)
            {
                passportClerkData[i].state = AVAILABLE;
                passportClerkBreakCV[i]->Signal(&passportLineLock);
                printf("Manager has woken up an PassportClerk\n");
            }
        }
        passportLineLock.Release();

        printf("Manager has counted a total of $%d for ApplicationClerks\n", appClerkMoney);
        printf("Manager has counted a total of $%d for PictureClerks\n", picClerkMoney);
        printf("Manager has counted a total of $%d for PassportClerks\n", passportClerkMoney);
        printf("Manager has counted a total of $%d for Cashier\n", cashierMoney);
        printf("Manager has counted a total of $%d for the passport office\n", totalMoney);

        currentThread->Yield();
    }
}

void Senator()
{

}

void getInput()
{

}

void Customer(int ssn) 
{
    //if (rand() < 0.5) 
    {
        DecideApplicationLine(ssn);
        //DecidePictureLine(ssn);
    } 
    //else 
    {
        //DecidePictureLine(ssn);
        //DecideApplicationLine(ssn);
    }

    //DecidePassportLine(ssn);
    //DecideCashierLine(ssn);
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
    
    appClerkLock = new Lock*[numAppClerks];
    picClerkLock = new Lock*[numPicClerks];
    passportClerkLock = new Lock*[numPassportClerks];
    
    customerData = new CustomerData[numCustomers];
    appClerkData = new ApplicationClerkData[numAppClerks];
    passportClerkData = new PassportClerkData[numPassportClerks];
    picClerkData = new PictureClerkData[numPicClerks];
    cashierData = new CashierData[numCashiers];

    appClerkBreakCV = new Condition*[numAppClerks];
    picClerkBreakCV = new Condition*[numPicClerks];
    passportClerkBreakCV = new Condition*[numPassportClerks];

    for(int i = 0; i < numAppClerks; i++)
    {
        sprintf(name, "applicationClerkLock %d\n", i);
        appClerkLock[i] = new Lock(name);
        sprintf(name, "applicationClerkCV %d\n", i);
        appClerkCV[i] = new Condition(name);
        sprintf(name, "applicationClerkLineCV %d\n", i);
        appClerkLineCV[i] = new Condition(name);
    }

    for(int i = 0; i < numPicClerks; i++)
    {
        sprintf(name, "pictureClerkLock %d\n", i);
        picClerkLock[i] = new Lock(name);
        sprintf(name, "pictureClerkCV %d\n", i);
        picClerkCV[i] = new Condition(name);
    }

    for(int i = 0; i < numPassportClerks; i++)
    {
        sprintf(name, "passportClerkLock %d\n", i);
        passportClerkLock[i] = new Lock(name);
        sprintf(name, "passportClerkCV %d\n", i);
        passportClerkCV[i] = new Condition(name);
    }

    //for loop of user input
    for(int i = 0; i < numCustomers; i++) 
    {
        name = new char [20];
        sprintf(name, "customer %d", i);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)Customer, i); //picture or application first, faciliate all of the interactions
        //customerData[i].money = getMoney();//100,

        // whatever other customerData
    }

    for(int i = 0; i < numAppClerks; i++) 
    {
        name = new char [20];
        sprintf(name, "ApplicationClerk %d", i);
        t = new Thread(name);
        t->Fork((VoidFunctionPtr)ApplicationClerk, i); //picture or application first, faciliate all of the interactions
        //customerData[i].money = getMoney();//100,

        // whatever other customerData
    }

    //for loop of user input
    /*for(int i = 0; i < numPicClerks; i++) 
    {
        name = new char [20];
        sprintf(name,"picture clerk %d",i);

        t = new Thread(name);
        //t->Fork((VoidFunctionPtr)picture, i);
        //picClerkDataData[i].state = AVAILABLE;
        // whatever other customerData
    }

    for(int i = 0; i < numPassportClerks; i++) 
    {
        name = new char [20];
        sprintf(name,"application clerk %d",i);

        t = new Thread(name);
        //t->Fork((VoidFunctionPtr)customer, i);
        appClerkData[i].state = AVAILABLE;
        // whatever other customerData
    }
    for(int i = 0; i < numCashiers; i++) 
    {
        name = new char [20];
        sprintf(name,"application clerk %d",i);

        t = new Thread(name);
        //t->Fork((VoidFunctionPtr)customer, i);
        appClerkData[i].state = AVAILABLE;
        // whatever other customerData
    }*/
   
}
#endif

