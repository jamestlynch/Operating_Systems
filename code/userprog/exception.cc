// exception.cc 
//  Entry point into the Nachos kernel from user programs.
//  There are two kinds of things that can cause control to
//  transfer back to here from user code:
//
//  syscall -- The user code explicitly requests to call a procedure
//  in the Nachos kernel.  Right now, the only function we support is
//  "Halt".
//
//  exceptions -- The user code does something that the CPU can't handle.
//  For instance, accessing memory that doesn't exist, arithmetic errors,
//  etc.  
//
//  Interrupts (which can also cause control to transfer from user
//  code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include <stdio.h>
#include <iostream>
#include <string>

using namespace std;

int copyin(unsigned int vaddr, int len, char *buf) 
{
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n = 0;      // The number of byts copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) 
    {
      result = machine->ReadMem( vaddr, 1, paddr );
      while(!result) // FALL 09 CHANGES
      {
        result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
      } 
      
      buf[n++] = *paddr;
     
      if ( !result ) 
      {
        //translation failed
        return -1;
      }

      vaddr++;
    }
    delete paddr;
    return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes to the current thread's virtual address vaddr.
    // Return the number of bytes so written, or -1 if an error
    // occors.  Errors can generally mean a bad virtual address was
    // passed in.
    bool result;
    int n=0;      // The number of bytes copied in

    while ( n >= 0 && n < len) {
      // Note that we check every byte's address
      result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );

      if ( !result ) {
  //translation failed
  return -1;
      }

      vaddr++;
    }

    return n;
}


void Create_Syscall(unsigned int vaddr, int len) {
    // Create the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  No
    // way to return errors, though...
    char *buf = new char[len + 1];  // Kernel buffer to put the name in

    if (!buf) return;

    if( copyin(vaddr,len,buf) == -1 ) {
  printf("%s","Bad pointer passed to Create\n");
  delete buf;
  return;
    }

    buf[len]='\0';

    fileSystem->Create(buf,0);
    delete[] buf;
    return;
}

int Open_Syscall(unsigned int vaddr, int len) {
    // Open the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  If
    // the file is opened successfully, it is put in the address
    // space's file table and an id returned that can find the file
    // later.  If there are any errors, -1 is returned.
    char *buf = new char[len+1];  // Kernel buffer to put the name in
    OpenFile *f;      // The new open file
    int id;       // The openfile id

    if (!buf) {
  printf("%s","Can't allocate kernel buffer in Open\n");
  return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
  printf("%s","Bad pointer passed to Open\n");
  delete[] buf;
  return -1;
    }

    buf[len]='\0';

    f = fileSystem->Open(buf);
    delete[] buf;

    if ( f ) {
  if ((id = currentThread->space->fileTable.Put(f)) == -1 )
      delete f;
  return id;
    }
    else
  return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one. For disk files, the file is looked
    // up in the current address space's open file table and used as
    // the target of the write.
    
    char *buf;    // Kernel buffer for output
    OpenFile *f;  // Open file for output

    if ( id == ConsoleInput) return;
    
    if ( !(buf = new char[len]) ) {
  printf("%s","Error allocating kernel buffer for write!\n");
  return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
      printf("%s","Bad pointer passed to to write: data not written\n");
      delete[] buf;
      return;
  }
    }

    if ( id == ConsoleOutput) {
      for (int ii=0; ii<len; ii++) {
  printf("%c",buf[ii]);
      }
    } else {
  if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
      f->Write(buf, len);
  } else {
      printf("%s","Bad OpenFileId passed to Write\n");
      len = -1;
  }
    }

    delete[] buf;
}

void WriteInt_Syscall(int integer) {
  printf("%d", integer);
}

int Random(int lower, int upper){
  srand(time(0));
  int randomnumber = rand() % upper + lower ;
  return randomnumber;
}

int WriteOne(int vaddr, int len, int num1)
{
    if (len < 0) 
    { // Validate length is nonzero and positive
      printf("%s","Length for lock's identifier name must be nonzero and positive\n");
      locksLock->Release();
      return -1;
    }

    char * buf = new char[len + 1];

    if ( !buf ) 
    { // If error allocating memory for character buffer
      printf("%s","Error allocating kernel buffer for creating new lock!\n");
      locksLock->Release();
      return -1;
    } 
    
    if ( copyin(vaddr, len, buf) == -1 ) 
    { // If failed to read memory from vaddr passed in
      printf("%s","Bad pointer passed to create new lock\n");
      locksLock->Release();
      delete[] buf;
      return -1;
    }

    buf[len] = '\0'; //Finished grabbing the identifier for the Lock, add null terminator character
    
    printf(buf, num1);
    return 0;
}

