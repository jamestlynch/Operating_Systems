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
    // TODO: this doesn't seem correct
    delete this;
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

    IntStatus oldLevel = interrupt->SetLevel(IntOff); // Disable interrupts

    NoffHeader noffH;
    unsigned int vpn, ppn, size;
    int offset;
    bool readOnly = false, valid = false;

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

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", numPages, size);
    
    // Store the location of each virtual page in the Page Table
    //  Memory available: Load into memory
    //  Memory full: Track location in executable
    pageTable = new PageTableEntry[numPages];
    for (vpn = 0; vpn < numPages; vpn++) 
    {
        // Code is read-only
        /*if ((vpn * PageSize) < (unsigned int)noffH.initData.inFileAddr)
        {
            readOnly = true;
        }

        memLock->Acquire();*/


        // TODO: Do not use memBitMap->Find() here, in fact, remove all code below
        // Find available Memory, if any.
        // TODO: Change back to memBitMap->Find(), Causing IPT Miss to load from Executable without Mem being full.
        /*ppn = -1; // memBitMap->Find();

        // Memory is available: Load into Memory
        if (ppn != -1)
        {
            memFIFO->Append((void *)ppn); // Maintain order of Adding to Memory for FIFO Eviction

            valid = true;
            offset = ppn * PageSize;

            // Update IPT whenever adding/removing from Memory
            ipt[ppn].virtualPage = vpn;
            ipt[ppn].physicalPage = ppn;
            ipt[ppn].valid = true;
            ipt[ppn].dirty = false;
            ipt[ppn].use = false;
            ipt[ppn].readOnly = readOnly;
            ipt[ppn].space = this;

            // Load from Executable into Main Memory
            executable->ReadAt(
                &(machine->mainMemory[ppn * PageSize]), // Store into mainMemory at physical page
                PageSize, // Read 128 bytes
                noffH.code.inFileAddr + vpn * PageSize); // From this position in executable

            DEBUG('p', "AddrSpace PageTable: Load Executable into Main Memory:\n \tdata\t%d\n \tvpn\t\t%d\n \tppn\t\t%d\n",
                machine->mainMemory[ppn * PageSize], vpn, ppn);
        }
        // Memory full: Store position in executable for later retrieval*/
        //else
        //{
            offset = noffH.code.inFileAddr + vpn * PageSize;

           /* DEBUG('p', "AddrSpace PageTable: Memory Full, Page beginning at vaddr %d will need to be loaded from executable:\n \tvaddr\t%d\n \tvpn\t\t%d\n \tppn\t\t%d\n",
                offset, offset, vpn, ppn);
            interrupt->Halt();*/
        //}
        //memLock->Release();
        // TODO: Set Valid bit to false
        pageTable[vpn].virtualPage = vpn;
        pageTable[vpn].physicalPage = ppn;
        pageTable[vpn].offset = offset;
        pageTable[vpn].readOnly = readOnly;
        pageTable[vpn].valid = false;
        pageTable[vpn].swapped = false;
        pageTable[vpn].dirty = false;
        pageTable[vpn].use = false;
    }
    // TODO: Store executable in pointer
    // Store Open Executable so on-demand memory can take place
    //fileTable.Put(executable);

    //add openfile pointer to address space class

    DEBUG('p', "AddrSpace: CurrentThread = %s\n", currentThread->getName());

    /*int execLength = executable->Length();

    OpenFile *exec = (OpenFile *)fileTable.Get(0);
    DEBUG('p', "AddrSpace: Executable Length = %d, fileTable[0]->Length() = %d\n", execLength, exec->Length());*/

    (void) interrupt->SetLevel(oldLevel); // Restore interrupts
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


