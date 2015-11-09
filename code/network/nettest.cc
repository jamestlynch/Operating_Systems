// nettest.cc 
//	Test out message delivery between two "Nachos" machines,
//	using the Post Office to coordinate delivery.
//
//	Two caveats:
//	  1. Two copies of Nachos must be running, with machine ID's 0 and 1:
//		./nachos -m 0 -o 1 &
//		./nachos -m 1 -o 0 &
//
//	  2. You need an implementation of condition variables,
//	     which is *not* provided as part of the baseline threads 
//	     implementation.  The Post Office won't work without
//	     a correct implementation of condition variables.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"
#include <sstream>


// Test out message delivery, by doing the following:
//	1. send a message to the machine with ID "farAddr", at mail box #0
//	2. wait for the other machine's message to arrive (in our mailbox #0)
//	3. send an acknowledgment for the other machine's message
//	4. wait for an acknowledgement from the other machine to our 
//	    original message

int dovalidatelockindex(int index)
{
    // (1) Index corresponds to valid location
    if (index < 0)
    {
        printf("%s","Invalid lock table index, negative.\n");
        return -1;
    }

    int size = slocks.size();

    // (1) Index corresponds to valid location
    if (index > size - 1)
    {
        printf("%s","Invalid lock table index, bigger than size.\n");
        return -1;
    }

    ServerLock * currentServerLock = slocks.at(index);

    // (2) Defined lock
    if (!currentServerLock)
    {
        printf("Lock %d is NULL.\n", index);
        return -1;
    }

    return 0;
}
void doCreateLock(char* name){

    //acquire the server lock
    bigServerLock->Acquire();
    printf("creating a lock inside of server");
    
    ServerLock *sl= new ServerLock(name);
    printf("create lock name: %s", name);
    sl->waitqueue = new List();
    slocks.push_back(sl);
    bigServerLock->Release();
    return;
}
void doAcquireLock(int indexlock){

}
void doReleaseLock(int indexlock){

}
void doDestroyLock(int indexlock){

}
int dovalidatecvindeces(int indexcv, int indexlock)
{
    // // (1) index to valid location
    // if (indexcv < 0 || indexlock < 0)
    // {
    //     printf("%s","Invalid index.\n");
    //     return -1;
    // }

    // int csize = conditions.size();
    // int lsize = slocks.size();

    // // (1) index to valid location
    // if (indexcv > csize - 1 || indexlock > lsize - 1)
    // {
    //     printf("%s","Index out of bounds.\n");
    //     return -1;
    // }

    // KernelCV * currentKernelCV = conditions.at(indexcv);
    // KernelLock * currentKernelLock = slocks.at(indexlock);

    // // (2) index to defined lock and cv
    // if (!currentKernelCV  || !currentKernelLock)
    // {
    //     printf("Condition %d is set to NULL or Lock %d is set to NULL.\n", indexcv, indexlock);
    //     return -1;
    // }

    // // (3) lock belongs to process
    // if (currentKernelLock->space != currentThread->space)
    // {
    //     printf("Lock %d does not belong to the current process.\n", indexlock);
    //     return -1;
    // }

    // // (3) cv belongs to process
    // if (currentKernelCV->space != currentThread->space)
    // {
    //     printf("Condition %d does not belong to the current process.\n", indexcv);
    //     return -1;
    // }

    return 0;
}
void doCreateCV(char* name){
    ServerCV *sc= new ServerCV(name);
    sc->waitqueue= new List();
    return;
}
void doWaitCV(int indexlock, int indexcv){

}
void doSignalCV(int indexlock, int indexcv){

}
void doBroadcastCV(int indexlock, int indexcv){

}
void doDestroyCV(int indexlock, int indexcv){

}

void Server(){
    
    stringstream ss;
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;

    char buffer[MaxMailSize];
    char *data = "Requested is complete";

    while (true){
        printf("Server is started. waiting for message.\n");
        postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
        printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout);

        outPktHdr.to = inPktHdr.from;     
        outMailHdr.to = 0;//machine id
        outMailHdr.from = 0;

        ss.str(buffer);

        char* type= new char;
        char *lockname = new char;
        ss>> type;
        outMailHdr.length = strlen(data) + 1;

        //----------------------------------------------------------------------
        // SERVER stuff
        // get information passed from the client above. read it in and
        // determine what syscall is being requested. then parse the rest
        // of the request depending on the data it needs
        // PARAMETERS:
        // CREATE LOCK AND CREATE CV NEED: name
        // DESTROY LOCK AND DESTORY CV NEED: index
        // ACQUIRE / RELEASE NEED: indexlock
        // WAIT / SIGNAL / BROADCAST NEED: indexcv and indexlock
        // 
        //----------------------------------------------------------------------
       printf("Type of call: %s\n", type);
       if (strcmp(type, "CL") == 0)
             {
                printf("Server received Create Lock request from client.\n");
                ss >> lockname;
                //printf("lockname is: %d", lockname);
                doCreateLock(lockname);
                
            }
        else if (strcmp(type, "AL") == 0) //need to pass index
             {
                printf("Inside if else statement\n");
            }
        else if (strcmp(type, "RL") == 0) //need to pass index
             {
                printf("Inside if else statement\n");
            }
        else if (strcmp(type, "DL") == 0)
             {
                printf("Inside if else statement\n");
            }
        else if (strcmp(type, "CC") == 0)
             {
                printf("Inside if else statement\n");
            }
        else if (strcmp(type, "WC") == 0)
             {
                printf("Inside if else statement\n");
            }
        else if (strcmp(type, "SC") == 0)
             {
                printf("Inside if else statement\n");
            }
        else if (strcmp(type, "BC") == 0)
             {
                printf("Inside if else statement\n");
            }
        else if (strcmp(type, "DC") == 0)
             {
                printf("Inside if else statement\n");
            }
            
        }
    }

void
MailTest(int farAddr)
{
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char *data = "Hello there!";
    char *ack = "Got it!";
    char buffer[MaxMailSize];

    //creating 

    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = farAddr;		
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outMailHdr.length = strlen(data) + 1;

    // Send the first message
    bool success = postOffice->Send(outPktHdr, outMailHdr, data); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the first message from the other machine
    postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);

    // Send acknowledgement to the other machine (using "reply to" mailbox
    // in the message that just arrived
    outPktHdr.to = inPktHdr.from;
    outMailHdr.to = inMailHdr.from;
    outMailHdr.length = strlen(ack) + 1;
    success = postOffice->Send(outPktHdr, outMailHdr, ack); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the ack from the other machine to the first message we sent.
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);

    // Then we're done!
    interrupt->Halt();
}