int WriteTwo(unsigned int vaddr, int len, int num1, int num2)
{
    if (len < 0) 
    { // Validate length is nonzero and positive
      printf("%s","Length for lock's identifier name must be nonzero and positive\n");
      locksLock->Release();
      return -1;
    }

    char * buf = new char[len + 1];

    if ( !buf ) 
    { // If error allocating memory for character buffer
      printf("%s","Error allocating kernel buffer for creating new lock!\n");
      locksLock->Release();
      return -1;
    } 
    
    if ( copyin(vaddr, len, buf) == -1 ) 
    { // If failed to read memory from vaddr passed in
      printf("%s","Bad pointer passed to create new lock\n");
      locksLock->Release();
      delete[] buf;
      return -1;
    }

    buf[len] = '\0'; //Finished grabbing the identifier for the Lock, add null terminator character
    printf(buf, num1, num2);
    return 0;
}



#define RED               "\x1b[31m"
#define ANSI_COLOR_RESET  "\x1b[0m"
void WriteError_Syscall(unsigned int vaddr, int len) {
  printf(RED);  
  Write_Syscall(vaddr, len, 1);
  printf(ANSI_COLOR_RESET);
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one.    We reuse len as the number of bytes
    // read, which is an unnessecary savings of space.
    char *buf;    // Kernel buffer for input
    OpenFile *f;  // Open file for output

    if ( id == ConsoleOutput) return -1;
    
    if ( !(buf = new char[len]) ) {
  printf("%s","Error allocating kernel buffer in Read\n");
  return -1;
    }

    if ( id == ConsoleInput) {
      //Reading from the keyboard
      scanf("%s", buf);

      if ( copyout(vaddr, len, buf) == -1 ) {
  printf("%s","Bad pointer passed to Read: data not copied\n");
      }
    } else {
  if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
      len = f->Read(buf, len);
      if ( len > 0 ) {
          //Read something from the file. Put into user's address space
            if ( copyout(vaddr, len, buf) == -1 ) {
        printf("%s","Bad pointer passed to Read: data not copied\n");
    }
      }
  } else {
      printf("%s","Bad OpenFileId passed to Read\n");
      len = -1;
  }
    }

    delete[] buf;
    return len;
}

void Close_Syscall(int fd) {
    // Close the file associated with id fd.  No error reporting.
    OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

    if ( f ) {
      delete f;
    } else {
      printf("%s","Tried to close an unopen file\n");
    }
}

// CreateLock()
//  Creates a lock object protected by the OS through this syscall.
//  Locks belong to a single process and we guarantee this by associating the 
//    program's address space.
//  CreateLock takes two parameters:
//    name – unsigned int pointer to the string identifier
//      * sizeOfName
//    sizeOfName – length of the string identifier since there is no null terminating byte
//      * sizeOfName should not be negative
//      * sizeOfName should be nonzero
//  CreateLock returns:
//    index value of the lock within the vector
//  
//  kernelLock Struct
//
//  TODO:
//    Validations:
//      Room in table (Probably doesn't apply)
//      Thread belong to same process as thread creator
int checkLockErrors(int index)
{
  if (index < 0) 
  {
    printf("%s","Invalid lock table index, negative.\n");
    return -1;
  }

  int size = locks.size();


  if (index > size - 1) 
  {
    printf("%s","Invalid lock table index, bigger than size.\n");
    return -1;
  }

  KernelLock * curKernelLock = locks.at(index);

  if (!curKernelLock)
  {
    printf("Lock %d is NULL.\n", index);
    return -1;
  }

  if (curKernelLock->space != currentThread->space) 
  {
    printf("Lock %d does not belong to the current process.\n", index);
    return -1;
  }

  return 0;
}