void
AddrSpace::LoadIntoMemory(unsigned int vpn, unsigned int ppn)
{
    executable->ReadAt(
        &(machine->mainMemory[ppn * PageSize]), // Store into mainMemory at physical page
        PageSize, // Read 128 bytes
        currentThread->space->pageTable[vpn].offset); // From this position in executable
    //TODO: Loading from executable or swap file?
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

int
AddrSpace::InitRegisters()
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

    return numPages - (UserStackSize / PageSize);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific to this 
//  address space, that needs saving and invalidate the TLB entries.
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{
    unsigned int vpn;

    IntStatus oldLevel = interrupt->SetLevel(IntOff); // Disable interrupts
    
    // TODO: this should be checking the dirty bit in TLB correct? Yeah, this isn't correct

    // The dirty bit tells us if memory has been written do during its time 
    //  in main memory. If it has been, then once we discard the memory we 
    //  must write it back to disk.
    for (vpn = 0; vpn < numPages; vpn++)
    {
        if (pageTable[vpn].dirty)
        {
            // TODO: Propagate changes to PageTable 
            //  Either Swap File -> Executable or Memory -> Executable
            //  WriteToExecutable(vpn);
        }
    }

    (void) interrupt->SetLevel(oldLevel); // Restore interrupts
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that this address 
//  space can run.
//----------------------------------------------------------------------

void
AddrSpace::RestoreState() 
{
    unsigned int i, ppn;

    IntStatus oldLevel = interrupt->SetLevel(IntOff); // Disable interrupts
    
    // TODO: Why invalidate all pages i main memory not belonging to my process?
    // Invalidate all Pages in Main Memory not belonging to my Process
    for (ppn = 0; ppn < NumPhysPages; ppn++)
    {
        if (ipt[ppn].space == this)
        {
            ipt[ppn].valid = false;
        }
    }
    
#ifdef USE_TLB

    // Invalidate all TLB pages on context switch.
    for (i = 0; i < TLBSize; i++)
    {
        machine->tlb[i].valid = false;
    }

#else

    // Load this process' paging information into processor
    //  Can only have pageTable OR TLB; Not both.
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;

#endif // USE_TLB

    (void) interrupt->SetLevel(oldLevel); // Restore interrupts
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
//  Adds an additional stack to the thread's AddrSpace by creating a new
//  Page Table UserStackSize pages bigger. Will attempt to fill any 
//  available Memory, otherwise will just store 
//
//  Returns the starting page for the new stack.
//----------------------------------------------------------------------

int
AddrSpace::NewUserStack()
{
    unsigned int vpn, ppn;
    int offset = -1; // Start out nowhere until demanded
    bool readOnly = false, valid = false;
    
    // TODO: Why disable interrupts and acquire a lock? seems like we only need to do one of these things... 
    // if the lock is already acquired and we disable interrupts we will get DeadLock


    IntStatus oldLevel = interrupt->SetLevel(IntOff); // Disable interrupts

    // New PageTable = Old PageTable + 8 pages for new stack
    PageTableEntry * newPT = new PageTableEntry[numPages + (UserStackSize / PageSize)];
    
    // Copy old entries
    memcpy(newPT, pageTable, numPages);

    // Add 8 Stack Entries
    for (vpn = numPages; vpn < numPages + (UserStackSize / PageSize); vpn++)
    {
        memLock->Acquire();

        // Memory Available: Load into Memory
        if (ppn = memBitMap->Find() != -1)
        {
            memFIFO->Append((void *)ppn); // Maintain order of Adding to Memory for FIFO Eviction

            offset = ppn * PageSize;
            valid = true;

            // Update IPT whenever adding/removing from Memory
            ipt[ppn].virtualPage = vpn;
            ipt[ppn].physicalPage = ppn;
            ipt[ppn].valid = true;
            ipt[ppn].dirty = false;
            ipt[ppn].use = false;
            ipt[ppn].readOnly = readOnly;
            ipt[ppn].space = this;

            DEBUG('p', "AddrSpace PageTable (NewUserStack): Load Stack Pages into Main Memory:\n \tvpn\t\t%d\n \tppn\t\t%d\n",
                vpn, ppn);
        }
        memLock->Release();

        // If Stack Reg not in Memory, starts out nowhere until demanded

        pageTable[vpn].virtualPage = vpn;
        pageTable[vpn].physicalPage = ppn;
        pageTable[vpn].offset = offset;
        pageTable[vpn].readOnly = readOnly;
        pageTable[vpn].valid = valid;
        pageTable[vpn].swapped = false;
        pageTable[vpn].dirty = false;
        pageTable[vpn].use = false;
    }

    delete pageTable;

    pageTable = newPT;
    numPages += (UserStackSize / PageSize);

    RestoreState(); // Machine needs to know about new numPages and Page Table

    (void) interrupt->SetLevel(oldLevel); // Restore interrupts

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

void
AddrSpace::ReclaimStack(int stackPage)
{
    unsigned int vpn, ppn;

    IntStatus oldLevel = interrupt->SetLevel(IntOff); // Disable interrupts

    // TODO: Why disable interrupts and acquire a lock? seems like we only need to do one of these things... 
    // if the lock is already acquired and we disable interrupts we will get DeadLock

    // (1) Invalidate stack pages    
    for (vpn = stackPage; vpn < stackPage + (UserStackSize / PageSize); vpn++)
    {
        pageTable[vpn].valid = false;
        pageTable[vpn].use = false;
    }

    // TODO: You shouldn't check if they're the same space but if they're the same VPN... There could be other pages from this addrspace in the IPT
    // TODO: What about checking if it's in the TLB?
    // (2) Clear physical memory so it can be reused
    memLock->Acquire();
    for (ppn = 0; ppn < NumPhysPages; ppn++)
    {
        if (ipt[ppn].space == this)
        {
            ipt[ppn].valid = false;

            memBitMap->Clear(ppn);
        }
    }
    memLock->Release();

    (void) interrupt->SetLevel(oldLevel); // Restore interrupts
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

void 
AddrSpace::ReclaimPageTable()
{
    unsigned int ssn, ppn;

    IntStatus oldLevel = interrupt->SetLevel(IntOff); // Disable interrupts

    // TODO: Why disable interrupts and acquire a lock? seems like we only need to do one of these things... 
    // if the lock is already acquired and we disable interrupts we will get DeadLock
    // TODO: What about checking if it's in the TLB?

    // (2) Clear physical memory so it can be reused
    memLock->Acquire(); 
    for (ppn = 0; ppn < NumPhysPages; ppn++)
    {
        if (ipt[ppn].space == this)
        {
            ipt[ppn].valid = false;

            memBitMap->Clear(ppn);
        }
    }
    memLock->Release();

    (void) interrupt->SetLevel(oldLevel); // Restore interrupts

    // (1) Invalidate entire Page Table
    delete pageTable;
}


