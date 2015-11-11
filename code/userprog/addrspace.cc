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

//----------------------------------------------------------------------
// Table::Table
//  Initialize an array with a lock synchronizing its updates.
//
//  "s" -- the size of the table
//----------------------------------------------------------------------

Table::Table(int s) : map(s), table(0), lock(0), size(s)
{
    table = new void *[size];
    lock = new Lock("TableLock");
}

//----------------------------------------------------------------------
// Table::Table
//  De-allocate Table's array and lock.
//----------------------------------------------------------------------

Table::~Table()
{
    if (table)
    {
	   delete table;
	   table = 0;
    }
    if (lock)
    {
	   delete lock;
	   lock = 0;
    }
}

//----------------------------------------------------------------------
// Table::Get
//  Return the element associated with the given if, or 0 if there is 
//  none.
//
//  "i" -- the element's table index
//----------------------------------------------------------------------

void *
Table::Get(int i)
{
    // table index is nonnegative, within range and table's map bit set
    return (i >= 0 && i < size && map.Test(i)) ? table[i] : 0;
}

//----------------------------------------------------------------------
// Table::Put
//  Put the element in the table and return the slot it used. Use a lock
//  so 2 files don't get the same space.
//
//  Returns -1 if no available Table entries, else the index for element
//
//  "f" -- Element to be put inside the table
//----------------------------------------------------------------------

int 
Table::Put(void *f)
{
    int row;

    lock->Acquire();
    row = map.Find();
    lock->Release();
    
    if (row != -1)
        table[row] = f;

    return row;
}

//----------------------------------------------------------------------
// Table::Remove
//  Remove the element associated with identifier i from the table, and
//  return it.
//
//  Returns 0 if no matching element to remove, else the element.
//
//  "i" -- the element's table index
//----------------------------------------------------------------------

void *
Table::Remove(int i)
{
    void *f = 0;

    // Element inside of table: Remove it
    if (Get(i) != 0)
    {
        lock->Acquire();
        map.Clear(i);
        f = table[i];
        table[i] = 0;
        lock->Release();
    }

    return f;
}

//----------------------------------------------------------------------
// PageTableEntry::PageTableEntry
//  Initialize an empty Page Table Entry.
//----------------------------------------------------------------------

PageTableEntry::PageTableEntry()
{
    virtualPage = -1;
    physicalPage = -1;
    offset = -1;
    readOnly = false;
    valid = false;
    swapped = false;
    dirty = false;
    use = false;
}

//----------------------------------------------------------------------
// PageTableEntry::~PageTableEntry
//  Deallocate Page Table Entry.
//----------------------------------------------------------------------

PageTableEntry::~PageTableEntry()
{
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
// 	Create an address space to run a user program. While Memory is avail
//  load the program from a file "executable" into Main Memory and 
//  update the IPT; else, store the location in the Executable of the 
//  virtual page in the Page Table.
//
//	Assumes that the object code file is in NOFF format.
//
//	"executable" -- the file containing object code to load into memory
//
//  NOTE:   It's possible to fail to fully construct the address space 
//          for several reasons, including being unable to allocate 
//          memory, and being unable to read key parts of the 
//          executable. Incompletely consructed address spaces have the 
//          member constructed set to false.
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executableFile) : fileTable(MaxOpenFiles) 
{
    executable = executableFile; //openfile pointer

    NoffHeader noffH;
    unsigned int vpn, size;
    int offset;

    // Make sure Big Endian
    executable->ReadAt((char *)&noffH, sizeof(noffH), 0); // Get noffHeader
    if ((noffH.noffMagic != NOFFMAGIC) && (WordToHost(noffH.noffMagic) == NOFFMAGIC))
    {
        SwapHeader(&noffH);
    }

    // Must be Nachos object code file
    ASSERT(noffH.noffMagic == NOFFMAGIC);

    // Get size of the PageTable: Complete pages for code, data, stack and heap
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size;
    numPages = divRoundUp(size, PageSize) + divRoundUp(UserStackSize, PageSize);
    size = numPages * PageSize;
    // we need to increase the size
    // to leave room for the stack

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", numPages, size);
    
    // Store the location of each virtual page in the Page Table
    pageTable = new PageTableEntry[numPages];
    for (vpn = 0; vpn < numPages - divRoundUp(UserStackSize, PageSize) - divRoundUp(noffH.uninitData.size, PageSize); vpn++) 
    {
        offset = noffH.code.inFileAddr + (vpn * PageSize);

        pageTable[vpn].virtualPage = vpn;
        pageTable[vpn].physicalPage = -1;
        pageTable[vpn].offset = offset;
        pageTable[vpn].readOnly = false;
        pageTable[vpn].valid = false;
        pageTable[vpn].swapped = false;
        pageTable[vpn].dirty = false;
        pageTable[vpn].use = false;
    }

    for(vpn = numPages - divRoundUp(UserStackSize, PageSize) - divRoundUp(noffH.uninitData.size, PageSize); vpn < numPages; vpn++)
    {
        pageTable[vpn].virtualPage = vpn;
        pageTable[vpn].physicalPage = -1;
        pageTable[vpn].offset = -1;
        pageTable[vpn].readOnly = false;
        pageTable[vpn].valid = false;
        pageTable[vpn].swapped = false;
        pageTable[vpn].dirty = false;
        pageTable[vpn].use = false;
    }
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space. Release pages, page tables, files and 
//  file tables
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    delete pageTable;
}