int CreateLock_Syscall(unsigned int vaddr, int len) 
{ //validate/load in from userprog memory as 'client'
  locksLock->Acquire();
  if (len <= 0) 
  { // Validate length is nonzero and positive
    printf("%s","Length for lock's identifier name must be nonzero and positive\n");
    locksLock->Release();
    return -1;
  }

  char * buf = new char[len + 1];

  if ( !buf ) 
  { // If error allocating memory for character buffer
    printf("%s","Error allocating kernel buffer for creating new lock!\n");
    locksLock->Release();
    return -1;
  } 
  
  if ( copyin(vaddr, len, buf) == -1 ) 
  { // If failed to read memory from vaddr passed in
    printf("%s","Bad pointer passed to create new lock\n");
    locksLock->Release();
    delete[] buf;
    return -1;
  }

  buf[len] = '\0'; //Finished grabbing the identifier for the Lock, add null terminator character
  
  KernelLock * newKernelLock = new KernelLock();

  Lock * lock = new Lock(buf);  
  newKernelLock->toDelete = false;
  newKernelLock->space = currentThread->space;
  newKernelLock->lock = lock;

  //put the new kernel lock object into the lock table
  locks.push_back(newKernelLock);
  int lockIndex = locks.size() - 1;

  locksLock->Release();

  return lockIndex; // TODO: Return the index of the lock
}

int AcquireLock(int index)
{
  if(checkLockErrors(index) == -1)
  {
    return -1;
  }

  if(locks.at(index)->toDelete)
  {
    printf("Cannot acquire lock because it's been marked for deletion."); 
    return -1;  
  }

  processLock->Acquire();
  processInfo.at(currentThread->processID)->numExecutingThreads--;
  processInfo.at(currentThread->processID)->numSleepingThreads++;
  processLock->Release();

  locks.at(index)->lock->Acquire();

  processLock->Acquire();
  processInfo.at(currentThread->processID)->numExecutingThreads++;
  processInfo.at(currentThread->processID)->numSleepingThreads--;
  processLock->Release();
  
  return index;
}

void DeleteLock(int indexlock)
{
  KernelLock * curKernelLock = locks.at(indexlock);

  delete curKernelLock->lock;
  delete curKernelLock;
  locks.at(indexlock) = NULL;

  printf("Lock %d was successfully deleted.\n", indexlock);
}

int ReleaseLock(int indexlock)
{
  if(checkLockErrors(indexlock) == -1)
  {
    return -1;
  }

  KernelLock * curKernelLock = locks.at(indexlock);
  curKernelLock->lock->Release();

  if (curKernelLock->toDelete && curKernelLock->lock->sleepqueue->IsEmpty() && !curKernelLock->lock->state)
  {
    locksLock->Acquire();
    DeleteLock(indexlock);
    locksLock->Release();
  }

  return 0;
}

int DestroyLock(int indexlock)
{
  locksLock->Acquire();

  if (indexlock < 0) 
  {
    printf("%s","Invalid index for destroy\n");
    locksLock->Release();
    return -1;
  }
  int size = locks.size();
  if (indexlock > size - 1) 
  {
    printf("%s","index out of bounds for destroy\n");
    locksLock->Release();
    return -1;
  }

  KernelLock * curKernelLock = locks.at(indexlock);

  if (!curKernelLock || !curKernelLock->lock)
  {
    printf("Lock struct or Lock var of index %d is null and can't be destroyed.\n", indexlock);
    locksLock->Release();
    return -1;
  }

  if (curKernelLock->space != currentThread->space) 
  {
    printf("Lock of index %d does not belong to the current process\n", indexlock);
    locksLock->Release();
    return -1;
  }

  if (curKernelLock->lock->sleepqueue->IsEmpty() && !curKernelLock->lock->state)
  {
    DeleteLock(indexlock);
    locksLock->Release();
    return 0;
  }

  printf("Lock %d toDelete is set to true. Cannot be deleted because sleepqueue is not empty.\n", indexlock);
  curKernelLock->toDelete = true;
  locksLock->Release();
  return -1;
}

