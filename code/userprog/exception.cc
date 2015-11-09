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
#include <sstream>

using namespace std;

//========================================================================================================================================
//
// System Calls
//  System calls are how user programs request services from the OS.
//  Below are Syscalls for:
//  (1) File System
//  (2) Thread Execution
//  (3) Synchronization Objects
//
//  See also:   syscall.h   the syscall declarations
//              start.s     the assembly assists for user programs
//
//========================================================================================================================================

//----------------------------------------------------------------------
// copyin
//  Copy len bytes from the current thread's virtual address vaddr into
//  buffer. Return the number of bytes read, or -1 if an error occurs. 
//  Errors generally mean a bad virtual address was passed in 
//  (translation failed). This method can be used to copy in strings 
//  from user memory to a buffer in syscalls below.
//
//  "vaddr" -- the starting virtual address in current thread
//  "len" -- the length of (in bytes) to copy
//  "buf" -- the temporary memory to output for calling function to use
//----------------------------------------------------------------------

int copyin(unsigned int vaddr, int len, char *buf) 
{
    bool result;
    int bytes = 0; // The number of bytes copied in
    int *paddr = new int;

    // Read len bytes into buffer
    while (bytes >= 0 && bytes < len)
    {
        result = machine->ReadMem( vaddr, 1, paddr ); // Read 1 byte at vaddr into paddr

        //while(!result) // FALL 09 CHANGES
        //{
        //    result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
        //}

        buf[bytes++] = *paddr; // Update value of buffer to byte read from vaddr (per loop)

        // If translation failed
        if (!result)
            return -1;

        vaddr++; // Next byte of program's memory
    }

    delete paddr;
    return len;
}

//----------------------------------------------------------------------
// copyout
//  Copy len bytes to the current thread's virtual address vaddr.
//  Return the number of bytes so written, or -1 if an error occurs. 
//  Errors can generally mean a bad virtual address was passed in. Used
//  by Read_Syscall to copy Console Input or File contents to userprog's
//  memory.
//
//  "vaddr" -- the starting virtual address in current thread
//  "len" -- the length of (in bytes) to copy
//  "buf" -- the temporary memory to output for calling function to use
//----------------------------------------------------------------------

int copyout(unsigned int vaddr, int len, char *buf) {
    bool result;
    int bytes = 0; // The number of bytes copied in

    while ( bytes >= 0 && bytes < len)
    {
        // Note that we check every byte's address
        result = machine->WriteMem( vaddr, 1, (int)(buf[bytes++]) ); // Write 1 byte of buffer to vaddr

        // If translation failed
        if (!result)
            return -1;

        vaddr++; // Next byte of program's memory
    }

    return bytes;
}

//----------------------------------------------------------------------
// Create_Syscall
//  Create the file with the name in the user buffer pointed to by 
//  vaddr. The file name is at most MAXFILESIZE chars long.  No way to 
//  return errors, though...
//
//  "vaddr" -- the virtual address of the file name
//  "len" -- the length of the file name
//----------------------------------------------------------------------

/*bool SendtoServer(PacketHeader pktHdr, MailHeader mailHdr, char *data){
  return true;
}
        // Send a message to a mailbox on a remote 
        // machine.  The fromBox in the MailHeader is 
        // the return box for ack's.
    
void ReceivefromServer(PacketHeader *pktHdr, MailHeader *mailHdr, char *data){

}
*/
void Create_Syscall(unsigned int vaddr, int len) {
    char *buf = new char[len + 1]; // Kernel buffer to copy file name into

    // Out of memory
    if (!buf)
        return;

    // Translation failed; else string copied into buf (!= -1)
    if (copyin(vaddr,len,buf) == -1) 
    {
        printf("%s","Bad pointer passed to Create\n");
        delete buf;
        return;
    }

    buf[len] = '\0'; // Add null terminating character to file name

    fileSystem->Create(buf,0);
    delete[] buf;
    return;
}

//----------------------------------------------------------------------
// Open_Syscall
//  Open the file with the name in the user buffer pointed to by vaddr. 
//  The file name is at most MAXFILESIZE chars long. If the file is 
//  opened successfully, it is put in the address space's file table and 
//  an id returned that can find the file later. If there are any errors, 
//  -1 is returned.
//  Side effects: Adds file to process' fileTable, if successful
//
//  "vaddr" -- the virtual address of the file name
//  "len" -- the length of the file name
//----------------------------------------------------------------------

int Open_Syscall(unsigned int vaddr, int len) {
    char *buf = new char[len+1]; // Kernel buffer to copy file name into
    OpenFile *f; // The new open file
    int id; // The openfile id

    // Out of memory
    if (!buf) 
    {
        printf("%s","Can't allocate kernel buffer in Open\n");
        return -1;
    }

    // Translation failed; else string copied into buf (!= -1)
    if (copyin(vaddr,len,buf) == -1)
    {
        printf("%s","Bad pointer passed to Open\n");
        delete[] buf;
        return -1;
    }

    buf[len]='\0'; // Add null terminating character to file name

    f = fileSystem->Open(buf);
    delete[] buf;

    // File successfully opened
    if (f)
    {
        // File Table full: delete file; else: Put in open File Table
        if ((id = currentThread->space->fileTable.Put(f)) == -1)
            delete f;
        return id; // Program may reference file again
    }
    else
        return -1;
}

//----------------------------------------------------------------------
// Write_Syscall
//  Write the buffer to the given disk file. If ConsoleOutput is the 
//  fileID, data goes to the synchronized console instead. If a Write 
//  arrives for the synchronized Console, and no such console exists, 
//  create one. For disk files, the file is looked up in the current 
//  address space's open file table and used as the target of the write.
//
//  "vaddr" -- the virtual address of the file name
//  "len" -- the length of the file name
//----------------------------------------------------------------------

void Write_Syscall(unsigned int vaddr, int len, int id) {
    char *buf = new char[len]; // Kernel buffer for output
    OpenFile *f; // Open file for output

    // Cannot write with ConsoleInput
    if (id == ConsoleInput)
        return;

    // Out of memory
    if (!buf)
    {
        printf("%s","Error allocating kernel buffer for write!\n");
        return;
    } 
    
    // Translation failed; else string copied into buf (!= -1)
    if (copyin(vaddr,len,buf) == -1)
    {
        printf("%s","Bad pointer passed to to write: data not written\n");
        delete[] buf;
        return;
    }

    // Synchronous output
    if (id == ConsoleOutput)
    {
        for (int ii=0; ii<len; ii++)
        {
            printf("%c",buf[ii]);
        }
    }
    else
    {
        // id = Open File for Process
        if ((f = (OpenFile *) currentThread->space->fileTable.Get(id)))
        {
            f->Write(buf, len);
        }
        else
        {
            printf("%s","Bad OpenFileId passed to Write\n");
            len = -1;
        }
    }

    delete[] buf;
}