//----------------------------------------------------------------------
// LoadIntoMemory
//  Store needed page in Main Memory. We do this by:
//  (1) Figuring out the location of the Memory Page (Swap File or Exec)
//  (2) Loading from the file to Main Memory
//  (3) Updating the Page Table to reflect the location change
//
//  "vpn" -- to be converted to the starting page within the file
//  "ppn" -- the physical page in memory to load into
//----------------------------------------------------------------------

void AddrSpace::LoadIntoMemory(int vpn, int ppn)
{
    DEBUG('p', "LoadIntoMemory: CurrentThread = %s, VPN = %d\n", 
        currentThread->getName(), vpn);

    if (pageTable[vpn].swapped)
    {
        DEBUG('p', "LoadFromSwap: Load vaddr %d into Main Memory:\n\tvpn\t%d\n \tppn\t%d\n \toffset\t%d\n",
            (vpn * PageSize), vpn, ppn, pageTable[vpn].offset);

        // TODO: Load from Swap file
        // (2) Load from Swap File into Main Memory
        swapFile->ReadAt(&(machine->mainMemory[ppn * PageSize]), // Store into mainMemory at physical page
                            PageSize, // Read 128 bytes
                            pageTable[vpn].offset); // From this position in swap file

        // TODO: Clear Swap File entry
        swapBitMap->Clear(pageTable[vpn].offset / PageSize);
        pageTable[vpn].swapped = false;
        pageTable[vpn].offset = -1;
    }
    else if (pageTable[vpn].offset != -1)
    {
        DEBUG('p', "LoadFromExecutable: Load vaddr %d into Main Memory:\n\tvpn\t%d\n \tppn\t%d\n \toffset\t%d\n" ,
            (vpn * PageSize), vpn, ppn, pageTable[vpn].offset);

        executable->ReadAt(
             &(machine->mainMemory[ppn * PageSize]), // Store into mainMemory at physical page
             PageSize, // Read 128 bytes
             pageTable[vpn].offset); // From this position in executable
    }
    // Offset should be -1 only when neither in Executable nor Swap (offset = vaddr in file)
    // This is the case for unused Stack Pages
    else
    {       
        
        DEBUG('p', "Loading unused Stack Page to Memory\n");
        // (2) Clear Memory
        //bzero(machine->mainMemory[ppn * PageSize], PageSize);
    }

    // (3) Update the Page Table
    pageTable[vpn].physicalPage = ppn;
    pageTable[vpn].valid = true;

    // (5) Update the Inverted Page Table
    ipt[ppn].virtualPage = vpn;
    ipt[ppn].physicalPage = ppn;
    ipt[ppn].space = this;
    ipt[ppn].readOnly = pageTable[vpn].readOnly;
    ipt[ppn].valid = true;
    ipt[ppn].dirty = false;
    ipt[ppn].use = pageTable[vpn].use;

}

