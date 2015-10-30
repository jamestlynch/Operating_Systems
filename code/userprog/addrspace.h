// addrspace.h 
//	Data structures to keep track of executing user programs 
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "table.h"

#define UserStackSize		1024 	// increase this as necessary!

#define MaxOpenFiles 256
#define MaxChildSpaces 256

#include "translate.h"

class IPTEntry;
class PageTableEntry;

class AddrSpace {
  public:
    AddrSpace(OpenFile *executable);	// Create an address space,
					// initializing it with the program
					// stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space

    int InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch
    Table fileTable;			// Table of openfiles

    int NewPageTable();
    void ReclaimStack(int stackPage);
    void ReclaimPageTable();

// private:
    TranslationEntry *pageTable;	// Assume linear page table translation
    PageTableEntry *pageT;

    unsigned int numPages;		// Number of pages in the virtual 
					// address space
};

//translation entry with process owner. might need to add more stuff to this class



/*
inside addrspace constructor set enum to executable. change it inside of 
handle memory full to swapfile. on an ipt miss set to main memory.
*/

#endif // ADDRSPACE_H