//----------------------------------------------------------------------
// Read_Syscall
//  Write the buffer to the given disk file. If ConsoleOutput is the 
//  fileID, data goes to the synchronized console instead. If a Write 
//  arrives for the synchronized Console, and no such console exists, 
//  create one. We reuse len as the number of bytes read, which is an
//  unnessecary savings of space.
//----------------------------------------------------------------------

int Read_Syscall(unsigned int vaddr, int len, int id) {
    char *buf; // Kernel buffer for input
    OpenFile *f; // Open file for output

    if (id == ConsoleOutput) return -1;
    
    if (!(buf = new char[len]))
    {
        printf("%s","Error allocating kernel buffer in Read\n");
        return -1;
    }

    if (id == ConsoleInput)
    {
        // Reading from the keyboard
        scanf("%s", buf);

        if (copyout(vaddr, len, buf) == -1)
        {
            printf("%s","Bad pointer passed to Read: data not copied\n");
        }
    }
    else
    {
        if ((f = (OpenFile *) currentThread->space->fileTable.Get(id)))
        {
            len = f->Read(buf, len);

            if (len > 0)
            {
                // Read something from the file. Put into user's address space
                if (copyout(vaddr, len, buf) == -1)
                {
                    printf("%s","Bad pointer passed to Read: data not copied\n");
                }
            }
        }
        else
        {
            printf("%s","Bad OpenFileId passed to Read\n");
            len = -1;
        }
    }

    delete[] buf;
    return len;
}

//----------------------------------------------------------------------
// Close_Syscall
//  Close the file associated with id fd.  No error reporting.
//----------------------------------------------------------------------

void Close_Syscall(int fd) {
    OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

    if (f)
    {
        delete f;
    }
    else
    {
        printf("%s","Tried to close an unopen file\n");
    }
}

//----------------------------------------------------------------------
// PrintError_Syscall
//  Helper Method for printing red output to console while debugging.
//----------------------------------------------------------------------

#define RED               "\x1b[31m"
#define ANSI_COLOR_RESET  "\x1b[0m"

void PrintError_Syscall(unsigned int vaddr, int len)
{
    printf(RED);
    Write_Syscall(vaddr, len, ConsoleOutput);
    printf(ANSI_COLOR_RESET);
}

//----------------------------------------------------------------------
// PrintfOne_Syscall
//  Equivalent to calling printf with one extra parameter. Calls printf
//  with the string copyin'd at thread's vaddr and the integer params.
//
//  "vaddr" -- the virtual address of the string with format specifiers
//  "len" -- the length of the string
//  "num1" -- the first integer to be output inside the string.
//----------------------------------------------------------------------

void PrintfOne_Syscall(unsigned int vaddr, int len, int num1)
{
    // Validate length is nonzero and positive
    if (len < 0) 
    {
        printf("%s","Length for lock's identifier name must be nonzero and positive\n");
        return;
    }

    char *buf = new char[len + 1]; // Kernel buffer to copy file name into

    // Out of memory
    if (!buf)
        return;

    // Translation failed; else string copied into buf (!= -1)
    if (copyin(vaddr,len,buf) == -1) 
    {
        printf("%s","Bad pointer passed to Create\n");
        delete buf;
        return;
    }

    buf[len] = '\0'; // Add null terminating character to file name
    
    printf(buf, num1);
    return;
}

//----------------------------------------------------------------------
// PrintfTwo_Syscall
//  Equivalent to calling printf with two extra parameters. Calls printf
//  with the string copyin'd at thread's vaddr and the integer params.
//
//  "vaddr" -- the virtual address of the string with format specifiers
//  "len" -- the length of the string
//  "num1" -- the first integer to be output inside the string.
//  "num2" -- the second integer to be output inside the string.
//----------------------------------------------------------------------

void PrintfTwo_Syscall(unsigned int vaddr, int len, int num1, int num2)
{
    // Validate length is nonzero and positive
    if (len < 0) 
    {
        printf("%s","Length for lock's identifier name must be nonzero and positive\n");
        return;
    }

    char *buf = new char[len + 1]; // Kernel buffer to copy file name into

    // Out of memory
    if (!buf)
        return;

    // Translation failed; else string copied into buf (!= -1)
    if (copyin(vaddr,len,buf) == -1) 
    {
        printf("%s","Bad pointer passed to Create\n");
        delete buf;
        return;
    }

    buf[len] = '\0'; // Add null terminating character to file name
    
    printf(buf, num1, num2);
    return;
}

//----------------------------------------------------------------------
// Random_Syscall
//  Syscall wrapper for a randomly seeded Random method.
//----------------------------------------------------------------------

int Random_Syscall(int lower, int upper)
{
    srand(time(0));
    int randomNumber = rand() % upper + lower;
    return randomNumber;
}

//----------------------------------------------------------------------
// validatelockindex
//  Validate lock index corresponds to a (1) valid location, (2) defined
//  lock that (3) belongs to currentThread's process.
//
//  Returns -1 if invalid index, 0 if valid
//
//  "index" -- the location of the KernelLock object inside locks 
//----------------------------------------------------------------------

int validatelockindex(int index)
{
    // (1) Index corresponds to valid location
    if (index < 0)
    {
        printf("%s","Invalid lock table index, negative.\n");
        return -1;
    }

    int size = locks.size();

    // (1) Index corresponds to valid location
    if (index > size - 1)
    {
        printf("%s","Invalid lock table index, bigger than size.\n");
        return -1;
    }

    KernelLock * currentKernelLock = locks.at(index);

    // (2) Defined lock
    if (!currentKernelLock)
    {
        printf("Lock %d is NULL.\n", index);
        return -1;
    }

    // (3) Belongs to currentThread's process
    if (currentKernelLock->space != currentThread->space)
    {
        printf("Lock %d does not belong to the current process.\n", index);
        return -1;
    }

    return 0;
}

//----------------------------------------------------------------------
// CreateLock_Syscall
//  After grabbing the name for the lock, adds a new KernelLock holding
//  which process owns it, whether it's been flagged for deletion (by
//  program Exiting or calling DestroyLock), and the lock object.
//
//  Returns the "indexlock" so user program may Acquire and Release lock
//
//  "vaddr" -- the virtual address of the lock name
//  "len" -- length of lock name
//----------------------------------------------------------------------