void AddrSpace::RemoveFromMemory(int vpn, int ppn)
{
    DEBUG('p', "RemoveFromMemory: CurrentThread = %s\n", currentThread->getName());

    // TODO: save to swap file
    if(ipt[ppn].dirty)
    {
        int swapPage = swapBitMap->Find();

        ASSERT(swapPage != -1);
    
        swapFile->WriteAt(&(machine->mainMemory[ppn * PageSize]), // Store into mainMemory at physical page
                            PageSize, // Read 128 bytes
                            swapPage * PageSize);

        pageTable[vpn].offset = swapPage * PageSize;
        pageTable[vpn].swapped = true;

        DEBUG('p', "SaveToSwap: Save vaddr %d into Swap:\n \t\n \tvpn\t%d\n \tppn\t%d\n \toffset\t%d\n" ,
                    (vpn * PageSize), vpn, ppn, pageTable[vpn].offset);
    }
    
    // Update the Page Table
    pageTable[vpn].physicalPage = -1;
    pageTable[vpn].valid = false;
}


//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so that we can 
//  immediately jump to user code. Note that these will be saved/
//  restored into the currentThread->userRegisters when this thread is 
//  context switched out.
//
//  Returns the stack page.
//----------------------------------------------------------------------

int AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
        machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because of 
    //  branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

    // Set the stack register to the end of the address space, where we
    //  allocated the stack; but subtract off a bit, to make sure we 
    //  don't accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);

    DEBUG('a', "Initializing stack register to %x\n", numPages * PageSize - 16);

    return numPages - divRoundUp(UserStackSize, PageSize);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific to this 
//  address space, that needs saving and invalidate the TLB entries.
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{
    int tlbEntry;

    IntStatus oldLevel = interrupt->SetLevel(IntOff); // Disable interrupts
    
    // The dirty bit tells us if memory has been written do during its time 
    // in TLB. If it has been, then we need to propogate to the IPT.
    
    for (tlbEntry = 0; tlbEntry < TLBSize; tlbEntry++)
    {
        if (machine->tlb[tlbEntry].dirty && machine->tlb[tlbEntry].valid)
        {
            // Propagate changes to IPT 
            ipt[machine->tlb[tlbEntry].physicalPage].dirty = true;
        }

        machine->tlb[tlbEntry].valid = false;
    }

    (void) interrupt->SetLevel(oldLevel); // Restore interrupts
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that this address 
//  space can run.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
#ifdef USE_TLB

#else

    // Load this process' paging information into processor
    // Can only have pageTable OR TLB; Not both.
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;

#endif // USE_TLB
}

//----------------------------------------------------------------------
// AddrSpace::NewUserStack
//  Adds an additional stack to the thread's AddrSpace by creating a new
//  Page Table UserStackSize pages bigger. Will attempt to fill any 
//  available Memory, otherwise will just store 
//
//  Returns the starting page for the new stack.
//----------------------------------------------------------------------

int AddrSpace::NewUserStack()
{
    int vpn;

    // New PageTable = Old PageTable + 8 pages for new stack
    PageTableEntry * newPT = new PageTableEntry[numPages + divRoundUp(UserStackSize, PageSize)];
    
    // Copy old entries
    memcpy(newPT, pageTable, numPages);

    // Add 8 Stack Entries
    for (vpn = numPages; vpn < numPages + divRoundUp(UserStackSize, PageSize); vpn++)
    {
        // If Stack Reg not in Memory, starts out nowhere until demanded
        pageTable[vpn].virtualPage = vpn;
        pageTable[vpn].physicalPage = -1;
        pageTable[vpn].offset = -1;
        pageTable[vpn].readOnly = false;
        pageTable[vpn].valid = false;
        pageTable[vpn].swapped = false;
        pageTable[vpn].dirty = false;
        pageTable[vpn].use = false;
    }

    delete pageTable;

    pageTable = newPT;
    numPages += divRoundUp(UserStackSize, PageSize);

    RestoreState(); // Machine needs to know about new numPages and Page Table

    return numPages; // Starting page for stack (grows up)
}

