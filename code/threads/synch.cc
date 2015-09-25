// synch.cc 
//  Routines for synchronizing threads.  Three kinds of
//  synchronization routines are defined here: semaphores, locks 
//      and condition variables (the implementation of the last two
//  are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"


#include <stdio.h>

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


bool debuggingLocks = false;
bool debuggingLockErrors = false;
bool debuggingCVs = false;
bool debuggingCVErrors = false;



//----------------------------------------------------------------------
// Semaphore::Semaphore
//  Initialize a semaphore, so that it can be used for synchronization.
//
//  "debugName" is an arbitrary name, useful for debugging.
//  "initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
//  De-allocate semaphore, when no longer needed.  Assume no one
//  is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
//  Wait until semaphore value > 0, then decrement.  Checking the
//  value and decrementing must be done atomically, so we
//  need to disable interrupts before checking the value.
//
//  Note that Thread::Sleep assumes that interrupts are disabled
//  when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts
    
    while (value == 0) {            // semaphore not available
    queue->Append((void *)currentThread);   // so go to sleep
    currentThread->Sleep();
    } 
    value--;                    // semaphore available, 
                        // consume its value
    
    (void) interrupt->SetLevel(oldLevel);   // re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
//  Increment semaphore value, waking up a waiter if necessary.
//  As with P(), this operation must be atomic, so we need to disable
//  interrupts.  Scheduler::ReadyToRun() assumes that threads
//  are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)    // make thread ready, consuming the V immediately
    scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
    //printf("[Semaphore::V] %s releasing\n", currentThread->getName());
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) 
{
    name = debugName;
   // lockOwner = NULL;
    lockOwner=NULL;
    //state = 0;
    sleepqueue = new List;

}
Lock::~Lock() 
{
     delete sleepqueue;
}
void Lock::Acquire()
{
    if (debuggingLocks) printf(YELLOW  "[Lock::Acquire] (%s) %s called acquire."  ANSI_COLOR_RESET  "\n", name, currentThread->getName());
    
    IntStatus old = interrupt->SetLevel(IntOff);
    if (isHeldByCurrentThread())
    {  
        //printf("[Lock::Acquire] CurrentThread is already the lock owner. \n");    
        //check if the thread that is trying to acquire the lock already owns it
        (void*) interrupt->SetLevel(old); //restore interrupts
        return;
    }
    if (!lockOwner)
    {
        //I can have it, make state busy, make myself the lock owner
        lockOwner=currentThread; //i am the lock owner
        state = 1; //set the lock state to busy
        if (debuggingLocks) printf(YELLOW  "[Lock::Acquire] (%s) %s acquired the lock."   ANSI_COLOR_RESET "\n", name, lockOwner->getName());
    }
    else 
    {
        if (debuggingLocks) printf(YELLOW  "[Lock::Acquire] (%s) %s is trying to acquire a lock already owned by %s. %s is being put on lock's queue."  ANSI_COLOR_RESET  "\n", name, currentThread->getName(), lockOwner->getName(), currentThread->getName());
        
        sleepqueue->Append((void *)currentThread); //put current thread on lock’s wait Q
        currentThread->Sleep();
        old = interrupt->SetLevel(IntOff);
        this->Acquire();

    }
    (void*) interrupt->SetLevel(old); //end of acquire
    //printf("lock owner is: %s\n", currentThread->getName());
    return;
}
void Lock::Release() 
{
    if (debuggingLocks) printf(YELLOW  "[Lock::Release] (%s) %s called release."  ANSI_COLOR_RESET  "\n", name, currentThread->getName());

    IntStatus oldLevel = interrupt->SetLevel(IntOff); //turn off interrupts
    
    if(!isHeldByCurrentThread())
    {
        if (debuggingLocks || debuggingLockErrors) printf(RED  "[Lock::Release] (%s) ERROR: %s is trying to release a lock owned by %s. Returning."  ANSI_COLOR_RESET  "\n", name, currentThread->getName(), lockOwner->getName());
        interrupt->SetLevel(oldLevel);

        return;
    }
    if(isHeldByCurrentThread()) //if current thread is lockowner
    {
        if (debuggingLocks) printf(YELLOW  "[Lock::Release] (%s) %s released the lock."  ANSI_COLOR_RESET  "\n", name, currentThread->getName());
        
        state = 0;//free the lock
        lockOwner = NULL;//return lockOwner to NULL
        Thread * newThread = (Thread *) sleepqueue->Remove(); //get the next thread that is asleep
        
        if(newThread != NULL) //if the next thread isn't null
        {
            scheduler->ReadyToRun(newThread); //put it in the scheduler.
        }
  }
    interrupt->SetLevel(oldLevel);
}

bool Lock::isHeldByCurrentThread()
{
    return currentThread == lockOwner;
}


Condition::Condition(char* debugName) 
{
    name = debugName;
    waitingLock = NULL;
    waitqueue = new List;
}

Condition::~Condition() 
{ 
    delete waitqueue;
}