int checkCVErrors(int indexcv, int indexlock)
{
  if (indexcv < 0 || indexlock < 0) 
  {
    printf("%s","Invalid index.\n");
    return -1;
  }
  int csize= conditions.size();
  int lsize= locks.size();
  if (indexcv > csize - 1 || indexlock > lsize - 1) 
  {
    printf("%s","Index out of bounds.\n");
    return -1;
  }

  KernelCV * curKernelCV = conditions.at(indexcv);
  KernelLock * curKernelLock = locks.at(indexlock);

  if (!curKernelCV  || !curKernelLock)
  {
    printf("Condition %d is set to NULL or Lock %d is set to NULL.\n", indexcv, indexlock);
    return -1;
  }

  if (curKernelLock->space != currentThread->space) 
  {
    printf("Lock %d does not belong to the current process.\n", indexlock);
    return -1;
  }

  if (curKernelCV->space != currentThread->space) 
  {
    printf("Condition %d does not belong to the current process.\n", indexcv);
    return -1;
  }

  return 0;
}

int CreateCV(unsigned int vaddr, int len)
{
  conditionsLock->Acquire();
  
  if (len <= 0) 
  { // Validate length is nonzero and positive
    printf("Invalid length for CV identifier\n");
    conditionsLock->Release();
    return -1;
  }

  char * buf = new char[len + 1];

  if ( !buf ) 
  { // If error allocating memory for character buffer
    printf("Error allocating kernel buffer for creating new CV!\n");
    conditionsLock->Release();
    return -1;
  } 
  
  if ( copyin(vaddr, len, buf) == -1 ) 
  { // If failed to read memory from vaddr passed in
    printf("Bad pointer passed to create new CV\n");
    delete[] buf;
    conditionsLock->Release();
    return -1;
  }

  buf[len] = '\0';
  
  KernelCV * newKernelCV = new KernelCV();
  newKernelCV->toDelete = false;
  newKernelCV->space = currentThread->space;
  newKernelCV->condition = new Condition(buf);

  conditions.push_back(newKernelCV);
  int conditionIndex = conditions.size() - 1;

  conditionsLock->Release();

  return conditionIndex;
}

int Wait(int indexcv, int indexlock)
{
  if(checkCVErrors(indexcv, indexlock) == -1)
  {
    return -1;
  }

  if(conditions.at(indexcv)->toDelete)
  {
    printf("Cannot wait on condition because it's been marked for deletion."); 
    return -1;  
  }

  processLock->Acquire();
  processInfo.at(currentThread->processID)->numExecutingThreads--;
  processInfo.at(currentThread->processID)->numSleepingThreads++;
  processLock->Release();

  conditions.at(indexcv)->condition->Wait(locks.at(indexlock)->lock);

  processLock->Acquire();
  processInfo.at(currentThread->processID)->numExecutingThreads++;
  processInfo.at(currentThread->processID)->numSleepingThreads--;
  processLock->Release();
  
  return 0;
}

void DeleteCondition(int indexcv)
{
  KernelCV * curKernelCV = conditions.at(indexcv);

  delete curKernelCV->condition;
  delete curKernelCV;
  conditions.at(indexcv) = NULL;

  printf("Condition %d was successfully deleted.\n", indexcv);
}

int Signal(int indexcv, int indexlock)
{
  if(checkCVErrors(indexcv, indexlock) == -1)
  {
    return -1;
  }

  KernelCV * curKernelCV = conditions.at(indexlock);

  curKernelCV->condition->Signal(locks.at(indexlock)->lock);



  if(curKernelCV->toDelete && curKernelCV->condition->waitqueue->IsEmpty())
  {
    conditionsLock->Acquire();
    DeleteCondition(indexcv);
    conditionsLock->Release();
  }
  
  return 0;
}

int Broadcast(int indexcv, int indexlock)
{
  if(checkCVErrors(indexcv, indexlock) == -1)
  {
    return -1;
  }

  KernelCV * curKernelCV = conditions.at(indexlock);
  curKernelCV->condition->Broadcast(locks.at(indexlock)->lock);

  if(curKernelCV->toDelete)
  {
    conditionsLock->Acquire();
    DeleteCondition(indexcv);
    conditionsLock->Release();
  }

  return 0;
}

