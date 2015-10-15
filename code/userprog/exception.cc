// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
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

using namespace std;

int copyin(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n=0;			// The number of bytes copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) {
      result = machine->ReadMem( vaddr, 1, paddr );
      while(!result) // FALL 09 CHANGES
	  {
   			result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
	  }	
      
      buf[n++] = *paddr;
     
      if ( !result ) {
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
    int n=0;			// The number of bytes copied in

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
    char *buf = new char[len+1];	// Kernel buffer to put the name in

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
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    OpenFile *f;			// The new open file
    int id;				// The openfile id

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
    
    char *buf;		// Kernel buffer for output
    OpenFile *f;	// Open file for output

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

int Read_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one.    We reuse len as the number of bytes
    // read, which is an unnessecary savings of space.
    char *buf;		// Kernel buffer for input
    OpenFile *f;	// Open file for output

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
int 
CreateLock(unsigned int vaddr, int len) {

  if (len <= 0) { // Validate length is nonzero and positive
    printf("%s","Invalid length for lock identifier\n");
    return -1;
  }

  char *buf;

  if ( !(buf = new char[len]) ) { // If error allocating memory for character buffer
    printf("%s","Error allocating kernel buffer for creating new lock!\n");
    return -1;
  } else {
    if ( copyin(vaddr,len,buf) == -1 ) { // If failed to read memory from vaddr passed in
      printf("%s","Bad pointer passed to create new lock\n");
      delete[] buf;
      return -1;
    }
  }

  buf[len] = '\0'; // Finished grabbing the identifier for the Lock, add null terminator character

  locksLock->Acquire(); //acquire table lock
  
  KernelLock * newKernelLock = new KernelLock();

  Lock *lock = new Lock(buf);  
  newKernelLock->toDelete = false;
  newKernelLock->space = currentThread->space;
  newKernelLock->lock = lock;

  //put the new kernel lock object into the lock table
  locks.push_back(*newKernelLock);
  locksLock->Release();

  delete[] buf;
  return 0; // TODO: Return the index of the lock
}

// AcquireLock
//  Input:
//    int index – the index in the lock table that user program is requesting
//    * Within the bounds of the vector
//    * Lock they're trying to acquire belongs to their process 
//    * Index is positive
//  
void
AcquireLock(int index){

  if (index < 0) {
    printf("%s","Invalid lock table index\n");
    return;
  }

  locksLock->Acquire(); // Synchronize lock access, subsequent threads will go on queue

  if (index > locks.size()) {
    printf("%s","Invalid lock table index\n");
    locksLock->Release();
    return;
  }

  KernelLock * kernelLock = locks.at(index);
  locksLock->Release();



  // TODO: if lock is set toDestroy == TRUE, prevent other threads from acquiring



  if (kernelLock->space != currentThread.space) {
    printf("%s","Lock does not belong to the current process");
    locksLock->Release();
    return;
  }

  kernelLock->lock->Acquire();
  return;
}

// ReleaseLock()
//  
void 
ReleaseLock(int index){
  if (index < 0) {
    printf("%s","Invalid lock table index\n");
    return;
  }

  locksLock->Acquire(); // Synchronize lock access, subsequent threads will go on queue

  if (index > locks.size()) {
    printf("%s","Invalid lock table index\n");
    locksLock->Release();
    return;
  }

  KernelLock * kernelLock = locks.at(index);
  locksLock->Release();



  // TODO: Do we need to check if lock is set toDestroy == TRUE



  if (kernelLock->space != currentThread.space) {
    printf("%s","Lock does not belong to the current process");
    locksLock->Release();
    return;
  }

  kernelLock->lock->Release();
  return;
}

// DestroyLock
//  toDestroy = TRUE
//  if no waiting threads, delete it here
//  Where do we delete / detect all waiting threads finishing?
void DestroyLock(int index){
  // lockTlock->Acquire();

  // //get index of lock to be destroyed, from lock table.
  // // make sure the lock being destroyed is allowed to be destroyed....who creates and destroys locks?
  // //remove from lock table...free memory

  // lockTlock->Release();
}
void CreateCV(){
  // cvTLock->Acquire();
  // kernelCondition * newkcond = new kernelCondition();

  // Condition *newkcond= new Condition(/*NAME*/);  //use a buffer to get the name. create a new kernel lock object. set all values.
  // newkcond->toDelete = false;
  // newkcond->as = currentThread->space;
  // newkcond->condition = lock;

  // //put the new kernel lock object into the lock table
  // cvT->Put(newkcond);
  // /*
  // TO DO 
  // make sure there is available cv in cvT and all aren't being used.
  // */
  // cvTLock->Release();
}
void Wait(){
  // cvTLock->Acquire();
  

  // cvTLock->Release();
}
void Signal(){
  // cvTLock->Acquire();
  // //DO STUFF
  // cvTLock->Release();
}
void Broadcast(){
  // cvTLock->Acquire();
  // //DO STUFF
  // cvTLock->Release();
}
void DestroyCV(){
  // cvTLock->Acquire();
  // //DO STUFF
  // cvTLock->Release();
}
void Halt(){
  // interrupt->Halt();
}

void Yield_Syscall() {
  currentThread->Yield();
}

void Exit_Syscall(){


// currentThread->Finish(); //needs to be in here according to piazza
}
void Fork_Syscall(/*void (*func)*/){

}
void Exec_Syscall(){

}

void Join_Syscall(){

}


void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv=0; 	// the return value from a syscall

    if ( which == SyscallException ) {
	switch (type) {
	    default:
		DEBUG('a', "Unknown syscall - shutting down.\n");
	    case SC_Halt:
		DEBUG('a', "Shutdown, initiated by user program.\n");
		interrupt->Halt();
		break;
      case SC_Exit:
    DEBUG('a', "Exit Syscall.\n");
    Exit_Syscall();
    break;
      case SC_Exec:
      DEBUG('a', "Exec syscall.\n");
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
	    case SC_Write:
		DEBUG('a', "Write syscall.\n");
		Write_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
		break;
	    case SC_Read:
		DEBUG('a', "Read syscall.\n");
		rv = Read_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
		break;
	    case SC_Close:
		DEBUG('a', "Close syscall.\n");
		Close_Syscall(machine->ReadRegister(4));
		break;
      case SC_Fork:
    DEBUG('a', "Fork syscall.\n");
    Fork_Syscall();
    break;
      case SC_Yield:
    DEBUG('a', "Yield syscall.\n");
    Yield_Syscall();
    break;
    

	}

	// Put in the return value and increment the PC
	machine->WriteRegister(2,rv);
	machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
	machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
	machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
	return;
    } else {
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }
}