void Condition::Wait(Lock * conditionLock) 
{
    if (debuggingCVs) printf(BLUE  "[Condition::Wait] (%s) %s called wait."  ANSI_COLOR_RESET  "\n", name, currentThread->getName());
    IntStatus oldLevel = interrupt->SetLevel(IntOff); //disable interrupts
    if(!conditionLock) //if condition lock is not owned by curr thread
    { 
        if (debuggingCVs || debuggingCVErrors) printf(RED  "[Condition::Wait] (%s) ERROR: %s passed in a invalid (null) lock."  ANSI_COLOR_RESET  "\n", name, currentThread->getName());
        interrupt->SetLevel(oldLevel);//restore interrupts
        return;
    }
    if(!conditionLock->isHeldByCurrentThread()) //if condition lock is not owned by curr thread
    { 
        if (debuggingCVs || debuggingCVErrors) printf(RED  "[Condition::Wait] (%s) ERROR: %s is trying to wait on a condition using a lock it does not own (and therefore does not have access to the crit. sect.)."  ANSI_COLOR_RESET  "\n", name, currentThread->getName());
        interrupt->SetLevel(oldLevel);//restore interrupts
        return;
    }    
    if (!waitingLock){
        waitingLock=conditionLock;
        if (debuggingCVs) printf(BLUE  "[Condition::Wait] (%s) CV's waiting lock being set to %s."  ANSI_COLOR_RESET  "\n", name, conditionLock->getName());
    }
    if (waitingLock != conditionLock){
        if (debuggingCVs || debuggingCVErrors) printf(RED  "[Condition::Wait] (%s) ERROR: CV already has a corresponding lock %s, but %s is trying to use lock %s."  ANSI_COLOR_RESET  "\n",  name, waitingLock->getName(), currentThread->getName(), conditionLock->getName());
        interrupt->SetLevel(oldLevel);//restore interrupts
        return;
    }
    //waitingLock=conditionLock;
    conditionLock->Release(); //release condition lock
    waitqueue->Append((void*) currentThread); //add current thread to wait queue
    currentThread->Sleep(); //put current thread to sleep
    conditionLock->Acquire(); //acquire condition lock
    interrupt->SetLevel(oldLevel); //enable interrupts
} 
void Condition::Signal(Lock * conditionLock)
{
    if (debuggingCVs) printf(BLUE  "[Condition::Signal] (%s) %s called signal."  ANSI_COLOR_RESET  "\n", name, currentThread->getName());
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    if(!conditionLock) //if condition lock is not owned by curr thread
    { 
        if (debuggingCVs || debuggingCVErrors) printf(RED  "[Condition::Signal] (%s) ERROR: %s passed in a invalid (null) lock."  ANSI_COLOR_RESET  "\n", name, currentThread->getName());
        interrupt->SetLevel(oldLevel);//restore interrupts
        return;
    }
    if (!conditionLock->isHeldByCurrentThread())
    {
        if (debuggingCVs || debuggingCVErrors) printf(RED  "[Condition::Signal] (%s) ERROR: %s is trying to signal a condition using a lock it does not own (and therefore does not have access to the crit. sect.)."  ANSI_COLOR_RESET  "\n", name, currentThread->getName());
        interrupt->SetLevel(oldLevel);
        return;
    }
    if (waitingLock && waitingLock != conditionLock) {
        if (debuggingCVs || debuggingCVErrors) printf(RED  "[Condition::Signal] (%s) ERROR: %s trying to signal a condition using the wrong lock. (%s = wrong lock, %s = proper lock)" ANSI_COLOR_RESET "\n", name, currentThread->getName(), conditionLock->getName(), waitingLock->getName());
        interrupt->SetLevel(oldLevel);//restore interrupts
        return;
    }
    if (waitqueue->IsEmpty()){
        if (debuggingCVs || debuggingCVErrors) printf(RED  "[Condition::Signal] (%s) %s signalled, but there were no waiting threads."  ANSI_COLOR_RESET  "\n", name, currentThread->getName());
        waitingLock = NULL;
        interrupt->SetLevel(oldLevel);//restore interrupts
        return;
    }

    Thread *next = (Thread *)waitqueue->Remove();
    if(next!=NULL) //while waitqueue isn't empty
    {
        if (debuggingCVs) printf(BLUE  "[Condition::Signal] (%s) %s signalled %s."  ANSI_COLOR_RESET  "\n", name, currentThread->getName(), next->getName());
        scheduler->ReadyToRun(next);
        interrupt->SetLevel(oldLevel);
    }
    if(waitqueue->IsEmpty())
    {
        waitingLock = NULL;
    }
} 
void Condition::Broadcast(Lock* conditionLock)
{
    if (debuggingCVs) printf(BLUE  "[Condition::Broadcast] (%s) %s called broadcast."  ANSI_COLOR_RESET  "\n", name, currentThread->getName());
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    if(!conditionLock) //if condition lock is not owned by curr thread
    { 
        if (debuggingCVs || debuggingCVErrors) printf(RED  "[Condition:Broadcast] (%s) ERROR: %s passed in a invalid (null) lock."  ANSI_COLOR_RESET  "\n", name, currentThread->getName());
        interrupt->SetLevel(oldLevel);//restore interrupts
        return;
    }
    if(!conditionLock->isHeldByCurrentThread())
    {
        if (debuggingCVs || debuggingCVErrors) printf(RED  "[Condition:Broadcast] (%s) ERROR: %s is trying to broadcast a condition using a lock it does not own (and therefore does not have access to the crit. sect.)."  ANSI_COLOR_RESET  "\n", name, currentThread->getName());
        interrupt->SetLevel(oldLevel); 
        return;
    }
    if (waitingLock && waitingLock != conditionLock) {
        if (debuggingCVs || debuggingCVErrors) printf(RED  "[Condition:Broadcast] (%s) ERROR: %s trying to broadcast a condition using the wrong lock. (%s = wrong lock, %s = proper lock)" ANSI_COLOR_RESET "\n", name, currentThread->getName(), conditionLock->getName(), waitingLock->getName());
        interrupt->SetLevel(oldLevel);//restore interrupts
        return;
    }
    interrupt->SetLevel(oldLevel);
    while(!waitqueue->IsEmpty()) {
        if (debuggingCVs) printf(BLUE  "[Condition::Broadcast] (%s) %s is signalling another thread."  ANSI_COLOR_RESET  "\n", name, currentThread->getName());
        Signal(conditionLock);
    }
}