int DestroyCV(int indexcv)
{
  conditionsLock->Acquire();

  if (indexcv < 0) 
  {
    printf("%s","Invalid index for destroy\n");
    conditionsLock->Release();
    return -1;
  }

  int size = conditions.size();

  if (indexcv > size - 1) 
  {
    printf("%s","index out of bounds for destroy\n");
    conditionsLock->Release();
    return -1;
  }

  KernelCV * curKernelCV = conditions.at(indexcv);

  if (!curKernelCV || !curKernelCV->condition)
  {
      printf("Condition struct or condition var of index %d is null and can't be destroyed.\n", indexcv);
      conditionsLock->Release();
      return -1;
  }

  if (curKernelCV->space != currentThread->space) 
  {
    printf("Condition of index %d does not belong to the current process\n", indexcv);
    conditionsLock->Release();
    return -1;
  }
   
  if (curKernelCV->condition->waitqueue->IsEmpty())
  {
    DeleteCondition(indexcv);
    conditionsLock->Release();
    return 0;
  }

  printf("Condition %d toDelete set to true. Cannot be deleted since waitqueue is not empty.\n", indexcv);
  curKernelCV->toDelete = true;
  conditionsLock->Release();
  return 0;
}

void Halt()
{
  interrupt->Halt();
}

void Yield_Syscall() 
{
  currentThread->Yield();
}


void Exit_Syscall(int status)
{
  if(status != 0)
  {
    printf("Arg must be 0 to exit.");
  }

  currentThread->Yield();
  processLock->Acquire();

  if(processInfo.at(currentThread->processID)->numExecutingThreads > 1) 
  {
    printf("Thread is not last in process. Finishing thread.\n");
    
    processInfo.at(currentThread->processID)->numExecutingThreads--;

    currentThread->space->ReclaimStack(currentThread->stackPage);

    processLock->Release();

    currentThread->Finish();

    return;
  }
  else if(processInfo.size() > 1)
  {
    printf("Thread is last in process. Cleaning up process.\n");
    int lsize=locks.size();
    for (int i = 0; i < lsize; i++)
    {
      if(locks.at(i))
      {
        if (locks.at(i)->space == currentThread->space)
        {
          locksLock->Acquire();
          DeleteLock(i);
          locksLock->Release();
        }
      }
    }
    int csize= conditions.size();
    for (int i = 0; i< csize; i++)
    {
      if(conditions.at(i))
      {
        if (conditions.at(i)->space == currentThread->space)
        {
          conditionsLock->Acquire();
          DeleteCondition(i);
          conditionsLock->Release();
        }
      }
    }
    currentThread->space->ReclaimPageTable();

    // Delete process
    Process * p = processInfo.at(currentThread->processID);
    delete p->space;
    delete p;
    processInfo.at(currentThread->processID) = NULL;

    processLock->Release();
    currentThread->Finish();
    return;
  }

  printf("Thread is last in process and this is last process. Nachos halting.\n");
  processLock->Release();

  interrupt->Halt();
}

void Kernel_Thread(int vaddr)
{
  machine->WriteRegister(PCReg, vaddr);
  machine->WriteRegister(NextPCReg, vaddr + 4);

  int stackPage = currentThread->space->NewPageTable();
  int stackAddr = (stackPage * PageSize) - 16;

  currentThread->stackPage = stackPage - (UserStackSize / PageSize);

  machine->WriteRegister(StackReg, stackAddr);

  machine->Run();
}

void Fork_Syscall(unsigned int vaddr, int len, unsigned int vFuncAddr)
{
  if (len <= 0) 
  { // Validate length is nonzero and positive
    printf("Invalid length for thread identifier.\n");
    conditionsLock->Release();
    return;
  }

  char * buf = new char[len + 1];

  if ( !buf ) 
  { // If error allocating memory for character buffer
    printf("Error allocating kernel buffer for creating new thread!\n");
    conditionsLock->Release();
    return;
  } 
  
  if ( copyin(vaddr, len, buf) == -1 ) 
  { // If failed to read memory from vaddr passed in
    printf("Bad pointer passed to create new thread.\n");
    delete[] buf;
    conditionsLock->Release();
    return;
  }

  buf[len] = '\0';

  processLock->Acquire();

  Process * p = processInfo.at(currentThread->processID);
  Thread * t = new Thread(buf);

  t->processID = p->processID;
  t->space = p->space;

  p->numExecutingThreads++;

  t->Fork((VoidFunctionPtr)Kernel_Thread, vFuncAddr);

  processLock->Release();
}

