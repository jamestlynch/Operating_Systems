// system.cc 
//	Nachos initialization and cleanup routines.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "machine.h"


// This defines *all* of the global data structures used by Nachos.
// These are all initialized and de-allocated by this file.

Thread *currentThread;          // the thread we are running now
Thread *threadToBeDestroyed;  	// the thread that just finished
Scheduler *scheduler;			// the ready list
Interrupt *interrupt;           // interrupt status
Statistics *stats;              // performance metrics
Timer *timer;                   // the hardware timer device, for invoking context switches


#ifdef FILESYS_NEEDED
    FileSystem *fileSystem;
#endif

#ifdef FILESYS
    SynchDisk *synchDisk;
#endif

// USER_PROGRAM requires either FILESYS or FILESYS_STUB
#ifdef USER_PROGRAM
    Machine *machine;

    BitMap *memBitMap;
    
    vector<KernelLock*> locks;
    vector<KernelCV*> conditions;
    vector<Process*> processInfo;

    Lock *memLock;
    Lock *processLock;
    Lock *conditionsLock;
    Lock *locksLock;
#endif

#ifdef NETWORK
    PostOffice *postOffice;
#endif


// External definition, to allow us to take a pointer to this function
extern void Cleanup();


//----------------------------------------------------------------------
// TimerInterruptHandler
// 	Interrupt handler for the timer device.  The timer device is
//	set up to interrupt the CPU periodically (once every TimerTicks).
//	This routine is called each time there is a timer interrupt,
//	with interrupts disabled.
//
//	Note that instead of calling Yield() directly (which would
//	suspend the interrupt handler, not the interrupted thread
//	which is what we wanted to context switch), we set a flag
//	so that once the interrupt handler is done, it will appear as 
//	if the interrupted thread called Yield at the point it is 
//	was interrupted.
//
//	"dummy" is because every interrupt handler takes one argument,
//		whether it needs it or not.
//----------------------------------------------------------------------
static void
TimerInterruptHandler(int dummy)
{
    if (interrupt->getStatus() != IdleMode)
	interrupt->YieldOnReturn();
}

//----------------------------------------------------------------------
// Initialize
// 	Initialize Nachos global data structures.  Interpret command
//	line arguments in order to determine flags for the initialization.  
// 
//	"argc" is the number of command line arguments (including the name
//		of the command) -- ex: "nachos -d +" -> argc = 3 
//	"argv" is an array of strings, one for each command line argument
//		ex: "nachos -d +" -> argv = {"nachos", "-d", "+"}
//----------------------------------------------------------------------
void
Initialize(int argc, char **argv)
{
    int argCount;
    char* debugArgs = "";
    bool randomYield = FALSE;

#ifdef USER_PROGRAM
    bool debugUserProg = FALSE;	// single step user program
#endif
#ifdef FILESYS_NEEDED
    bool format = FALSE;	// format disk
#endif
#ifdef NETWORK
    double rely = 1;		// network reliability
    int netname = 0;		// UNIX socket name
#endif
    
    for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount) {
	argCount = 1;
	if (!strcmp(*argv, "-d")) {
	    if (argc == 1)
		debugArgs = "+";	// turn on all debug flags
	    else {
	    	debugArgs = *(argv + 1);
	    	argCount = 2;
	    }
	} else if (!strcmp(*argv, "-rs")) {
	    ASSERT(argc > 1);
	    RandomInit(atoi(*(argv + 1)));	// initialize pseudo-random
						// number generator
	    randomYield = TRUE;
	    argCount = 2;
	}
#ifdef USER_PROGRAM
	if (!strcmp(*argv, "-s"))
	    debugUserProg = TRUE;
#endif
#ifdef FILESYS_NEEDED
	if (!strcmp(*argv, "-f"))
	    format = TRUE;
#endif
#ifdef NETWORK
	if (!strcmp(*argv, "-l")) {
	    ASSERT(argc > 1);
	    rely = atof(*(argv + 1));
	    argCount = 2;
	} else if (!strcmp(*argv, "-m")) {
	    ASSERT(argc > 1);
	    netname = atoi(*(argv + 1));
	    argCount = 2;
	}
#endif
    }

    DebugInit(debugArgs);			// initialize DEBUG messages
    stats = new Statistics();			// collect statistics
    interrupt = new Interrupt;			// start up interrupt handling
    scheduler = new Scheduler();		// initialize the ready queue
    if (randomYield)				// start the timer (if needed)
	timer = new Timer(TimerInterruptHandler, 0, randomYield);

    threadToBeDestroyed = NULL;

    // We didn't explicitly allocate the current thread we are running in.
    // But if it ever tries to give up the CPU, we better have a Thread
    // object to save its state. 
    currentThread = new Thread("main");		
    currentThread->setStatus(RUNNING);
    //printf("1");
    interrupt->Enable();
    //printf("2");
    CallOnUserAbort(Cleanup);			// if user hits ctl-C
    //printf("3");
    
#ifdef USER_PROGRAM
    machine = new Machine(debugUserProg);	// this must come first

    // Create tables for tracking processes, condition variables, and locks
    // TODO: Define Table *processT;

    // Synchronize these tables with locks so only one program can access at a time
    // TODO: Do we really need locks since OS is the only "program" updating/reading from these tables?
    // TODO: Define Lock *processLock;
    memLock = new Lock("MemBitMapLock");
    conditionsLock = new Lock("KernelCVLock");
    locksLock = new Lock("KernelLocksLock");
    processLock = new Lock("ProcessLock");

    memBitMap = new BitMap(NumPhysPages); //num phys pages goes in machine.h according to class notes 

#endif

#ifdef FILESYS
    synchDisk = new SynchDisk("DISK");
#endif

#ifdef FILESYS_NEEDED
    fileSystem = new FileSystem(format);
#endif

#ifdef NETWORK
    postOffice = new PostOffice(netname, rely, 10);
#endif
}

//----------------------------------------------------------------------
// Cleanup
// 	Nachos is halting.  De-allocate global data structures.
//----------------------------------------------------------------------
void
Cleanup()
{
    printf("\nCleaning up...\n");
#ifdef NETWORK
    delete postOffice;
#endif
    
#ifdef USER_PROGRAM
    delete machine; //delete machine

    delete locksLock;
    delete conditionsLock;
    delete processLock;
    delete memLock;

    delete memBitMap;

    for(int i = 0; i < locks.size(); i++)
    {
        if(locks[i])
        {
            delete locks[i]->lock;
        }
    }

    for(int i = 0; i < conditions.size(); i++)
    {
        if(conditions[i])
        {
            delete conditions[i]->condition;
        }
    }

    for(int i = 0; i < processInfo.size(); i++)
    {
        if(processInfo[i])
        {
            delete processInfo[i]->space;
        }
    }
#endif

#ifdef FILESYS_NEEDED
    delete fileSystem;
#endif

#ifdef FILESYS
    delete synchDisk;
#endif
    
    delete timer;
    delete scheduler;
    delete interrupt;
    
    Exit(0);
}

