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
#include "synch.h"
#include "synchlist.h"
//#include "filesys.h"
#include <vector>
#include <list>
#include <queue>
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
extern int tlbCounter;


class Machine;
class AddrSpace;
class Mail;

#ifdef USER_PROGRAM

    // Machine exectures user programs
	extern Machine *machine;

	// Syscall Synchronization Objects also need to know (1) their program
    //  and whether set toDelete so clean up can occur when last Threads is 
    //  done waiting.
    // Store all Kernel Locks and CVs inside of tables indexed by their IDs.

    struct KernelLock
    {
        Lock *lock;
        AddrSpace *space;
        bool toDelete;
    };

    struct KernelCV
    {
        Condition *condition;
        AddrSpace *space;
        bool toDelete;
    };

    extern vector<KernelLock*> locks;
    extern vector<KernelCV*> conditions;

    extern Lock *locksLock; // Synchronize lock table
    extern Lock *conditionsLock; // Synchronize CV table


    // To properly clean up User Programs on Exit, store information about
    //  all running processes inside a table.

    struct Process
    {
    	AddrSpace *space;
    	int numSleepingThreads;
    	int numExecutingThreads;
    	int processID;
    };

    extern vector<Process*> processInfo;
    extern int processCount;
    extern Lock *processLock; // Synchronize Process table

    // Memory Management
    //  Keep track of available Memory (memBitMap), Memory Page ownership
    //  (ipt), order of Memory updates for FIFO deletion (memFIFO), and 
    //  synchronize all updates to Main Memory since it's a shared resource

	extern BitMap *memBitMap;
    extern Lock *memLock;

    // IPTs store Pages from various user programs; We need a way of telling
    // the process that the memory belongs to.

    class IPTEntry : public TranslationEntry
    {
        public:
            AddrSpace * space; // Process memory belongs to
    };

    extern IPTEntry *ipt;

    extern OpenFile *swapFile;
    extern BitMap *swapBitMap;

    // Memory Eviction Strategy
    //  Get from -P flag when running Nachos, memFIFO stores the order PPNs
    //  allocated for FIFO eviction.

    extern bool isFIFO;
    extern list<int> memFIFO;

#endif

#ifdef USE_TLB

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
    extern int netname;

#endif

#endif // SYSTEM_H