void Exec_Thread()
{
  currentThread->stackPage = currentThread->space->InitRegisters();
  currentThread->space->RestoreState();

  machine->Run();
}

void Exec_Syscall(int vaddr, int len)
{
  if (len <= 0) 
  { // Validate length is nonzero and positive
    printf("Invalid length for thread identifier.\n");
    return;
  }

  char * buf = new char[len + 1];

  if ( !buf ) 
  { // If error allocating memory for character buffer
    printf("Error allocating kernel buffer for creating new thread!\n");
    return;
  } 
  
  if ( copyin(vaddr, len, buf) == -1 ) 
  { // If failed to read memory from vaddr passed in
    printf("Bad pointer passed to create new thread.\n");
    delete[] buf;
    return;
  }

  buf[len] = '\0';

  processLock->Acquire();

  OpenFile *executable = fileSystem->Open(buf);
  AddrSpace *space;

  if (executable == NULL) 
  {
    printf("Unable to open file %s\n", buf);
    processLock->Release();
    return;
  }

  space = new AddrSpace(executable);

  Process * p = new Process();
  Thread * t = new Thread(buf);

  processInfo.push_back(p);

  p->processID = processInfo.size() - 1;
  p->space = space;
  p->numExecutingThreads = 1;
  p->numSleepingThreads = 0;

  t->processID = p->processID;
  t->space = space;

  delete executable;

  t->Fork((VoidFunctionPtr)Exec_Thread, 0);

  processLock->Release();
}

void Join_Syscall()
{
}