int CreateLock_Syscall(unsigned int vaddr, int len) 
{
  #ifdef NETWORK

    printf("TEST INSIDE CREATE LOCK\n", sizeof("TEST INSIDE CREATE LOCK\n"), 1);
  if (len <= 0)
    {
        printf("%s","Length for lock's identifier name must be nonzero and positive\n");
        locksLock->Release();
        return -1;
    }

    char * buf = new char[len + 1];
    char * message= new char[40]; 
    //max size of a message can be 40 according to class notes



    // Translation failed; else string copied into buf (!= -1)
    if (copyin(vaddr, len, buf) == -1)
    {
        printf("%s","Bad pointer passed to create new lock\n");
        locksLock->Release();
        delete[] buf;
        return -1;
    }
    buf[len] = '\0';

    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;

    //2 bits for client machine 
    //#/server each, 2 bits for postoficec #
    // thread #, for client and server, and instruction
    //for 2 bits
    char buffer[MaxMailSize];

    outPktHdr.to = 0;   
    outMailHdr.to = 0;
    outMailHdr.from = 1;

  std::stringstream temp;
  temp << "CL" << buf; //currently passses in CL, and the name. 
  temp >> message;
  outMailHdr.length = strlen(message) +1;

  //Send the request
  printf("in create syscall\n");
  bool success = postOffice->Send(outPktHdr, outMailHdr, message);
  if ( !success ) {
    printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
    interrupt->Halt();
  }

  //Wait for the server's response
  postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
  printf("Lock %s created by server\n", buffer);
  fflush(stdout);

  //Return the server's response to calling program
  int returnValue = atoi(buffer);
  return returnValue;


#else //PROJECT 2 CODE

    locksLock->Acquire(); // Interupts enabled, need to synchronize

    // Validate length is nonzero and positive
    if (len <= 0)
    {
        printf("%s","Length for lock's identifier name must be nonzero and positive\n");
        locksLock->Release();
        return -1;
    }

    char * buf = new char[len + 1];

    // Out of memory
    if (!buf)
    {
        printf("%s","Error allocating kernel buffer for creating new lock!\n");
        locksLock->Release();
        return -1;
    }

    // Translation failed; else string copied into buf (!= -1)
    if (copyin(vaddr, len, buf) == -1)
    {
        printf("%s","Bad pointer passed to create new lock\n");
        locksLock->Release();
        delete[] buf;
        return -1;
    }

    buf[len] = '\0'; // Add null terminating character to lock name

   // lock with metadata
    KernelLock * newKernelLock = new KernelLock(); // cv with metadata
    
    newKernelLock->toDelete = false; // flagged for deletion via Exit or DestroyLock
    newKernelLock->space = currentThread->space; // lock process
    newKernelLock->lock = new Lock(buf); // OS lock

    locks.push_back(newKernelLock); // Add to lock collection; indexed by lockID
    int indexLock = locks.size() - 1;

    locksLock->Release();

    return 0; // Processes can Acquire/Release
    #endif
}

//----------------------------------------------------------------------
// AcquireLock_Syscall
//  Acquires lock if index is (1) valid location, (2) defined, 
//  (3) belongs to currentThread's process, and (4) lock is not flagged
//  for deletion. Must be careful about keeping process table consistent
//  because thread could be put to sleep if trying to Acquire a lock
//  already owned.
//
//  Returns "indexLock"
//
//  "indexLock" -- the id for lock inside locks trying to be Acquired
//----------------------------------------------------------------------

int AcquireLock_Syscall(int indexlock)
{
  #ifdef NETWORK
  #else
  #endif

    // Lock index: (1) valid location, (2) defined, (3) belongs to currentThread's process
    if (validatelockindex(indexlock) == -1)
    {
        return -1;
    }

    // Don't allow threads to Acquire Locks that have been flagged for deletion
    if (locks.at(indexlock)->toDelete)
    {
        printf("Cannot acquire lock because it's been marked for deletion.");
        return -1;
    }

    // May go to sleep inside Acquire
    processLock->Acquire(); // Synchronize process info (interrupts enabled)
    processInfo.at(currentThread->processID)->numExecutingThreads--;
    processInfo.at(currentThread->processID)->numSleepingThreads++;
    processLock->Release();

    locks.at(indexlock)->lock->Acquire();

    // Made it out alive, correct numExecutingThreads
    processLock->Acquire();
    processInfo.at(currentThread->processID)->numExecutingThreads++;
    processInfo.at(currentThread->processID)->numSleepingThreads--;
    processLock->Release();

    return indexlock;
}

//----------------------------------------------------------------------
// deletelock
//  If lock has been no threads waiting to Acquire and (1) user called 
//  Destroy or (2) user called Release and already marked for deletion, 
//  delete the lock.
//
//  "indexLock" -- the (valid) index of lock inside locks
//----------------------------------------------------------------------

void deletelock(int indexlock)
{
    KernelLock * currentKernelLock = locks.at(indexlock);

    delete currentKernelLock->lock;
    delete currentKernelLock;
    locks.at(indexlock) = NULL;

    printf("Lock %d was successfully deleted.\n", indexlock);
}

//----------------------------------------------------------------------
// ReleaseLock_Syscall
//  Releases lock if index is (1) valid location, (2) defined, 
//  (3) belongs to currentThread's process. If lock is flagged for
//  deletion and there are no waiting threads, delete the lock.
//
//  Returns -1 if unable to Release, "indexLock" if successful
//
//  "indexlock" -- index of lock in locks collection
//----------------------------------------------------------------------

int ReleaseLock_Syscall(int indexlock)
{
    // Lock index: (1) valid location, (2) defined, (3) belongs to currentThread's process
    if (validatelockindex(indexlock) == -1)
    {
        return -1;
    }

    KernelLock * currentKernelLock = locks.at(indexlock);
    currentKernelLock->lock->Release();

    // Lock is set to delete and it is free, with no waiting threads: Delete Lock.
    if (currentKernelLock->toDelete && currentKernelLock->lock->isAbleToDelete())
    {
        locksLock->Acquire();
        deletelock(indexlock);
        locksLock->Release();
    }

    return indexlock;
}

//----------------------------------------------------------------------
// DestroyLock_Syscall
//  Deletes the lock or marks it for future deletion (sleeping threads) 
//  if index is (1) valid location, (2) defined, (3) belongs to 
//  currentThread's process.
//
//  Returns 0 if successfully deleted lock, -1 if invalid index or only
//  set to delete in the future because threads are waiting to Acquire.
//
//  "indexlock" -- the index of lock in locks collection
//----------------------------------------------------------------------

int DestroyLock_Syscall(int indexlock)
{
    // Lock index: (1) valid location, (2) defined, (3) belongs to currentThread's process
    if (validatelockindex(indexlock) == -1)
    {
        return -1;
    }

    locksLock->Acquire();

    KernelLock * currentKernelLock = locks.at(indexlock);

    // Lock is FREE with no waiting Threads
    if (currentKernelLock->lock->isAbleToDelete())
    {
        deletelock(indexlock);
        locksLock->Release();
        return 0;
    }

    // Threads are waiting to Acquire; Let them use lock then Delete on last Release.
    printf("Lock %d toDelete is set to true. Cannot be deleted because sleepqueue is not empty.\n", indexlock);
    currentKernelLock->toDelete = true;
    locksLock->Release();
    return -1;
}

