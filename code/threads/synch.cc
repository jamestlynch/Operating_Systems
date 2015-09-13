// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
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
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
    printf("[Semaphore::V] %s acquired semaphore\n", currentThread->getName());
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) {
    name = debugName;
    lockOwner = NULL;
    state = 0;
    queue = new List();

}
Lock::~Lock() {
     delete queue;
}
void Lock::Acquire(){
    printf("[Lock::Acquire] %s called Acquire\n", currentThread->getName());
    IntStatus old = interrupt->SetLevel(IntOff);
    
    if (isHeldByCurrentThread()){  
        printf("[Lock::Acquire] CurrentThread is already lock owner and trying to acquire. Returning. \n");    
        //check if the thread that is trying to acquire the lock already owns it
        interrupt->SetLevel(old); //restore interrupts
        return;
    }

    else if (!lockOwner){
        //I can have it, make state busy, make myself the lock owner
        lockOwner=currentThread; //i am the lock owner
        state = 1; //busy
        printf("[Lock::Acquire] You are the new lock owner: %s\n", lockOwner->getName());
     }
     else {
        printf("[Lock::Acquire] %s is not lock owner, lock owner is: %s\n", currentThread->getName(), lockOwner->getName());
        //put current thread on lock’s wait Q
        queue->Append((void *)currentThread); // so go to sleep
        currentThread->Sleep();
    }
    
    (void) interrupt->SetLevel(old); //end of acquire
    printf("[Lock::Acquire] End of Acquire.\n");
    return;
}
void Lock::Release() {
    printf("Release\n");
    IntStatus old= interrupt->SetLevel(IntOff);
    if (!isHeldByCurrentThread()){
       //error message not lock owner
       interrupt->SetLevel(old);
       return;
     }
     //thread is lock owner
     if (!queue->IsEmpty()){
        lockOwner= (Thread *)queue->Remove(); //make them the owner
        scheduler->ReadyToRun(lockOwner);//put thread at the back of the ready queue 
      }
     else{
        lockOwner = NULL;
        state = 0; //state of the lock is free
    }

    interrupt->SetLevel(old);
    return;
}
bool Lock::isHeldByCurrentThread(){
    if(currentThread == NULL) return FALSE;

    if(currentThread == lockOwner) return TRUE;
    else return FALSE;
}
Condition::Condition(char* debugName) 
{
    name = debugName;
    waitingLock = NULL;
}
Condition::~Condition() { }

void Condition::Wait(Lock* conditionLock) 
{ 
    IntStatus oldLevel = interrupt->SetLevel(IntOff); // disable interrupts
 printf("Wait condition %d\n", conditionLock->getState() );
    if (!conditionLock) // checking if parameters are safe
    {
        printf("[Condition::Wait] conditionLock is null! Returning...\n");
        (void) interrupt->SetLevel(oldLevel); // re-enable interrupts
        return;
    }

    if (!waitingLock) // No lock associated with C.V. yet, set parameter = to our lock
    {
        printf("[Condition::Wait] nobody is waiting, waitingLock is null! Returning...\n");
        waitingLock = conditionLock;
    }


    if (waitingLock != conditionLock) // Passing in wrong lock!
    {
        printf("[Condition::Wait] Passed in the wrong lock! Returning...\n");
        (void) interrupt->SetLevel(oldLevel); // re-enable interrupts
        return;
    }

    // Before going to sleep, add myself to the C.V.'s waitQueue
    conditionLock->queue->Append((Thread *)currentThread);
    conditionLock->Release();
    currentThread->Sleep(); // Sleep until this waitingLock is available
    conditionLock->Acquire();
    (void) interrupt->SetLevel(oldLevel); // re-enable interrupts
}

void Condition::Signal(Lock* conditionLock) 
{
    printf("[Condition::Signal] Called.\n");


    IntStatus oldLevel = interrupt->SetLevel(IntOff); // disable interrupts

    if (!conditionLock) // checking if parameters are safe
    {
        printf("[Condition::Signal] conditionLock is null! Returning...\n");
        (void) interrupt->SetLevel(oldLevel); // re-enable interrupts
        return;
    }

    // BELOW IS PROBABLY WRONG

    printf("[Condition::Signal] before queue\n");

    //printf("waiting lock %d",(int)waitingLock);
    if (waitingLock){
       // if ((Thread *)waitingLock->queue->IsEmpty())
        //{
            printf("[Condition::Signal] C.V.'s lock has no threads on its waitingQueue! Returning...\n");
            (void) interrupt->SetLevel(oldLevel); // re-enable interrupts
            return;
          //  }
    }
    //else{

    //}
    printf("[Condition::Signal] after queue\n");
    if (waitingLock != conditionLock)
    {
        printf("[Condition::Signal] Passed in the wrong lock! Returning...\n");
        (void) interrupt->SetLevel(oldLevel); // re-enable interrupts
        return;
    }

    printf("[Condition::Signal] Before waking thread.\n");

    // BELOW IS PROBABLY WRONG




    //  NOTE: Done out of order from lecture below:
    // Wake 1 waiting thread
    // Remove 1 thread from wait queue
    Thread* waitingThread = (Thread *)waitingLock->queue->Remove();

    printf("[Condition::Signal] After removing thread %s from lock's waitQueue.\n", waitingThread->getName());

    waitingThread->setStatus(READY);

    printf("[Condition::Signal] After setting thread's status to READY.\n");


    // Put thread on scheduler's readyQueue
    scheduler->ReadyToRun(waitingThread);

    printf("[Condition::Signal] After telling scheduler thread is ReadyToRun.\n");


}

void Condition::Broadcast(Lock* conditionLock) 
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff); // disable interrupts

    if (!waitingLock)
    {
        printf("[Condition::Broadcast] conditionLock is null! Returning...\n");
        (void) interrupt->SetLevel(oldLevel); // re-enable interrupts
        return;
    }

    if (waitingLock != conditionLock)
    {
        printf("[Condition::Broadcast] Passed in the wrong lock! Returning...\n");
        (void) interrupt->SetLevel(oldLevel); // re-enable interrupts
        return;
    }

    while (!waitingLock->queue->IsEmpty())
    {
        this->Signal(waitingLock);
    }
}