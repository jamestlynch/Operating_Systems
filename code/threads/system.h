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
#include "synch.h"
#include <vector>
#include <string>
using namespace std;

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  	// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;				// performance metrics
extern Timer *timer;					// the hardware alarm clock

class Machine;


#ifdef USER_PROGRAM

	extern Machine* machine;	// user program memory and registers

	//create tables for processes, condition variables, and locks
	#include "addrspace.h"

	// KernalLock extra info for cleaning up and guaranteeing process lock ownership
    struct KernelLock
    {
        Lock *lock;
        AddrSpace *space;
        bool toDelete;      
    };

    // KernelCV extra info for cleaning up and guaranteeing process CV ownership
    struct KernelCV
    {
        Condition *condition;
        AddrSpace *space;
        bool toDelete;
    };  
    struct Process{
    	AddrSpace *space;
    	int numThreads;
    	int processID;
    };
    //has all the threads
    //destroy locks/cvs associated w the process

	extern BitMap *memoryBitMap;

	extern vector<KernelLock*> locks;
	extern vector<KernelCV*> conditions;
	extern vector<Process*> processInfo;

	//create locks around these tables so only one program can access at a time
	#include "synch.h"
	extern Lock *processLock; //lock on process table
	extern Lock *conditionsLock;	//lock on cv table
	extern Lock *locksLock;	//lock on lock table

#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 

	#include "filesys.h"
	extern FileSystem *fileSystem;

#endif

#ifdef FILESYS

	#include "synchdisk.h"
	extern SynchDisk *synchDisk;

#endif

#ifdef NETWORK

	#include "post.h"
	extern PostOffice* postOffice;

#endif

#endif // SYSTEM_H