//----------------------------------------------------------------------
// validatecvindeces
//  Validate lock and cv index corresponds to a (1) valid location, 
//  (2) defined lock and cv that (3) belongs to currentThread's process.
//
//  Returns -1 if invalid index, 0 if valid
//
//  "indexlock" -- the location of the KernelLock object inside locks 
//  "indexcv" -- the location of the KernerlCV object inside conditions 
//----------------------------------------------------------------------

int validatecvindeces(int indexcv, int indexlock)
{
    // (1) index to valid location
    if (indexcv < 0 || indexlock < 0)
    {
        printf("%s","Invalid index.\n");
        return -1;
    }

    int csize = conditions.size();
    int lsize = locks.size();

    // (1) index to valid location
    if (indexcv > csize - 1 || indexlock > lsize - 1)
    {
        printf("%s","Index out of bounds.\n");
        return -1;
    }

    KernelCV * currentKernelCV = conditions.at(indexcv);
    KernelLock * currentKernelLock = locks.at(indexlock);

    // (2) index to defined lock and cv
    if (!currentKernelCV  || !currentKernelLock)
    {
        printf("Condition %d is set to NULL or Lock %d is set to NULL.\n", indexcv, indexlock);
        return -1;
    }

    // (3) lock belongs to process
    if (currentKernelLock->space != currentThread->space)
    {
        printf("Lock %d does not belong to the current process.\n", indexlock);
        return -1;
    }

    // (3) cv belongs to process
    if (currentKernelCV->space != currentThread->space)
    {
        printf("Condition %d does not belong to the current process.\n", indexcv);
        return -1;
    }

    return 0;
}

//----------------------------------------------------------------------
// CreateCV_Syscall
//  Creates a condition with the specified name. KernelCV inside of 
//  conditions stores the cv object, process, and whether it's been 
//  marked for later deletion. 
//
//  Returns "indexcv" for later Wait, Signal, and Broadcast.
//
//  "vaddr" -- the virtual address of the cv name
//  "len" -- length of cv name
//----------------------------------------------------------------------

int CreateCV_Syscall(unsigned int vaddr, int len)
{
    conditionsLock->Acquire(); // Synchronize CV creation; Interrupts enabled

    // Validate length is nonzero and positive
    if (len <= 0)
    {
        printf("Invalid length for CV identifier\n");
        conditionsLock->Release();
        return -1;
    }

    char * buf = new char[len + 1];

    // Out of memory
    if (!buf)
    {
        printf("Error allocating kernel buffer for creating new CV!\n");
        conditionsLock->Release();
        return -1;
    }

    // Translation failed; else string copied into buf (!= -1)
    if (copyin(vaddr, len, buf) == -1)
    {
        printf("Bad pointer passed to create new CV\n");
        delete[] buf;
        conditionsLock->Release();
        return -1;
    }

    buf[len] = '\0'; // Add null terminating character to cv name

    KernelCV * newKernelCV = new KernelCV(); // cv with metadata
    newKernelCV->toDelete = false; // flagged for later deletion
    newKernelCV->space = currentThread->space; // condition corresponds to single process
    newKernelCV->condition = new Condition(buf); // condition object

    conditions.push_back(newKernelCV); // Add to collection for later usage
    int conditionIndex = conditions.size() - 1;

    conditionsLock->Release();

    return conditionIndex; // User can call Wait, Signal, Broadcast
}

//----------------------------------------------------------------------
// Wait_Syscall
//  Waits on condition if indeces correspond to (1) valid locations, 
//  (2) defined objects, (3) belong to currentThread's process, and 
//  (4) are not flagged for deletion. Must be careful about keeping 
//  process table consistent when going to sleep during Wait.
//
//  Returns "indexcv"
//
//  "indexcv" -- index of condition inside of conditions collection
//  "indexlock" -- index of lock inside of locks collection
//----------------------------------------------------------------------

int Wait_Syscall(int indexcv, int indexlock)
{
    // Lock/CV indeces: (1) valid location, (2) defined, (3) belongs to currentThread's process
    if (validatecvindeces(indexcv, indexlock) == -1)
    {
        return -1;
    }

    // Don't allow more threads to Wait on a condition marked for deletion
    if (conditions.at(indexcv)->toDelete)
    {
        printf("Cannot wait on condition because it's been marked for deletion.");
        return -1;
    }

    // Going to sleep inside Wait, keep process table consistent for Exit
    processLock->Acquire();
    processInfo.at(currentThread->processID)->numExecutingThreads--;
    processInfo.at(currentThread->processID)->numSleepingThreads++;
    processLock->Release();

    conditions.at(indexcv)->condition->Wait(locks.at(indexlock)->lock);

    processLock->Acquire();
    processInfo.at(currentThread->processID)->numExecutingThreads++;
    processInfo.at(currentThread->processID)->numSleepingThreads--;
    processLock->Release();

    return indexcv;
}

//----------------------------------------------------------------------
// deletecondition
//  If condition has been no threads Waiting to be Signaled or Broadcast
//  and (1) user called Destroy or (2) last thread called Signal and 
//  already marked for deletion or (3) any thread called Broadcast and 
//  already marked for deletion.
//
//  "indexcv" -- the (valid) index of condition inside conditions
//----------------------------------------------------------------------

void deletecondition(int indexcv)
{
    KernelCV * curKernelCV = conditions.at(indexcv);

    delete curKernelCV->condition;
    delete curKernelCV;
    conditions.at(indexcv) = NULL;

    printf("Condition %d was successfully deleted.\n", indexcv);
}

//----------------------------------------------------------------------
// Signal_Syscall
//  Signals a single Waiting thread for a condition if lock and cv 
//  indeces correspond to (1) valid locations, (2) defined objects, 
//  (3) belong to currentThread's process. If thread is the last thread
//  Waiting on condition and condition was marked for deletion, delete
//  condition.
//  
//  Returns "indexcv" if calling Signal successful, -1 if input errors
//
//  "indexcv" -- index of condition inside of conditions collection
//  "indexlock" -- index of lock inside of locks collection
//----------------------------------------------------------------------

int Signal_Syscall(int indexcv, int indexlock)
{
    // Lock/CV indeces: (1) valid location, (2) defined, (3) belongs to currentThread's process
    if (validatecvindeces(indexcv, indexlock) == -1)
    {
        return -1;
    }

    KernelCV * curKernelCV = conditions.at(indexcv);
    curKernelCV->condition->Signal(locks.at(indexlock)->lock);

    // Marked for deletion and no waiting threads, delete
    if (curKernelCV->toDelete && curKernelCV->condition->waitqueue->IsEmpty())
    {
        conditionsLock->Acquire();
        deletecondition(indexcv);
        conditionsLock->Release();
    }

    return indexcv;
}

