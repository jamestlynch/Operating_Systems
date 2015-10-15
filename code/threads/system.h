// system.h 
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"
#include "bitmap.h"

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock
extern BitMap bitmap;


#ifdef USER_PROGRAM
#include "machine.h"
extern Machine* machine;	// user program memory and registers

//create tables for processes, condition variables, and locks
#include "addrspace.h"
extern Table* processT; //process table
extern Table* cvT;	//condition var table
extern Table* lockT;	//lock table

#include <std::vector>
extern vector<KernelLock> myLockArray;
extern vector<KernelCV> myCVArray;

//create locks around these tables so only one program can access at a time
#include "synch.h"
extern Lock* processTLock; //lock on process table
extern Lock* cvTLock;	//lock on cv table
extern Lock* lockTLock;	//lock on lock table

#include "bitmap.h"
extern BitMap *BitMap; //based on class notes, we need this...not sure what it's used for yet. @james or @austin do you know?

#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
#endif

#endif // SYSTEM_H
