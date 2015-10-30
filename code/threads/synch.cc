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

//========================================================================================================================================
//
// Semaphores
//
//  See also:   threadtest.cc   test suite for synchronization objects
//
//========================================================================================================================================

//----------------------------------------------------------------------
// Semaphore::Semaphore
//  Initialize a semaphore, so that it can be used for synchronization.
//
//  "debugName" -- an arbitrary name, useful for debugging.
//  "initialValue" -- the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
//  De-allocate semaphore, when no longer needed. Assume no one is still 
//  waiting on the semaphore!
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
//  NOTE:   Thread::Sleep assumes that interrupts are disabled when it 
//          is called.
//----------------------------------------------------------------------

void Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff); // Disable interrupts
    
    while (value == 0)
    {
        // Semaphore not available, so go to sleep
        queue->Append((void *)currentThread);
        currentThread->Sleep();
    }

    value--; // Semaphore available, consume its value
    
    (void) interrupt->SetLevel(oldLevel); // Re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
//  Increment semaphore value, waking up a waiter if necessary. As with 
//  P(), this operation must be atomic so we need to disable interrupts. 
//
//  NOTE:   Scheduler::ReadyToRun() assumes that threads are disabled 
//          when it is called.
//----------------------------------------------------------------------

void Semaphore::V()
{
    Thread *thread;
    
    IntStatus oldLevel = interrupt->SetLevel(IntOff); // Disable interrupts

    // Thread was waiting on semaphore, notify scheduler that thread woke up.
    thread = (Thread *)queue->Remove();
    if (thread != NULL)
        scheduler->ReadyToRun(thread);
    
    value++; // Increment semaphore value

    (void) interrupt->SetLevel(oldLevel); // Re-enable interrupts
}

//========================================================================================================================================
//
// Locks
//  
//  See also:   threadtest.cc   test suite for synchronization objects
//
//========================================================================================================================================

//----------------------------------------------------------------------
// Lock::Lock
//  Initialize a lock, so that it can be used for mutual exclusion.
//
//  "debugName" -- an arbitrary name, useful for debugging.
//----------------------------------------------------------------------

Lock::Lock(char* debugName) 
{
    name = debugName;
    state = FREE;
    lockOwner = NULL; // Thread that most recently Acquired lock
    sleepqueue = new List; // Waiting threads
}

//----------------------------------------------------------------------
// Lock::~Lock
//  De-allocate lock, when no longer needed. Assume no one is still 
//  waiting on the lock! (DestroyLock syscall guarantees this).
//----------------------------------------------------------------------

Lock::~Lock() 
{
     delete sleepqueue;
}

//----------------------------------------------------------------------
// Lock::Acquire
//  TODO: Description
//
//  NOTE:   Thread::Sleep assumes that interrupts are disabled when it 
//          is called.
//----------------------------------------------------------------------

void Lock::Acquire()
{    
    IntStatus old = interrupt->SetLevel(IntOff); // Disable interrupts
    
    if (isHeldByCurrentThread())
    {  
        DEBUG('l', "%s trying to Acquire lock it already owns.\n", currentThread->getName());
        (void*) interrupt->SetLevel(old); // Re-enable interrupts
        return;
    }

    // Another thread owns the lock, go to sleep.
    if (lockOwner)
    {
        DEBUG('l', "%s already owns %s lock. %s is now on lock's queue.\n", 
            lockOwner->getName(), name, currentThread->getName());
        sleepqueue->Append((void *)currentThread); // Put current thread on lock’s wait queue
        currentThread->Sleep(); // Take thread off Scheduler
        old = interrupt->SetLevel(IntOff); // Re-enable interrupts
        
        // Woken up, try and Acquire again.
        //  NOTE: Another thread may have Acquired since Release that woke up currentThread.
        this->Acquire();
    }

    // Lock is free, make currentThread the owner
    else 
    {
        lockOwner = currentThread;
        state = BUSY;
        DEBUG('l', "%s Acquired %s lock.\n", lockOwner->getName(), name);

    }

    (void*) interrupt->SetLevel(old); // Re-enable interrupts
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

//========================================================================================================================================
//
//  Condition Variables
//
//========================================================================================================================================

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
