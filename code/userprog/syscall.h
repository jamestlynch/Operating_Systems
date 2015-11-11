/* syscalls.h 																	*/
/*	Nachos system call interface.  These are Nachos kernel operations 			*/
/*	that can be invoked from user programs, by trapping to the kernel 			*/
/*	via the "syscall" instruction. Sets up system call codes and defines		*/
/*	the system call interfaces.													*/
/*																				*/
/*	This file is included by user programs and by the Nachos kernel. 			*/
/*																				*/
/* Copyright (c) 1992-1993 The Regents of the University of California.			*/
/* All rights reserved.  See copyright.h for copyright notice and limitation 	*/
/* of liability and disclaimer of warranty provisions.							*/

#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "copyright.h"

/*========================================================================================================================================	*/
/*																																			*/
/* System Call Codes 																														*/
/*	Used by the stubs to tell the kernel which system call is being 																		*/
/*	requested. 																																*/
/*																																			*/
/*========================================================================================================================================	*/

#define SC_Halt				0
#define SC_Exit				1
#define SC_Exec				2
#define SC_Join				3
#define SC_Create			4
#define SC_Open				5
#define SC_Read				6
#define SC_Write			7
#define SC_Close			8
#define SC_Fork				9
#define SC_Yield			10
#define SC_CreateLock		11
#define SC_AcquireLock		12
#define SC_ReleaseLock		13
#define SC_DestroyLock		14
#define SC_CreateCV			15
#define SC_Wait				16
#define SC_Signal			17	
#define SC_Broadcast		18	
#define SC_DestroyCV		19
#define SC_Random			20
#define SC_PrintError		21
#define SC_PrintfOne		22
#define SC_PrintfTwo		23
#define SC_CreateMonitor	24
#define SC_SetMonitor		25
#define SC_GetMonitor 		26
#define SC_DestroyMonitor	27

#define MAXFILENAME 256

#ifndef IN_ASM

/*========================================================================================================================================	*/
/*																																			*/
/* System Call Interfaces 																													*/
/*	These are the operations the Nachos kernel needs to support, to be 																		*/
/*	able to run user programs. Each of these is invoked by a user  																			*/
/*	program by simply calling the procedure; an assembly language stub  																	*/
/*	stuffs the system call code into a register (in start.s), and traps  																	*/
/*	to the kernel. The kernel procedures are then invoked in the Nachos  																	*/
/*	kernel, after appropriate error checking, from the system call entry  																	*/
/*	point in exception.cc. 																													*/
/*																																			*/
/*	See also: 	exception.cc 	System Call implementations																					*/
/*				start.s 		Assembly language assists for user 																			*/
/*								programs to call system calls																				*/
/*																																			*/
/*========================================================================================================================================	*/


/*========================================================================	*/
/*																			*/
/*	User-level process/thread operations 									*/
/*																			*/
/*========================================================================	*/

typedef int SpaceId; /* A unique identifier for an executing user program (address space) */

/*------------------------------------------------------------------------	*/
/* Halt 																	*/
/*	Stop Nachos, and print out performance status 							*/
/*------------------------------------------------------------------------	*/

void Halt();
 
/*------------------------------------------------------------------------	*/
/* Exec 																	*/
/*	Run the executable, stored in the Nachos file "name", and return the  	*/
/*	address space identifier.												*/
/*------------------------------------------------------------------------	*/

/*SpaceId Exec(char *name, int len);*/
void Exec(char *name, int len);

/*------------------------------------------------------------------------	*/
/* Fork 																	*/
/*	Fork a thread to run a procedure ("func") in the *same* address space 	*/
/*	as the current thread.													*/
/*------------------------------------------------------------------------	*/

void Fork(char *name, int len, void (*func)());

/*------------------------------------------------------------------------	*/
/* Yield 																	*/
/*	Yield the CPU to another runnable thread, whether in this address  		*/
/*	space or not. 															*/
/*------------------------------------------------------------------------	*/

void Yield(); 