//----------------------------------------------------------------------
// Broadcast_Syscall
//  Wakes up all Waiting thread for a condition if lock and cv indeces 
//  correspond to (1) valid locations, (2) defined objects, (3) belong 
//  to currentThread's process. If condition marked for deletion, now
//  safe to delete.
//
//  Returns "indexcv" if calling Broadcast successful, -1 if input error
//
//  "indexcv" -- index of condition inside of conditions collection
//  "indexlock" -- index of lock inside of locks collection
//----------------------------------------------------------------------

int Broadcast_Syscall(int indexcv, int indexlock)
{
    // Lock/CV indeces: (1) valid location, (2) defined, (3) belongs to currentThread's process
    if (validatecvindeces(indexcv, indexlock) == -1)
    {
        return -1;
    }

    KernelCV * curKernelCV = conditions.at(indexcv);
    curKernelCV->condition->Broadcast(locks.at(indexlock)->lock);

    // Just woke up any waiting threads. If marked for deletion, now delete.
    if (curKernelCV->toDelete)
    {
        conditionsLock->Acquire();
        deletecondition(indexcv);
        conditionsLock->Release();
    }

    return indexcv;
}

//----------------------------------------------------------------------
// DestroyCV_Syscall
//  Sets for deletion or deletes if cv index corresponds to a (1) valid 
//  location (2) defined object (3) belongs to currentThread's process.
//  Mark for later deletion by (1) last thread to Signal or (2) any 
//  thread to Broadcast.
//
//  Return 0 if successfully/synchronously deleted, -1 if error or later
//  deletion.
//
//  "indexcv" -- index of condition inside of conditions collection
//----------------------------------------------------------------------

int DestroyCV_Syscall(int indexcv)
{

    // Lock/CV indeces: (1) valid location, (2) defined, (3) belongs to currentThread's process
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

    KernelCV * currentKernelCV = conditions.at(indexcv);

    if (!currentKernelCV || !currentKernelCV->condition)
    {
        printf("Condition struct or condition var of index %d is null and can't be destroyed.\n", indexcv);
        conditionsLock->Release();
        return -1;
    }

    if (currentKernelCV->space != currentThread->space) 
    {
        printf("Condition of index %d does not belong to the current process\n", indexcv);
        conditionsLock->Release();
        return -1;
    }

    // No waiting threads when Destroy called, delete
    if (currentKernelCV->condition->waitqueue->IsEmpty())
    {
        deletecondition(indexcv);
        conditionsLock->Release();
        return 0;
    }

    // Mark for later deletion by (1) last thread to Signal or (2) any thread to Broadcast
    printf("Condition %d toDelete set to true. Cannot be deleted since waitqueue is not empty.\n", indexcv);
    currentKernelCV->toDelete = true;
    conditionsLock->Release();
    return -1;
}

//----------------------------------------------------------------------
// Yield_Syscall
//  Syscall allows thread some control over execution. This yields 
//  control to another thread, which the scheduler with switch if any
//  ready.
//----------------------------------------------------------------------
void CreateMV(){

}

void DestroyMV(){

} 
void GetMV(){

}

void SetMV(){

}

void Yield_Syscall() 
{
    currentThread->Yield();
}

//----------------------------------------------------------------------
// Exit_Syscall
//  When a thread Exits, there are three cases: (1) Other threads still
//  running inside its process, (2) it's the last thread running in
//  process, or (3) it's the last thread running in Nachos. If (1) just
//  clean up currentThread. If (2) clean up entire process. If (3) halt
//  Nachos execution.
//
//  "status" -- sucessful (0) or failed (1)
//----------------------------------------------------------------------

void Exit_Syscall(int status)
{
    // Status 0 = Success
    if (status != 0)
    {
        printf("Exit: Arg must be 0, %d instead.\n", status);
    }

    currentThread->Yield(); // Stop executing thread
    processLock->Acquire();

    // Other threads running in process: 
    //  (1) Reclaim its stack
    //  (2) Keep process table consistent
    //  (3) Finish this thread
    if (processInfo.at(currentThread->processID)->numExecutingThreads > 1) 
    {
        printf("Thread is not last in process. Finishing thread.\n");

        processInfo.at(currentThread->processID)->numExecutingThreads--;
        currentThread->space->ReclaimStack(currentThread->stackPage);
        processLock->Release();
        currentThread->Finish();

        return;
    }
    // Last thread in process, not last process: 
    //  (1) Delete locks, cvs associated with thread
    //  (2) Reclaim process' memory
    //  (3) Delete from process table
    //  (4) Finish this thread
    else if (processInfo.size() > 1)
    {
        printf("Thread is last in process. Cleaning up process.\n");
        
        int lsize = locks.size();
        for (int i = 0; i < lsize; i++)
        {
            if (locks.at(i))
            {
                if (locks.at(i)->space == currentThread->space)
                {
                    locksLock->Acquire();
                    deletelock(i);
                    locksLock->Release();
                }
            }
        }

        int csize = conditions.size();
        for (int i = 0; i< csize; i++)
        {
            if (conditions.at(i))
            {
                if (conditions.at(i)->space == currentThread->space)
                {
                    conditionsLock->Acquire();
                    deletecondition(i);
                    conditionsLock->Release();
                }
            }
        }

        currentThread->space->ReclaimPageTable();

        Process * p = processInfo.at(currentThread->processID);
        delete p->space;
        delete p;
        processInfo.at(currentThread->processID) = NULL;

        processLock->Release();
        currentThread->Finish();
        return;
    }

    // Nachos running no other threads: Halt
    printf("Thread is last in process and this is last process. Nachos halting.\n");
    processLock->Release();

    interrupt->Halt();
}

//----------------------------------------------------------------------
// runforkedthread
//  Execute new thread with PC at function passed in to Fork. Needs to 
//  add 8 pages to the bottom to store the new thread's stack and set 
//  the stack address to the bottom of the PageTable so we do not
//  overwrite code of another process (stack grows up).
//
//  "vaddr" -- the virtual address of starting function for new thread
//----------------------------------------------------------------------

void runforkedthread(int vaddr)
{
    // Start thread at passed-in function
    machine->WriteRegister(PCReg, vaddr);
    machine->WriteRegister(NextPCReg, vaddr + 4);

    // New PageTable = Old PageTable + 8 pages for new stack
    // Stack starts at bottom of PageTable; Must be 16 bytes above bottom 
    //  of page to avoid overwriting code space of next process 
    int stackPage = currentThread->space->NewUserStack();
    int stackAddr = (stackPage * PageSize) - 16;

    // Thread keeps track of stack's virtual address so it can translate
    //  address and loads in stack on context switch.
    currentThread->stackPage = stackPage - (UserStackSize / PageSize);

    machine->WriteRegister(StackReg, stackAddr);

    machine->Run();
}

