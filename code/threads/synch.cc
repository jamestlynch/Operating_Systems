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

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff); // Disable interrupts
    
    while (value == 0)
    {
        // Semaphore not available, so go to sleep
        queue->Append((void *)currentThread);
        currentThread->Sleep();
    }

    value--; // Semaphore available, consume its value
    
    (void) interrupt->SetLevel(oldLevel); // Restore interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
//  Increment semaphore value, waking up a waiter if necessary. As with 
//  P(), this operation must be atomic so we need to disable interrupts. 
//
//  NOTE:   Scheduler::ReadyToRun() assumes that threads are disabled 
//          when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    
    IntStatus oldLevel = interrupt->SetLevel(IntOff); // Disable interrupts

    // Thread was waiting on semaphore, notify scheduler that thread woke up.
    thread = (Thread *)queue->Remove();
    if (thread != NULL)
        scheduler->ReadyToRun(thread);
    
    value++; // Increment semaphore value

    (void) interrupt->SetLevel(oldLevel); // Restore interrupts
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
    state = LOCKFREE;
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
//  Attempt to gain control over critical section. If lock already has 
//  an owner, go to sleep and re-Acquire when thread wakes up. If lock
//  has no owner, make thread the owner.
//
//  NOTE:   Thread::Sleep assumes that interrupts are disabled when it 
//          is called.
//----------------------------------------------------------------------

void
Lock::Acquire()
{    
    IntStatus old = interrupt->SetLevel(IntOff); // Disable interrupts
    
    if (isHeldByCurrentThread())
    {  
        DEBUG('s', "%s trying to Acquire lock it already owns.\n", 
            currentThread->getName());
        (void*) interrupt->SetLevel(old); // Restore interrupts
        return;
    }

    // Another thread owns the lock, go to sleep.
    if (lockOwner)
    {
        DEBUG('s', "%s already owns %s lock. %s is now on lock's queue.\n", 
            lockOwner->getName(), name, currentThread->getName());
        sleepqueue->Append((void *)currentThread); // Put current thread on lock’s wait queue
        currentThread->Sleep(); // Take thread off Scheduler
        old = interrupt->SetLevel(IntOff); // Restore interrupts
        
        // Woken up, try and Acquire again.
        //  NOTE: Another thread may have Acquired since Release that woke up currentThread.
        Acquire();
    }

    // Lock is free, make currentThread the owner
    else 
    {
        lockOwner = currentThread;
        state = LOCKBUSY;
        DEBUG('s', "%s Acquired %s lock.\n", lockOwner->getName(), name);
    }

    (void*) interrupt->SetLevel(old); // Restore interrupts
}

//----------------------------------------------------------------------
// Lock::Release
//  Give up control over critical section. If lock is not the owner,
//  print error and return (Threads cannot release locks owned by 
//  other threads). If lock is the owner, free up lock and wake up first
//  waiting thread (if any).
//----------------------------------------------------------------------

void
Lock::Release() 
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff); // Disable interrupts
    
    // Thread trying to Release a lock it does not own
    if (!isHeldByCurrentThread())
    {
        DEBUG('s', "ERROR: %s is trying to Release %s lock owned by %s.\n", 
            currentThread->getName(), name, lockOwner->getName());
        interrupt->SetLevel(oldLevel); // Restore interrupts
        return;
    }

    // Thread allowed to Release, free up lock for another thread to Acquire
    else
    {
        DEBUG('s', "%s Released %s lock.", currentThread->getName(), name);
        state = LOCKFREE;
        lockOwner = NULL;

        // Get next waiting thread and put on scheduler
        Thread *waitingThread = (Thread *) sleepqueue->Remove();
        if (waitingThread)
        {
            scheduler->ReadyToRun(waitingThread);
        }
    }

    interrupt->SetLevel(oldLevel); // Restore interrupts
}

//----------------------------------------------------------------------
// Lock::isHeldByCurrentThread
//  Used by Release and Condition methods for verifying that the calling
//  thread is the lockOwner.
//
//  Returns TRUE if thread owns lock.
//----------------------------------------------------------------------

bool
Lock::isHeldByCurrentThread()
{
    return currentThread == lockOwner;
}

//========================================================================================================================================
//
//  Condition Variables
//
//  See also:   threadtest.cc   test suite for synchronization objects
//
//========================================================================================================================================

//----------------------------------------------------------------------
// Condition::Condition
//  Initialize a condition, so that it can be used instead of busy 
//  waiting.
//
//  "debugName" -- an arbitrary name, useful for debugging.
//----------------------------------------------------------------------

Condition::Condition(char* debugName) 
{
    name = debugName;
    conditionlock = NULL;
    waitqueue = new List;
}

//----------------------------------------------------------------------
// Condition::~Condition
//  De-allocate condition, when no longer needed. Assume no one is still 
//  waiting on the condition! (DestroyCV syscall guarantees this).
//----------------------------------------------------------------------

Condition::~Condition() 
{ 
    delete waitqueue;
}