/*------------------------------------------------------------------------	*/
/* Exit 																	*/
/* 	This user thread is done (status = 0 means exited normally).			*/
/*------------------------------------------------------------------------	*/

void Exit(int status);	

/*------------------------------------------------------------------------	*/
/* Join 																	*/
/*	(Unimplemented)															*/
/*	Only return once the the user program "id" has finished. Return the  	*/
/*	exit status.															*/
/*------------------------------------------------------------------------	*/

int Join(SpaceId id);	
 

/*========================================================================	*/
/*																			*/
/* File system operations: Create, Open, Read, Write, Close 				*/
/*	These functions are patterned after UNIX -- files represent both 		*/
/*	files *and* hardware I/O devices.										*/
/*																			*/
/* Note: 	If this assignment is done before doing the file system 		*/
/*			assignment, note that the Nachos file system has a stub 		*/
/*			implementation, which will work for the purposes of testing  	*/
/*			out these routines.												*/
/*																			*/
/*========================================================================	*/

/* When an address space starts up, it has two open files, representing */
/* keyboard input and display output (in UNIX terms, stdin and stdout).	*/
/* Read and Write can be used directly on these, without first opening 	*/
/* the console device.													*/

#define ConsoleInput	0
#define ConsoleOutput	1

typedef int OpenFileId; /* A unique identifier for an open Nachos file. */

/*------------------------------------------------------------------------	*/
/* Create 																	*/
/*	Create a Nachos file, with "name" 										*/
/*------------------------------------------------------------------------	*/

void Create(char *name, int size);

/*------------------------------------------------------------------------	*/
/* Open 																	*/
/*	Open the Nachos file "name", and return an "OpenFileId" that can be  	*/
/*	used to read and write to the file.										*/
/*------------------------------------------------------------------------	*/

OpenFileId Open(char *name, int size);

/*------------------------------------------------------------------------	*/
/* Write 																	*/
/*	Write "size" bytes from "buffer" to the open file.						*/
/*------------------------------------------------------------------------	*/

void Write(char *buffer, int size, OpenFileId id);

/*------------------------------------------------------------------------	*/
/* Read 																	*/
/* 	Read "size" bytes from the open file into "buffer". Return the 			*/
/*	number of bytes actually read -- if the open file isnt long enough, 	*/
/*	or if it is an I/O device, and there arent enough characters to 		*/
/*	read, return whatever is available (for I/O devices, you should 		*/
/*	always wait until you can return at least one character).				*/
/*------------------------------------------------------------------------	*/

int Read(char *buffer, int size, OpenFileId id);

/*------------------------------------------------------------------------	*/
/* Close 																	*/
/*	Close the file, were done reading and writing to it.					*/
/*------------------------------------------------------------------------	*/

void Close(OpenFileId id);

/*========================================================================	*/
/*																			*/
/* Synchronization Objects operations										*/
/*	Locks and Conditions for user programs to synchronize threads.			*/
/*																			*/
/*========================================================================	*/

/*------------------------------------------------------------------------	*/
/* CreateLock 																*/
/*	Creates lock with name and returns index to lock for future access.		*/
/*------------------------------------------------------------------------	*/

int CreateLock(char *name, int size);

/*------------------------------------------------------------------------	*/
/* AcquireLock 																*/
/*	Thread acquires lock if indexlock corresponds to a lock with no 		*/
/*	current owner and belongs to currentThreads process.					*/
/*------------------------------------------------------------------------	*/

int AcquireLock(int indexlock);

/*------------------------------------------------------------------------	*/
/* ReleaseLock 																*/
/*	Thread releases lock if indexlock corresponds to a lock it owns and 	*/
/*	belongs to currentThreads process.										*/
/*------------------------------------------------------------------------	*/

int ReleaseLock(int indexlock);

/*------------------------------------------------------------------------	*/
/* DestroyLock 																*/
/*	Marks lock for deletion and deletes lock when there are no more  		*/
/*	waiting threads.														*/
/*------------------------------------------------------------------------	*/

int DestroyLock(int indexlock);

