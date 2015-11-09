// progtest.cc 
//	Test routines for demonstrating that Nachos can load
//	a user program and execute it.  
//
//	Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "console.h"
#include "addrspace.h"
#include "synch.h"
#include "string.h"

#define QUANTUM 100

//----------------------------------------------------------------------
// StartProcess
// 	Run a user program.  Open the executable, load it into
//	memory, and jump to it.
//----------------------------------------------------------------------
void StartProcess(char *filename)
{
    processLock->Acquire();
    printf("TEST PRINT");

    OpenFile *executable = fileSystem->Open(filename);
    AddrSpace *space;

    if (executable == NULL) 
    {
	   printf("Unable to open file %s\n", filename);
       processLock->Release();
	   return;
    }

    space = new AddrSpace(executable);
    for(int i=0; i<10; i++){
        printf("inside start process valid bit: %d\n", ipt[i].valid);
        printf("inside start process ipt virtual page: %d\n", ipt[i].virtualPage);

    }

    Process * p = new Process();

    processInfo.push_back(p);
    p->processID = processInfo.size() - 1;
    p->space = space;
    p->numExecutingThreads = 1;
    p->numSleepingThreads = 0;

    currentThread->processID = p->processID;
    currentThread->space = space;

    delete executable;			// close file

    currentThread->stackPage = space->InitRegisters();		// set the initial register values
    space->RestoreState();		// load page table register

    processLock->Release();

    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}
std::vector< std::vector<int>* > monitorVars;
// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.

static Console *console;
static Semaphore *readAvail;
static Semaphore *writeDone;

//----------------------------------------------------------------------
// ConsoleInterruptHandlers
// 	Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

//----------------------------------------------------------------------
// ConsoleTest
// 	Test the console by echoing characters typed at the input onto
//	the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------

void 
ConsoleTest (char *in, char *out)
{
    char ch;

    console = new Console(in, out, ReadAvail, WriteDone, 0);
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);
    
    for (;;) {
	readAvail->P();		// wait for character to arrive
	ch = console->GetChar();
	console->PutChar(ch);	// echo it!
	writeDone->P() ;        // wait for write to finish
	if (ch == 'q') return;  // if q, quit
    }
}