//----------------------------------------------------------------------
// Condition::validateLock
//  Returns true if the lock:
//  (1) Exists
//  (2) Belongs to the current thread
//  (3) Lock is the same as corresponding Condition Lock
//
//  Side effect of setting Condition's Lock if no lock was defined.
//
//  "lock" -- should be lock corresponding to the Condition's crit.sect.
//----------------------------------------------------------------------

bool
Condition::validateLock(Lock * lock)
{
    // Validate input: lock is defined
    ASSERT(lock);

    // Enforce mutual exclusion: Must own lock to critical section 
    if (!lock->isHeldByCurrentThread())
    {
        DEBUG('s', "ERROR: %s trying to Wait on %s Condition without Acquiring CV's lock, %s.\n",
            currentThread->getName(), name, lock->getName());
        return false;
    }

    // Condition has no lock associated with it yet, set it to the lock passed in.
    if (!conditionlock)
    {
        conditionlock = lock;
        DEBUG('s', "Set %s condition lock: %s.\n", name, conditionlock->getName());
    }

    // Lock does not correspond to Condition's lock: Print error mesage
    else if (conditionlock != lock)
    {
        DEBUG('s', "ERROR: %s trying to Wait on %s Condition with improper lock, %s. (Condition's lock is %s)\n",
            currentThread->getName(), name, lock->getName(), conditionlock->getName());
        return false;
    }

    return true;
}

//----------------------------------------------------------------------
// Condition::Wait
//  Wait until other threads update a Condition by Signal/Broadcast. 
//  Current thread must own the lock passed in and the lock must 
//  correspond to the same critical section as the Condition. If so, go
//  to sleep by putting thread on the Condition's wait queue (so it can
//  be woken back up by Signal/Broadcast).
//
//  "lock" -- should be lock corresponding to the Condition's crit.sect.
//----------------------------------------------------------------------

void Condition::Wait(Lock * lock) 
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff); // Disable interrupts

    // Enforce mutual exclusion: Thread must own lock
    //  Lock must be the same as Condition's lock
    if (!validateLock(lock))
    {
        interrupt->SetLevel(oldLevel); // Restore interrupts
        return;
    }

    // Prevent deadlock of holding lock while going to sleep and keeping other 
    //  threads from Signaling or Waiting on the Condition.
    lock->Release();

    // Add to Wait queue so thread can be woken back up and go to sleep
    waitqueue->Append((void*) currentThread);
    currentThread->Sleep();

    // Re-entering critical section
    lock->Acquire();

    interrupt->SetLevel(oldLevel); // Restore interrupts
}

//----------------------------------------------------------------------
// Condition::Signal
//  Signal a single waiting thread. Current thread must have access to 
//  the critical section synchronized by the Condition lock. If there
//  are no more waiting Threads, recycle the Condition.
//
//  "lock" -- should be lock corresponding to the Condition's crit.sect.
//----------------------------------------------------------------------

void Condition::Signal(Lock * lock)
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff); // Disable interrupts

    // Enforce mutual exclusion: Thread must own lock
    //  Lock must be the same as Condition's lock
    if (!validateLock(lock))
    {
        interrupt->SetLevel(oldLevel); // Restore interrupts
        return;
    }

    // Pointless Signal, recycle Condition
    if (waitqueue->IsEmpty())
    {
        DEBUG('s', "%s Signalled %s Condition, but no threads waiting.\n",
            currentThread->getName(), name);
        conditionlock = NULL; // Free to be reused
        interrupt->SetLevel(oldLevel); // Restore interrupts
        return;
    }

    // Wake up one waiting thread
    Thread *waitingThread = (Thread *)waitqueue->Remove();
    if (waitingThread)
    {
        DEBUG('s', "%s Signalled %s Condition, waking up %s.\n",
            currentThread->getName(), name, waitingThread->getName());
        scheduler->ReadyToRun(waitingThread);
    }

    // Recycle Condition if no threads waiting
    if (waitqueue->IsEmpty())
    {
        conditionlock = NULL;
    }

    interrupt->SetLevel(oldLevel); // Restore interrupts
}

//----------------------------------------------------------------------
// Condition::Broadcast
//  Wake up all waiting threads. Current thread must have access to the
//  critical section synchronized by the Condition lock.
//
//  "lock" -- should be lock corresponding to the Condition's crit.sect.
//----------------------------------------------------------------------

void Condition::Broadcast(Lock* lock)
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff); // Disable interrupts

    // Enforce mutual exclusion: Thread must own lock
    //  Lock must be the same as Condition's lock
    if (!validateLock(lock))
    {
        interrupt->SetLevel(oldLevel); // Restore interrupts
        return;
    }

    // Wake up all Waiting Threads
    while (!waitqueue->IsEmpty())
    {
        DEBUG('s', "%s is Broadcasting to all Waiting Threads for %d Condition.\n",
            currentThread->getName(), name);
        Signal(lock);
    }

    interrupt->SetLevel(oldLevel); // Restore interrupts
}