int handleMemoryFull(){
  //update tlbtranslation
  //selecting a page to be evicted either FIFO or Random
  //how to determine if FIFO or random??? depending on the command line call

  /*EVICT! part 4. find a place in swap 
  file to copy it to, update page 
  table for what we evict,
  */
  //RANDOM= call rand() function, make between 1 and 31. evict the page at the rand value.

  //IF RANDOM EVICTION, USE RAND. DEFAULT IS RAND
  //nachos -P FIFO
  //nachos -P RAND argv[0]==rand??

 
    //if valid, propogate dirty bit to tlb. if dirty bit, to swapfile.
    int pageEvicted= rand() % NumPhysPages;
  
  //evict page to make a hole to load in the page that i need
  //since i got an IPT miss. replace that page with the needed page
  int ppn = pageEvicted;
  return ppn;

  //work to evict page is immaterial of the page you pick
  //IF FIFO
  // writing to the swap file
}
int handleIPTMiss( int neededVPN ) {
        int ppn = memBitMap->Find();  //Find a physical page of memory
        if ( ppn == -1 ) {
            ppn = handleMemoryFull();
        }

        return ppn;
        //read values from page table as to location of needed virtual page
        //copy page from disk to memory, if needed
}
void handlePageFault(unsigned int vaddr){
   
  //TLBLock->Acquire(); WE NEED A LOCK WHEN UPDATING THE PAGE TABLE STUFF. BUT NOT SURE AROUND EXACTLY WHAT
  tlbCounter++; 
  int vpn = vaddr/PageSize;
  int ppn=-1;
  for (int i=0; i < NumPhysPages; i++) {
      if (ipt[i].valid && ipt[i].virtualPage == vpn && ipt[i].space == currentThread->space) {
        ppn = i;
        break;
      }
  }
    if (ppn == -1) {
        ppn = handleIPTMiss(vpn);
    } 

    /* INCREMENT THE TLB . search the ipt for the proper 
    physical page number, then put that page number inside the tlb*/
    machine->tlb[tlbCounter%TLBSize].virtualPage= ipt[ppn].virtualPage;
    machine->tlb[tlbCounter%TLBSize].physicalPage= ipt[ppn].physicalPage;
    machine->tlb[tlbCounter%TLBSize].valid= true;
    machine->tlb[tlbCounter%TLBSize].use= ipt[ppn].use;
    machine->tlb[tlbCounter%TLBSize].dirty= ipt[ppn].dirty;
    //machine->tlb[tlbCounter].space= ipt[ppn].space;  ??????
    //TLBLock->Release(); ????
}
void ExceptionHandler(ExceptionType which) 
{
  //IF I AM A CLIENT
    //create message
    //send message to server
      //
    //receive response
    //parse response if valid, type=
  //

  int type = machine->ReadRegister(2); // Which syscall?
  int rv = 0;   // the return value from a syscall

  if ( which == SyscallException ) 
  {
    switch (type) 
    {
      default:
      DEBUG('a', "Unknown syscall - shutting down.\n");

      case SC_Halt:
      DEBUG('a', "Shutdown, initiated by user program.\n");
      interrupt->Halt();
      break;

      case SC_Exit:
      DEBUG('a', "Exit Syscall.\n");
      Exit_Syscall(machine->ReadRegister(4));
      break;

      case SC_Exec:
      DEBUG('a', "Exec syscall.\n");
      Exec_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
      break;

      case SC_Join:
      DEBUG('a', "Join syscall.\n");
      break;

      case SC_Create:
      DEBUG('a', "Create syscall.\n");
      Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
      break;

      case SC_Open:
      DEBUG('a', "Open syscall.\n");
      rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
      break;

      case SC_Read:
      DEBUG('a', "Read syscall.\n");
      rv = Read_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6));
      break;

      case SC_Write:
      DEBUG('a', "Write syscall.\n");
      Write_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6));
      break;

      case SC_WriteInt:
      DEBUG('a', "WriteInt syscall.\n");
      WriteInt_Syscall(machine->ReadRegister(4));
      break;

      case SC_WriteError:
      DEBUG('a', "WriteInt syscall.\n");
      WriteError_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
      break;

      case SC_Close:
      DEBUG('a', "Close syscall.\n");
      Close_Syscall(machine->ReadRegister(4));
      break;

      case SC_Fork:
      DEBUG('a', "Fork syscall.\n");
      Fork_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6));
      break;

      case SC_Yield:
      DEBUG('a', "Yield syscall.\n");
      Yield_Syscall();
      break;

      case SC_CreateLock:
      DEBUG('a', "Create Lock syscall.\n");
      rv = CreateLock_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
      break;

      case SC_AcquireLock:
      DEBUG('a', "Acquire Lock syscall.\n");
      rv = AcquireLock(machine->ReadRegister(4));
      break;

      case SC_ReleaseLock:
      DEBUG('a', "Release Lock syscall.\n");
      rv = ReleaseLock(machine->ReadRegister(4));
      break;

      case SC_DestroyLock:
      DEBUG('a', "Destroy Lock syscall.\n");
      rv = DestroyLock(machine->ReadRegister(4));
      break;

      case SC_CreateCV:
      DEBUG('a', "CreateCV syscall.\n");
      rv = CreateCV(machine->ReadRegister(4), machine->ReadRegister(5));
      break;

      case SC_Wait:
      DEBUG('a', "Wait syscall.\n");
      rv = Wait(machine->ReadRegister(4), machine->ReadRegister(5));
      break;

      case SC_Signal:
      DEBUG('a', "Signal syscall.\n");
      rv= Signal(machine->ReadRegister(4), machine->ReadRegister(5));
      break;

      case SC_Broadcast:
      DEBUG('a', "Broadcast syscall.\n");
      rv= Broadcast(machine->ReadRegister(4), machine->ReadRegister(5));
      break;

      case SC_DestroyCV:
      DEBUG('a', "Destroy Condition syscall.\n");
      rv= DestroyCV(machine->ReadRegister(4));
      break;

      case SC_Random:
      DEBUG('a', "Random syscall.\n");
      rv= Random(machine->ReadRegister(4), machine->ReadRegister(5));
      break;

      case SC_WriteOne:
      DEBUG('a', "Write one.\n");
      rv= WriteOne(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6));
      break;

      case SC_WriteTwo:
      DEBUG('a', "Write two.\n");
      rv= WriteTwo(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6), machine->ReadRegister(7));
      break;
    }

    //SERVER SENDS A MESSAGE BACK TO CLIENT OF SUCCESS/FAILURE, SEND BACK RETURN VALUE.
    // Put in the return value and increment the PC
    machine->WriteRegister(2, rv);
    machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
    machine->WriteRegister(NextPCReg, machine->ReadRegister(PCReg) + 4);
    return;
  }
  else if ( which == PageFaultException ) {
    handlePageFault(machine->ReadRegister(39));
    return;
  }
  else 
  {
    cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
    interrupt->Halt();
  }
}