/*------------------------------------------------------------------------	*/
/* CreateCV 																*/
/*	Creates CV with name and returns index to CV for future access.			*/
/*------------------------------------------------------------------------	*/

int CreateCV(char *name, int size);

/*------------------------------------------------------------------------	*/
/* Wait 																	*/
/*	Thread waits on condition if lock at indexlock corresponds to the  		*/
/*	lock belonging to the condition, currentThread owns the lock and CV 	*/
/*	belongs to the currentThreads process.									*/
/*------------------------------------------------------------------------	*/

int Wait(int indexcv, int indexlock);

/*------------------------------------------------------------------------	*/
/* Signal 																	*/
/*	Thread signals condition if lock at indexlock corresponds to the 		*/
/*	lock belonging to the condition, currentThread owns the lock and CV 	*/
/*	belongs to the currentThreads process.									*/
/*------------------------------------------------------------------------	*/

int Signal(int indexcv, int indexlock);

/*------------------------------------------------------------------------	*/
/* Broadcast 																*/
/*	Thread broadcasts condition if lock at indexlock corresponds to the 	*/
/*	lock belonging to the condition, currentThread owns the lock and CV 	*/
/*	belongs to the currentThreads process.									*/
/*------------------------------------------------------------------------	*/

int Broadcast(int indexcv, int indexlock);	

/*------------------------------------------------------------------------	*/
/* DestroyCV 																*/
/*	Marks CV for deletion and deletes CV when there are no more waiting		*/
/*	threads.																*/
/*------------------------------------------------------------------------	*/

int DestroyCV(int indexcv);


/*========================================================================	*/
/*																			*/
/* Additional Syscall operations											*/
/*																			*/
/*========================================================================	*/

/*------------------------------------------------------------------------	*/
/* PrintfTwo 																*/
/*	Equivalent to printf with one integer-formatted parameters.				*/
/*------------------------------------------------------------------------	*/

void PrintfOne(char* buffer, int size, int num1);

/*------------------------------------------------------------------------	*/
/* PrintfTwo 																*/
/*	Equivalent to printf with two integer-formatted parameters.				*/
/*------------------------------------------------------------------------	*/

void PrintfTwo(char* buffer, int size, int num1, int num2);

/*------------------------------------------------------------------------	*/
/* PrintError 																*/
/*	Convenience method for red console output while Debugging/Testing.		*/
/*------------------------------------------------------------------------	*/

void PrintError(char *buffer, int size);

/*------------------------------------------------------------------------	*/
/* Random 																	*/
/*	Kernel has much more convenient ways of retrieving random values. 		*/
/*------------------------------------------------------------------------	*/

int Random(int lower, int upper);


/*========================================================================	*/
/*																			*/
/* Remote Sharing operations												*/
/*	Monitor variables allow data sharing between multiple programs remotely	*/
/*																			*/
/*========================================================================	*/

/*------------------------------------------------------------------------	*/
/* CreateMonitor 															*/
/*	Creates monitor variable array of "arraysize" with "id" for reference	*/
/*	and returns "indexmv" to MV for future access.							*/
/*------------------------------------------------------------------------	*/

int CreateMonitor(char *id, int idlength, int arraysize);

/*------------------------------------------------------------------------	*/
/* SetMonitor 																*/
/*	Sets "indexvar" variable inside of MV array at "indexmv" to "value."	*/
/*------------------------------------------------------------------------	*/

int SetMonitor(int indexmv, int indexvar, int value);

/*------------------------------------------------------------------------	*/
/* GetMonitor 																*/
/*	Gets "indexvar" variable inside of MV array at "indexmv."				*/
/*------------------------------------------------------------------------	*/

int GetMonitor(int indexmv, int indexvar);

/*------------------------------------------------------------------------	*/
/* DestroyMonitor 															*/
/*	Deletes Monitor Variable array at "indexmv"								*/
/*------------------------------------------------------------------------	*/

int DestroyMonitor(int indexmv);

#endif /* IN_ASM */

#endif /* SYSCALL_H */