//----------------------------------------------------------------------
// Fork_Syscall
//  Creates a thread with the specified name. Keep the process table
//  consistent. Spawn new thread by calling thread Fork with the 
//  runforkedthread method which updates the process' PageTable and 
//  starts executing the thread.
//
//  "vaddr" -- the virtual address of the thread name
//  "len" -- length of thread name
//  "vFuncAddr" -- function that thread starts out executing
//----------------------------------------------------------------------

void Fork_Syscall(unsigned int vaddr, int len, unsigned int vFuncAddr)
{
    // Validate length is nonzero and positive
    if (len <= 0)
    {
        printf("%s","Length for thread's identifier name must be nonzero and positive\n");
        processLock->Release();
        return;
    }

    char * buf = new char[len + 1];

    // Out of memory
    if (!buf)
    {
        printf("%s","Error allocating kernel buffer for creating new thread!\n");
        processLock->Release();
        return;
    }

    // Translation failed; else string copied into buf (!= -1)
    if (copyin(vaddr, len, buf) == -1)
    {
        printf("%s","Bad pointer passed to create new thread\n");
        processLock->Release();
        delete[] buf;
        return;
    }

    buf[len] = '\0'; // Add null terminating character to thread name

    Process * p = processInfo.at(currentThread->processID);
    Thread * t = new Thread(buf);

    t->processID = p->processID;
    t->space = p->space;

    p->numExecutingThreads++;

    // Spawn new thread calling function, allocate thread stack, and let scheduler know thread is ready
    t->Fork((VoidFunctionPtr)runforkedthread, vFuncAddr);

    processLock->Release();
}

//----------------------------------------------------------------------
// runnewprocess
//  Sets up machine to run new process (InitRegisters) - PC at 0th 
//  vaddr, nextPC at next instruction, and stack at end of address space
//  - and assigns the machine's address space to currentThread's AS 
//  (RestoreState). Then lets machine know it can start execution loop. 
//----------------------------------------------------------------------

void runnewprocess()
{
    currentThread->stackPage = currentThread->space->InitRegisters();
    currentThread->space->RestoreState();

    machine->Run();
}

//----------------------------------------------------------------------
// Exec_Syscall
//  Creates a new process from executable passed in by:
//  (1) Creating a new Address Space with the executable loaded in
//  (2) Keeping track of the process in a process table (For Exit())
//  (3) Storing the AS and processID on the thread and...
//  (4) Running the process
//  
//  "vaddr" -- the vaddr of the executable name (reused for thread name)
//  "len" -- the length of the executable's name
//----------------------------------------------------------------------

void Exec_Syscall(int vaddr, int len)
{
    processLock->Acquire();

    // Validate length is nonzero and positive
    if (len <= 0)
    {
        printf("%s","Length for thread's identifier name must be nonzero and positive\n");
        processLock->Release();
        return;
    }

    char * buf = new char[len + 1];

    // Out of memory
    if (!buf)
    {
        printf("%s","Error allocating kernel buffer for creating new thread!\n");
        processLock->Release();
        return;
    }

    // Translation failed; else string copied into buf (!= -1)
    if (copyin(vaddr, len, buf) == -1)
    {
        printf("%s","Bad pointer passed to create new thread\n");
        processLock->Release();
        delete[] buf;
        return;
    }

    buf[len] = '\0'; // Add null terminating character to thread name

    // Create new process from Executable by loading into thread's AS
    OpenFile *executable = fileSystem->Open(buf);
    AddrSpace *space;

    if (executable == NULL)
    {
        printf("Unable to open file %s\n", buf);
        processLock->Release();
        return;
    }

    // Load executable into address space
    space = new AddrSpace(executable);

    // Add new process to process table; Used for determining how to handle Exits
    Process * p = new Process();
    p->processID = processInfo.size() - 1;
    p->space = space;
    p->numExecutingThreads = 1;
    p->numSleepingThreads = 0;
    processInfo.push_back(p);

    // Thread needs to be able to restore itself on context switches (AS) and handling Exits (processID)
    Thread * t = new Thread(buf);
    t->processID = p->processID;
    t->space = space;

    // Close executable; Completely loaded into AS
    //delete executable;

    // Let machine know how to run thread when scheduler switches process in
    t->Fork((VoidFunctionPtr)runnewprocess, 0);

    processLock->Release();
}

//----------------------------------------------------------------------
// Join_Syscall
//----------------------------------------------------------------------

void Join_Syscall() {}


//========================================================================================================================================
//
// Memory Management Unit (MMU)
//  Responsible for all address translation. It does this by: (Real OSes
//  only do steps 0-2 since Memory is usually sufficiently big).
//
//  (0) Receive and Validate virtual address from CPU.
//
//  (1) (translate.cc) Look inside of the Translation Lookaside Buffer 
//      (a) If vaddr found and valid, use physical address.
//
//  (2) (PageFault_Handler) If not in TLB, look in Inverted Page Table 
//      (IPT) to see if vaddr in Main Memory.
//      (a) If vaddr found and belongs to process, replace a TLB entry.
//
//  (3) (IPTMiss_Handler) If not in Main Memory, load into memory.
//      (a) Get vaddr from translation Page Tables
//      (b) Find an unused page of memory
//      (c) Load vpn to physical memory and Update IPT
//      (d) Finish PageFault_Handler.
//
//  (4) (MemoryFull_Handler) If Main Memory is full, replace a Memory Page.
//      (a) If dirty evicted page, Write to swapfile and Update Page Table.
//      (b) Finish IPTMiss_Handler.
//
//========================================================================================================================================

//----------------------------------------------------------------------
// GetPageToEvict
//  Based on the eviction flag passed into Nachos, either select a 
//  random Page of Memory to evict or the next Page in FIFO order.
//
//  Returns physical page number of Page to evict.
//----------------------------------------------------------------------

unsigned int GetPageToEvict()
{
    int ppn;

    // switch (memoryEviction)
    // {
    //     case EVICTRAND:
    //         // TODO: Do we need to check use bit before evicting?
    //         srand(time(0));
    //         ppn = rand() % NumPhysPages;
    //         break;
    //     case EVICTFIFO:
    //         ppn = (unsigned int)memFIFO->Remove();
    //         break;
    // }

    // return ppn;
    return 0;
}

//----------------------------------------------------------------------
// MemoryFull_Handler
//  When Memory is full, need to evict a Page.
//  (1) Get Page to Evict
//      (a) Random Eviction     nachos -evictRAND
//      (b) FIFO Eviction       nachos -evictFIFO
//  (2) (If EvictedPage is Dirty) Place in Swapfile and Update PageTable
//  (3) Finish IPTMiss_Handler and PageFault_Handler to update IPT/TLB.
//----------------------------------------------------------------------