//----------------------------------------------------------------------
// AddrSpace::ReclaimStack
//  When a thread calls Exit, there are three cases:
//  (1) Other threads still executing in the process
//  (2) Last executing thread in process, other Nachos threads running
//  (3) Last executing thread in Nachos
//
//  ReclaimStack handles the first case by reclaiming 8 pages of stack
//  by (1) invalidating the stack pages (VPN, PPN: valid = false) and
//  (2) clearing physical memory so it can be reused.
//----------------------------------------------------------------------

void AddrSpace::ReclaimStack(int stackPage)
{
    int vpn, tlbEntry;

    IntStatus oldLevel = interrupt->SetLevel(IntOff); // Disable interrupts

    // TODO: Why disable interrupts and acquire a lock? seems like we only need to do one of these things... 
    // if the lock is already acquired and we disable interrupts we will get DeadLock

    // (2) Clear physical memory so it can be reused
    //memLock->Acquire();
    for (vpn = stackPage; vpn < stackPage + divRoundUp(UserStackSize, PageSize); vpn++)
    {
        if(pageTable[vpn].valid)
        {
            ipt[pageTable[vpn].physicalPage].valid = false;
            for(tlbEntry = 0; tlbEntry < TLBSize; tlbEntry++)
            {
                if(machine->tlb[tlbEntry].valid && machine->tlb[tlbEntry].virtualPage == vpn && ipt[machine->tlb[tlbEntry].physicalPage].space == this)
                {
                    machine->tlb[tlbEntry].valid = false;
                }
            }

            memBitMap->Clear(pageTable[vpn].physicalPage);
        }

        // TODO: check swap file?
        if(pageTable[vpn].swapped)
        {
            swapBitMap->Clear(pageTable[vpn].offset / PageSize);
        }

        pageTable[vpn].virtualPage = -1;
        pageTable[vpn].physicalPage = -1;
        pageTable[vpn].offset = -1;
        pageTable[vpn].readOnly = false;
        pageTable[vpn].valid = false;
        pageTable[vpn].swapped = false;
        pageTable[vpn].dirty = false;
        pageTable[vpn].use = false;
    }
    //memLock->Release();

    interrupt->SetLevel(oldLevel); // Restore interrupts
}

//----------------------------------------------------------------------
// AddrSpace::ReclaimPageTable
//  When a thread calls Exit, there are three cases:
//  (1) Other threads still executing in the process
//  (2) Last executing thread in process, other Nachos threads running
//  (3) Last executing thread in Nachos
//
//  ReclaimPageTable handles the second case by reclaiming the entire 
//  process' Page Table by (1) invalidating entire page table and
//  (2) clearing physical memory so it can be reused.
//----------------------------------------------------------------------
void AddrSpace::ReclaimPageTable()
{
    int vpn, tlbEntry;

    IntStatus oldLevel = interrupt->SetLevel(IntOff); // Disable interrupts

    // TODO: Why disable interrupts and acquire a lock? seems like we only need to do one of these things... 
    // if the lock is already acquired and we disable interrupts we will get DeadLock

    // (2) Clear physical memory so it can be reused
    //memLock->Acquire();
    for(vpn = 0; vpn < numPages; vpn++)
    {
        if(pageTable[vpn].valid)
        {
            ipt[pageTable[vpn].physicalPage].valid = false;
            for(tlbEntry = 0; tlbEntry < TLBSize; tlbEntry++)
            {
                if(machine->tlb[tlbEntry].valid && machine->tlb[tlbEntry].virtualPage == vpn && ipt[machine->tlb[tlbEntry].physicalPage].space == this)
                {
                    machine->tlb[tlbEntry].valid = false;
                }
            }

            memBitMap->Clear(pageTable[vpn].physicalPage);
        }

        if(pageTable[vpn].swapped)
        {
            swapBitMap->Clear(pageTable[vpn].offset / PageSize);
        }

        pageTable[vpn].virtualPage = -1;
        pageTable[vpn].physicalPage = -1;
        pageTable[vpn].offset = -1;
        pageTable[vpn].readOnly = false;
        pageTable[vpn].valid = false;
        pageTable[vpn].swapped = false;
        pageTable[vpn].dirty = false;
        pageTable[vpn].use = false; 
    }
    //memLock->Release();

    interrupt->SetLevel(oldLevel); // Restore interrupts

    // (1) Invalidate entire Page Table
    delete pageTable;
}


