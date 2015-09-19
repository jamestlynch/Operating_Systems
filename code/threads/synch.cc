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
    printf("[Lock::Acquire] %s called Acquire\n", currentThread->getName());
    
    IntStatus old = interrupt->SetLevel(IntOff);
    if (isHeldByCurrentThread())
    {  
        printf("[Lock::Acquire] CurrentThread is already the lock owner. \n");    
        //check if the thread that is trying to acquire the lock already owns it
        (void*) interrupt->SetLevel(old); //restore interrupts
        return;
    }
    if (!lockOwner)
    {
        //I can have it, make state busy, make myself the lock owner
        lockOwner=currentThread; //i am the lock owner
        state = 1; //set the lock state to busy
        printf("[Lock::Acquire] You are the new lock owner: %s\n", lockOwner->getName());
    }
    else 
    {
        printf("[Lock::Acquire] %s are not lock owner, lock owner is: %s\n", currentThread->getName(), lockOwner->getName());
        sleepqueue->Append((void *)currentThread); //put current thread on lock’s wait Q
        currentThread->Sleep();
    }
    (void*) interrupt->SetLevel(old); //end of acquire
    printf("lock owner is: %s\n", currentThread->getName());
    return;
}
void Lock::Release() 
{
IntStatus oldLevel = interrupt->SetLevel(IntOff); //turn off interrupts
if(!isHeldByCurrentThread())
{
    printf("[Lock::Release] error");
    interrupt->SetLevel(oldLevel);
    return;
}
if(isHeldByCurrentThread()) //if current thread is lockowner
    {
    printf("[Lock::Release] current thread successfully releasing is %s\n", currentThread->getName());
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
    IntStatus oldLevel = interrupt->SetLevel(IntOff); //disable interrupts
    if(!conditionLock) //if condition lock is not owned by curr thread
    { 
        printf("[Condition:Wait] thread %s cannot release the condition lock\n", currentThread->getName());
        interrupt->SetLevel(oldLevel);//restore interrupts
        return;
    }
    if(!conditionLock->isHeldByCurrentThread()) //if condition lock is not owned by curr thread
    { 
        printf("[Condition:Wait] thread %s cannot release the condition lock\n", currentThread->getName());
        interrupt->SetLevel(oldLevel);//restore interrupts
        return;
    }    
    if (!waitingLock){
        waitingLock=conditionLock;
        printf("[Condition:Wait] setting waiting lock = condition lock: %s\n",  conditionLock->getName());
    }
    if (waitingLock != conditionLock){
        printf("[Condition:Wait] Fatal Error , condition lock: %s\n",  conditionLock->getName());
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
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    if(!conditionLock) //if condition lock is not owned by curr thread
    { 
        printf("[Condition:Wait] thread %s cannot release the condition lock\n", currentThread->getName());
        interrupt->SetLevel(oldLevel);//restore interrupts
        return;
    }
    if (!conditionLock->isHeldByCurrentThread())
    { //printf("[Condition::Signal] before queue\n");
        interrupt->SetLevel(oldLevel);
        return;
    }
    if (waitingLock && waitingLock!=conditionLock){
        printf("[Condition:Wait] Fatal Error");
        interrupt->SetLevel(oldLevel);//restore interrupts
        return;
    }
    Thread *next = (Thread *)waitqueue->Remove();
    if(next!=NULL) //while waitqueue isn't empty
    {
        scheduler->ReadyToRun(next);
        interrupt->SetLevel(oldLevel);
    }
    if(waitqueue->IsEmpty())
    {
        waitingLock=NULL;
    }
    printf("[Condition::Signal] waitingqueue is now null\n");
} 
void Condition::Broadcast(Lock* conditionLock)
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    if(!conditionLock) //if condition lock is not owned by curr thread
    { 
        printf("[Condition:Wait] thread %s cannot release the condition lock\n", currentThread->getName());
        interrupt->SetLevel(oldLevel);//restore interrupts
        return;
    }
    if(!conditionLock->isHeldByCurrentThread())
    {
        printf("[Condition:Broadcast] thread %s cannot the condition lock\n", currentThread->getName());
        interrupt->SetLevel(oldLevel); 
        return;
    }
     if (!waitingLock || waitingLock!=conditionLock){
        printf("[Condition:Wait] Fatal Error");
        interrupt->SetLevel(oldLevel);//restore interrupts
        return;
    }
    interrupt->SetLevel(oldLevel);
    while(!waitqueue->IsEmpty()){
        Signal(conditionLock);
    }
}