unsigned int MemoryFull_Handler()
{
    unsigned int i, ppn;

    // (1) Get Page to evict.
    //  Call eviction strategy based on flag passed into Nachos.
    ppn = GetPageToEvict();

#ifdef USE_TLB

    // (2) Dirty Page: Place evicted page in swapfile, update PageTable
    for (i = 0; i < TLBSize; i++)
    {
        // Memory has been written to during its time in main memory,
        //  Now that we're discarding the memory we must write it back
        //  to disk.
        if (machine->tlb[i].physicalPage == ppn && machine->tlb[i].valid && machine->tlb[i].dirty)
        {
            // TODO: Write to swapfile
            //  int physAddr = WriteToSwapFile(ppn);
            //  int swapPage = physAddr / PageSize;
            //  int offset = physAddr % PageSize;
            //  int vpn = tlb[ppn].virtualPage;
            
            //  currentThread->space->pageTable[vpn].dirty = true;
            //  currentThread->space->pageTable[vpn].swapped = true;
            //  currentThread->space->pageTable[vpn].physicalPage = swapPage;
            //  currentThread->space->pageTable[vpn].offset = offset;
        }
    }

#else

    // (2) Dirty Page: Place evicted page in swapfile, update PageTable
    for (i = 0; i < NumPhysPages; i++)
    {
        // Memory has been written to during its time in main memory,
        //  Now that we're discarding the memory we must write it back
        //  to disk.
        if (machine->tlb[i].physicalPage == ppn && machine->tlb[i].valid && machine->tlb[i].dirty)
        {
            // TODO: Write to swapfile
            //  int physAddr = WriteToSwapFile(ppn);
            //  int swapPage = physAddr / PageSize;
            //  int offset = physAddr % PageSize;
            //  int vpn = tlb[ppn].virtualPage;
            
            //  currentThread->space->pageTable[vpn].dirty = true;
            //  currentThread->space->pageTable[vpn].swapped = true;
            //  currentThread->space->pageTable[vpn].physicalPage = swapPage;
            //  currentThread->space->pageTable[vpn].offset = offset;
        }
    }

#endif

    return ppn;
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

void LoadIntoMemory(unsigned int vpn, unsigned int ppn)
{
    DEBUG('p', "Loading into Memory\n");

    // TODO: Load from Swap File
    if (currentThread->space->pageTable[vpn].swapped)
    {
        // (1) Get System Swap File
        // OpenFile * executable = (OpenFile *)currentThread->space->fileTable.Get(0);

        // (2) Load from Swap File into Main Memory
        // executable->ReadAt(
        //     &(machine->mainMemory[ppn * PageSize]), // Store into mainMemory at physical page
        //     PageSize, // Read 128 bytes
        //     currentThread->space->pageTable[vpn].offset); // From this position in executable

        currentThread->space->LoadIntoMemory(vpn, ppn);

        // Clear Swap File entry

        // (3) Update the Page Table
        // TODO:    Is this assumption correct: When we move to main memory, 
        //          Page is no longer swapped?
        currentThread->space->pageTable[vpn].physicalPage = ppn;
        currentThread->space->pageTable[vpn].offset = ppn * PageSize;
        currentThread->space->pageTable[vpn].valid = true;
        currentThread->space->pageTable[vpn].swapped = false;
    }
    // Offset should be -1 only when neither in Executable nor Swap (offset = vaddr in file)
    //  This is the case for unused Stack Pages
    else if (currentThread->space->pageTable[vpn].offset == -1)
    {
        DEBUG('p', "Loading unused Stack Page to Memory\n");

        // (2) Initialize data in memory
        machine->mainMemory[ppn * PageSize] = 0;

        // (3) Update the Page Table
        currentThread->space->pageTable[vpn].physicalPage = ppn;
        currentThread->space->pageTable[vpn].offset = ppn * PageSize;
        currentThread->space->pageTable[vpn].valid = true;
        currentThread->space->pageTable[vpn].swapped = false;
    }
    else
    {        
        DEBUG('p', "LoadIntoMemory: CurrentThread = %s\n", currentThread->getName());

        OpenFile *executable = currentThread->space->executable;

        executable->ReadAt(
             &(machine->mainMemory[ppn * PageSize]), // Store into mainMemory at physical page
             PageSize, // Read 128 bytes
             currentThread->space->pageTable[vpn].offset); // From this position in executable

        /*char *temp= currentThread->space->executable;
        OpenFile *executable= fileSystem->Open(temp);*/

        // (1) Get thread's executable
        
        DEBUG('p', "Loading from Executable, Executable Length = %d\n", executable->Length());

        // (2) Load from Executable into Main Memory
        // executable->ReadAt(
        //     &(machine->mainMemory[ppn * PageSize]), // Store into mainMemory at physical page
        //     PageSize, // Read 128 bytes
        //     currentThread->space->pageTable[vpn].offset); // From this position in executable

        // (3) Update the Page Table
        currentThread->space->pageTable[vpn].physicalPage = ppn;
        currentThread->space->pageTable[vpn].offset = ppn * PageSize;
        currentThread->space->pageTable[vpn].valid = true;
        currentThread->space->pageTable[vpn].swapped = false;

        DEBUG('p', "LoadFromExecutable: Load vaddr %d into Main Memory at %d Page:\n \tdata\t%d\n \tvpn\t%d\n \tppn\t%d\n",
                    (vpn * PageSize), ppn, machine->mainMemory[ppn * PageSize], vpn, ppn);
    }
}

//----------------------------------------------------------------------
// IPTMiss_Handler
//  When the translation is not inside of Main Memory, we need to put it
//  there and update the TLB. This handles updating Main Memory by:
//  (1) Finding a free page of Memory
//  (2) (If Memory is Full) Evicting a page to make room in Memory
//  (3) Looking up in where the virtual page is located in PageTable
//  (4) Moving entry from Swapfile or Executable to Main Memory
//  (5) Updating PageTable Entry's location
//
//  "vpn" -- the virtual page to be put in Main Memory
//----------------------------------------------------------------------

int IPTMiss_Handler(unsigned int vpn)
{
    // (1) Find free page of mainMem
    int ppn = memBitMap->Find();
    memFIFO->Append((void *)ppn); // Maintain order of Adding to Memory for FIFO Eviction
    
    DEBUG('p', "FIFO Append works.\n");

    // (2) Memory full; Evict (Random or FIFO) a page
    if (ppn == -1) {
        DEBUG('p', "Memory full.\n");
        ppn = MemoryFull_Handler();
    }

    // (3) Look up where Page located
    // (4) Move entry from Swapfile or Executable to Main Memory
    // (5) Update PageTable
    LoadIntoMemory(vpn, ppn);

    return ppn;
}

//----------------------------------------------------------------------
// PageFault_Handler
//  Caused when trying to find the physical page number for a vaddr not 
//  inside of the TLB during translation. Need to populate the TLB with 
//  address translation by finding where it is located in physical
//  memory and evicting a TLB entry, in FIFO order (using tlbCounter).
//  We do this by:
//  (1) Getting the Virtual Page from the vaddr
//  (2) Finding the entry in Main Memory
//  (3) (If not in memory) Handle Core Map/IPT Miss
//  (4) Propagate changes to Page Table
//  (5) Evicting a TLB entry
//  (6) Updating TLB entry with the translation for the vaddr
//
//  "vaddr" -- the bad vaddr that caused the TLB miss
//
//  See also:   translate.cc    Raises PageFaultException during 
//                              failed translations
//----------------------------------------------------------------------

void PageFault_Handler(unsigned int vaddr)
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff); // Disable interrupts

    // (1) Get Virtual Page from bad vaddr
    unsigned int vpn = vaddr / PageSize;
    
    // (2) Find entry in Main Memory
    int ppn = -1;
    for (unsigned int i = 0; i < NumPhysPages; i++)
    {
        //PRINT VALUES OUT HERE, NOT FINDING IT WHEN IT SHOULD.
        printf("ipt virtual page: %d\n", ipt[i].virtualPage);
        printf("vpn: %d\n", vpn);
        printf("ipt valid: %d\n", ipt[i].valid);

        // Memory entry is valid, belongs to same Process and is for the same Virtual Page
        if (ipt[i].virtualPage == vpn && ipt[i].valid && ipt[i].space == currentThread->space)
        {

            ppn = i;
            break;
        }
    }

    // (3) Handle not in Main Memory
    if (ppn == -1)
    {
        DEBUG('p', "Page not in Main Memory, IPT Miss.\n");
        ppn = IPTMiss_Handler(vpn);
    }

    // (4) Evict TLB Entry in FIFO order
    tlbCounter++;
    int evictEntry = tlbCounter % TLBSize;

    // (5) Propagate changes to Page Table
    if (machine->tlb[evictEntry].dirty)
    {
        ipt[ppn].dirty = true;

        // TODO: What are we doing here?
        //  currentThread->space->pageTable[vpn].dirty = true;
    }

    // (6) Update TLB with translation for vpn -> ppn
    int newEntry = evictEntry;


    DEBUG('p', "PageFault: Updating TLB[ %d ]:\n \tvaddr\t%d\n \tvpn\t%d\n \tppn\t%d\n",
        newEntry, vaddr, vpn, ppn);


    machine->tlb[newEntry].virtualPage = vpn;
    machine->tlb[newEntry].physicalPage = ppn;
    machine->tlb[newEntry].valid = true; // Belongs to process
    machine->tlb[newEntry].use = ipt[ppn].use; // TODO: What is use used for?
    machine->tlb[newEntry].dirty = ipt[ppn].dirty; // TODO: What is dirty used for?

    (void) interrupt->SetLevel(oldLevel); // Restore interrupts
}

