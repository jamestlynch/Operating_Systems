// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#include "table.h"
#include "synch.h"

extern "C" { int bzero(char *, int); };

Table::Table(int s) : map(s), table(0), lock(0), size(s) {
    table = new void *[size];
    lock = new Lock("TableLock");
}

Table::~Table() {
    if (table) {
	delete table;
	table = 0;
    }
    if (lock) {
	delete lock;
	lock = 0;
    }
}

void *Table::Get(int i) {
    // Return the element associated with the given if, or 0 if
    // there is none.

    return (i >=0 && i < size && map.Test(i)) ? table[i] : 0;
}

int Table::Put(void *f) {
    // Put the element in the table and return the slot it used.  Use a
    // lock so 2 files don't get the same space.
    int i;	// to find the next slot

    lock->Acquire();
    i = map.Find();
    lock->Release();
    if ( i != -1)
	table[i] = f;
    return i;
}

void *Table::Remove(int i) {
    // Remove the element associated with identifier i from the table,
    // and return it.

    void *f =0;

    if ( i >= 0 && i < size ) {
	lock->Acquire();
	if ( map.Test(i) ) {
	    map.Clear(i);
	    f = table[i];
	    table[i] = 0;
	}
	lock->Release();
    }
    return f;
}

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	"executable" is the file containing the object code to load into memory
//
//      It's possible to fail to fully construct the address space for
//      several reasons, including being unable to allocate memory,
//      and being unable to read key parts of the executable.
//      Incompletely consretucted address spaces have the member
//      constructed set to false.
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable) : fileTable(MaxOpenFiles) 
{
    NoffHeader noffH;
    unsigned int i, size;

    // Don't allocate the input or output to disk files
    fileTable.Put(0);
    fileTable.Put(0);

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && (WordToHost(noffH.noffMagic) == NOFFMAGIC))
    {
        SwapHeader(&noffH);
    }
    	
    ASSERT(noffH.noffMagic == NOFFMAGIC);

    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size;
    numPages = divRoundUp(size, PageSize) + divRoundUp(UserStackSize, PageSize);
    // we need to increase the size
	// to leave room for the stack
    size = numPages * PageSize;

    ASSERT(numPages <= NumPhysPages);		
    // check we're not trying
	// to run anything too big --
	// at least until we have
	// virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", numPages, size);
    
    // first, set up the translation 
    memLock->Acquire();

//need to implement for TLB use.
    
        /*pageTable = new PageTableEntry[numPages];
        for (i = 0; i < (numPages); i++) {
            pageTable[i].virtualPage = i;
            pageTable[i].valid = false;
            pageTable[i].use = false;
            pageTable[i].dirty = false;

        }*/

    

//pageTable is if not using TLB
    pageTable = new PageTableEntry[numPages];
    for (i = 0; i < numPages; i++) 
    {
    	pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
        pageTable[i].physicalPage = memBitMap->Find();
    	pageTable[i].valid = TRUE;
    	pageTable[i].use = FALSE;
    	pageTable[i].dirty = FALSE;

        // if the code segment was entirely on 
		// a separate page, we could set its 
		// pages to be read-only

        // find page memory that nobody is using
        // copy from executable to that page of memory
        // how much to copy? pagesize!

        /*if (pageTable[i].physicalPage == -1)
        {
          printf("No more physical memory available.\n");
          interrupt->Halt();
        }*/
        //printf("PageSize: %d, physicalPage: %d, virtualPage: %d", PageSize, pageTable[i].physicalPage, pageTable[i].virtualPage);

        //executable->ReadAt(&(machine->mainMemory[PageSize * pageTable[i].physicalPage]), PageSize, noffH.code.inFileAddr + (pageTable[i].virtualPage * PageSize));
    }
    for (i=0; i<NumPhysPages; i++){
        ipt[i].virtualPage    =   pageTable[i].virtualPage;
        ipt[i].physicalPage   =   pageTable[i].physicalPage;
        ipt[i].valid          =   pageTable[i].valid;
        ipt[i].use            =   pageTable[i].use;
        ipt[i].dirty          =   pageTable[i].dirty;
        ipt[i].readOnly       =   pageTable[i].readOnly;
        ipt[i].space          =   this;
    }

    memLock->Release();


    
    // zero out the entire address space, to zero the unitialized data segment 
    // and the stack segment
    // bzero(machine->mainMemory, size);

    // then, copy in the code and data segments into memory
    /*if (noffH.code.size > 0) 
    {
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", noffH.code.virtualAddr, noffH.code.size);
        executable->ReadAt(&(machine->mainMemory[noffH.code.virtualAddr]), noffH.code.size, noffH.code.inFileAddr);
    }

    if (noffH.initData.size > 0) 
    {
        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", noffH.initData.virtualAddr, noffH.initData.size);
        executable->ReadAt(&(machine->mainMemory[noffH.initData.virtualAddr]), noffH.initData.size, noffH.initData.inFileAddr);
    }*/
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//
// 	Dealloate an address space.  release pages, page tables, files
// 	and file tables
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    delete pageTable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

int AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	   machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %x\n", numPages * PageSize - 16);

    return numPages - (UserStackSize / PageSize);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{
    #ifdef USE_TLB
    // Invalidate all TLB pages on context switch. disable interrupts.
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    for (int i=0; i<4; i++){
        machine->tlb[i].valid=false;
    }
    //check if dirty bit was changed- if yes, update in pageTable.

    (void) interrupt->SetLevel(oldLevel);

#endif // USE_TLB

    
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    //machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}

int AddrSpace::NewPageTable()
{
    memLock->Acquire();

    
    
    PageTableEntry * newPT = new PageTableEntry[numPages + (UserStackSize / PageSize)]; // add 8 pages for new stack
    
    for(int i = 0; i < numPages; i++)
    {
        newPT[i].virtualPage    =   pageTable[i].virtualPage;
        newPT[i].physicalPage   =   pageTable[i].physicalPage;
        newPT[i].valid          =   pageTable[i].valid;
        newPT[i].use            =   pageTable[i].use;
        newPT[i].dirty          =   pageTable[i].dirty;
        newPT[i].readOnly       =   pageTable[i].readOnly;

        ipt[i].virtualPage    =   pageTable[i].virtualPage;
        ipt[i].physicalPage   =   pageTable[i].physicalPage;
        ipt[i].valid          =   pageTable[i].valid;
        ipt[i].use            =   pageTable[i].use;
        ipt[i].dirty          =   pageTable[i].dirty;
        ipt[i].readOnly       =   pageTable[i].readOnly;
        ipt[i].space          =   this;

    }

    for(int i = numPages; i < numPages + (UserStackSize / PageSize); i++)
    {
        newPT[i].virtualPage = i;   // for now, virtual page # = phys page #
        newPT[i].physicalPage = memBitMap->Find();
        newPT[i].valid = TRUE;
        newPT[i].use = FALSE;
        newPT[i].dirty = FALSE;
        newPT[i].readOnly = FALSE;  
    }

        /*ipt[i].virtualPage = i;   // for now, virtual page # = phys page #
        ipt[i].physicalPage = memBitMap->Find();
        ipt[i].valid = TRUE;
        ipt[i].use = FALSE;
        ipt[i].dirty = FALSE;
        // if the code segment was entirely on 
        // a separate page, we could set its 
        // pages to be read-only

        // find page memory that nobody is using
        // copy from executable to that page of memory
        // how much to copy? pagesize!
        if (newPT[i].physicalPage == -1)
        {
          printf("No more physical memory available.\n");
          interrupt->Halt();
        }
    }*/

    delete pageTable;

    pageTable = newPT;
    numPages += (UserStackSize / PageSize);

    RestoreState();

    memLock->Release();

    return numPages;
}

/*
- A thread calls Exit - not the last executing thread in the process
  - Reclaim 8 pages of stock
  - VPN, PPN, valid = false
      - memoryBitMap->Clear(ppn);
- Last executing thread in last process
    - interrupt->Halt();
- Last executing thread in a process - not last process (AddrSpace *)
    - reclaim all unreclaimed memory
    - Locks/CVs (match AddrSpace * w/ ProcessTable)
*/

void AddrSpace::ReclaimStack(int stackPage)
{
    memLock->Acquire();
    for(int i = stackPage; i < (UserStackSize / PageSize); i++)
    {
        memBitMap->Clear(pageTable[i].physicalPage);
        pageTable[i].valid = FALSE;
        pageTable[i].use = FALSE;
        pageTable[i].dirty = FALSE;
        pageTable[i].readOnly = FALSE;  
    }
    memLock->Release();
}

void AddrSpace::ReclaimPageTable()
{
    memLock->Acquire();
    for(int i = 0; i < numPages; i++)
    {
        if(pageTable[i].valid)
        {
            memBitMap->Clear(pageTable[i].physicalPage);
            pageTable[i].valid = FALSE;
            pageTable[i].use = FALSE;
            pageTable[i].dirty = FALSE;
            pageTable[i].readOnly = FALSE;  
        }
    }
    memLock->Release();
}