//========================================================================================================================================
//
// Exception Handler
//
//  See also:   syscall.h       System Call Codes and Interfaces
//              start.s         Assembly language assists for user 
//                              programs to call system calls
//              mipssum.cc      Raises SyscallExceptions while executing
//              translate.cc    Raises PageFaultException during 
//                              failed translations
//
//========================================================================================================================================

//----------------------------------------------------------------------
// ExceptionHandler
//  When machine encounters an Exception while executing a user program,
//  the OS needs to handle it. 
//
//  If the Exception is a SyscallException: 
//      (1) ExceptionHandler maps the Syscall Code (defined in 
//          syscall.h) to the Syscall function
//      (2) Calls the syscall: Load input from Registers 4, 5, 6, and 7
//      (3) (If the syscall returns a value) Put the return value in 
//          register 2, register for OP_LH/OP_LHU read from to store 
//          value in user prog register.
//      (4) Increment program counter (OneIntstruction before any 
//          execution)
//
//  If the Exception is a PageFaultException:
//      (1) Replace a TLB entry with the bad vaddr
//          (a) Already in mainMem: Copy entry to TLB from pageTable
//          (b) Not in mainMem: Load from executable to mainMem, update
//              IPT (space and vaddr), pageTable (location = mainMem),
//              and TLB (vaddr/paddr)
//          (c) Not in mainMem, mainMem is full: 
//              (1) Evict a page from mainMem (randomly or FIFO), write
//                  that page to SwapFile, propagate location change to 
//                  pageTable
//              (2) Do (b) for bad vaddr to update IPT
//      (2) Do NOT update program counter (now that data loaded to 
//          mem, process can execute instruction that threw Exception)
//
//  "which" -- type of Exception (Syscall, PageFault, etc.)
//----------------------------------------------------------------------

void ExceptionHandler(ExceptionType which) 
{
    int type = machine->ReadRegister(2); // Which syscall?
    int rv = 0; // the return value from a syscall

    if (which == SyscallException)
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
            rv = AcquireLock_Syscall(machine->ReadRegister(4));
            break;

            case SC_ReleaseLock:
            DEBUG('a', "Release Lock syscall.\n");
            rv = ReleaseLock_Syscall(machine->ReadRegister(4));
            break;

            case SC_DestroyLock:
            DEBUG('a', "Destroy Lock syscall.\n");
            rv = DestroyLock_Syscall(machine->ReadRegister(4));
            break;

            case SC_CreateCV:
            DEBUG('a', "CreateCV syscall.\n");
            rv = CreateCV_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
            break;

            case SC_Wait:
            DEBUG('a', "Wait syscall.\n");
            rv = Wait_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
            break;

            case SC_Signal:
            DEBUG('a', "Signal syscall.\n");
            rv = Signal_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
            break;

            case SC_Broadcast:
            DEBUG('a', "Broadcast syscall.\n");
            rv = Broadcast_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
            break;

            case SC_DestroyCV:
            DEBUG('a', "Destroy Condition syscall.\n");
            rv = DestroyCV_Syscall(machine->ReadRegister(4));
            break;

            case SC_PrintfOne:
            DEBUG('a', "Write one.\n");
            PrintfOne_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6));
            break;

            case SC_PrintfTwo:
            DEBUG('a', "Write two.\n");
            PrintfTwo_Syscall(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6), machine->ReadRegister(7));
            break;

            case SC_PrintError:
            DEBUG('a', "WriteInt syscall.\n");
            PrintError_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
            break;

            case SC_Random:
            DEBUG('a', "Random syscall.\n");
            rv = Random_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
            break;
        }

        // Put in the return value and increment the PC
        machine->WriteRegister(2, rv);
        machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
        machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
        machine->WriteRegister(NextPCReg, machine->ReadRegister(PCReg) + 4);
        return;
    }
    else if (which == PageFaultException)
    {
        // Bad vaddr in Register 39 by convention
        // TODO: Validate vaddr from CPU
        PageFault_Handler(machine->ReadRegister(BadVAddrReg));
        return;
    }
    else 
    {
        cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
        interrupt->Halt();
    }
}
