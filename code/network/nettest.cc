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
#include <queue>


// Test out message delivery, by doing the following:
//	1. send a message to the machine with ID "farAddr", at mail box #0
//	2. wait for the other machine's message to arrive (in our mailbox #0)
//	3. send an acknowledgment for the other machine's message
//	4. wait for an acknowledgement from the other machine to our 
//	    original message

int dovalidatelockindex(int index, PacketHeader inPktHdr)
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
    if (currentServerLock->machineID != inPktHdr.from)
    {
        printf("Lock %d does not belong to the current process.\n", index);
        return -1;
    }

    return 0;
}
void doCreateLock(char* name, PacketHeader inPktHdr){
    ServerLock *sl= new ServerLock(name);
    sl->machineID= inPktHdr.from;
    sl->state= 0; //free
    printf("create lock name: %s\n", name);
    slocks.push_back(sl);
}
void doAcquireLock(int indexlock){
    
        if (slocks.at(indexlock)->state == 1){ //lock is valid but busy so go on wait queue
                slocks.at(indexlock)->waitqueue.push_back(/**/); //(void*)inPktHdr.from)
            }
        else { // lock is valid and available so get the lock
                slocks.at(indexlock)->state = 1;
            }
    
}
void doReleaseLock(int indexlock){
        //check that the machineID trying to release the lock is the machine
        //id that owns the lock
        //slocks.at(indexlock)->waitqueue.pop_front();
    
}
void doDestroyLock(int indexlock){


}
int dovalidatecvindeces(int indexcv, int indexlock, PacketHeader inPktHdr)
{
    // (1) index to valid location
    if (indexcv < 0 || indexlock < 0)
    {
        printf("%s","Invalid index.\n");
        return -1;
    }

    int csize = sconditions.size();
    int lsize = slocks.size();

    // (1) index to valid location
    if (indexcv > csize - 1 || indexlock > lsize - 1)
    {
        printf("%s","Index out of bounds.\n");
        return -1;
    }

    ServerCV * currentServerCV = sconditions.at(indexcv);
    ServerLock * currentServerLock = slocks.at(indexlock);

    // (2) index to defined lock and cv
    if (!currentServerCV  || !currentServerLock)
    {
        printf("Condition %d is set to NULL or Lock %d is set to NULL.\n", indexcv, indexlock);
        return -1;
    }

    // (3) lock belongs to process
    /*if (currentServerLock->machineID != THE MACHINE ID WE PASSED IN)
    {
        printf("Lock %d belongs to a different client.\n", indexlock);
        return -1;
    }

    // (3) cv belongs to process
    if (currentServerCV->machineID != THE MACHINE ID WE PASSED IN)
    {
        printf("Condition %d belongs to a different client.\n", indexcv);
        return -1;
    }*/

    return 0;
}
void doCreateCV(char* name, PacketHeader inPktHdr){

    ServerCV *sc= new ServerCV(name);
    printf("create server name: %s\n", name);
    //sc->machineID= inPktHdr.from;
    sc->waitqueue= new List();
    sc->state= 0; //free
    sconditions.push_back(sc);
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

    while (true){
        printf("Server is started. waiting for message.\n");
        postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
        printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout);


        ss.str(buffer);

        char* type= new char;
        char *lockname= new char;
        char *cvname= new char;
        char* response="0";
        int length, lockindex, cvindex;
        bool success;
        ss >> type;

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

    // (1) index to valid location

       if (strcmp(type, "CL") == 0)
             {
                bigServerLock->Acquire();
                printf("Server received Create Lock request from client.\n");
                ss >> lockname;
                ss >> length;
                if (length <= 0)
                {
                    printf("%s","Length for locks's identifier name must be nonzero and positive\n");
                    response="-1";
                    bigServerLock->Release();
                    break;
                }
                else{
                //printf("lockname is: %d", lockname);
                doCreateLock(lockname, inPktHdr);
                printf("After creating the lock\n");
                bigServerLock->Release();
                }
            }
        else if (strcmp(type, "AL") == 0) //need to pass index
             {
                bigServerLock->Acquire();
                printf("Server received Acquire Lock request from client.\n");
                ss >> lockindex;
                int size = slocks.size();
                if (dovalidatelockindex(lockindex, inPktHdr)==-1){
                    printf("%s","acquire lock: lock index invalid or wrong machine.\n");
                    response="-1";
                    bigServerLock->Release();
                    break;
                }
                else{
                    doAcquireLock(lockindex);
                    printf("After acquiring the lock\n");
                    bigServerLock->Release();
                }
            }
        else if (strcmp(type, "RL") == 0) //need to pass index
             {
                printf("Server received Release Lock request from client.\n");
                bigServerLock->Acquire();
                ss >> lockindex;
                int size = slocks.size();
                if (dovalidatelockindex(lockindex, inPktHdr)==-1){
                    printf("%s","release lock: lock index invalid or wrong machine.\n");
                    response="-1";
                    bigServerLock->Release();
                    break;
                }
                else
                {
                    //printf("lockname is: %d", lockname);
                    doReleaseLock(lockindex);
                    printf("After creating the lock\n");
                    bigServerLock->Release();
                }
            }
        else if (strcmp(type, "DL") == 0)
             {
                printf("Server received Destroy Lock request from client.\n");
                bigServerLock->Acquire();
                ss >> lockindex;
                int size = slocks.size();
                if (dovalidatelockindex(lockindex, inPktHdr)==-1){
                    printf("%s","destroy lock: lock index invalid or wrong machine.\n");
                    response="-1";
                    bigServerLock->Release();
                    break;
                }
                else{
                //printf("lockname is: %d", lockname);
                doDestroyLock(lockindex);
                printf("the lock at specified index has been destroyed\n");
                bigServerLock->Release();
                }
            }
        /*else if (strcmp(type, "CC") == 0)
             {
                bigServerCV->Acquire();
                printf("Server received Create Condition request from client.\n");
                
                ss >> cvname;
                ss >> length;
                int locksize = sconditions.size();
                if (length <= 0)
                {
                    printf("%s","Length for cv's identifier name must be nonzero and positive\n");
                    response="-1";
                    //bigServerCV->Release();
                    break;
                }
                else{
                //printf("lockname is: %d", lockname);
                doCreateCV(lockname);
                printf("After creating the cv\n");
                bigServerCV->Release();
                }
            }
        else if (strcmp(type, "WC") == 0)
             {
                bigServerLock->Acquire();
                printf("Inside if else statement\n");
                bigServerLock->Release();
            }
        else if (strcmp(type, "SC") == 0)
             {
                bigServerLock->Acquire();
                printf("Inside if else statement\n");
                bigServerLock->Release();
            }
        else if (strcmp(type, "BC") == 0)
             {
                bigServerLock->Acquire();
                printf("Inside if else statement\n");
                bigServerLock->Release();
            }
        else if (strcmp(type, "DC") == 0)
             {
                bigServerLock->Acquire();
                printf("Inside if else statement\n");
                bigServerLock->Release();
            }*/
        if(outPktHdr.to != -1) 
            {
                outPktHdr.to = 1;
                outPktHdr.from= 0;      
                outMailHdr.to = 0;
                outMailHdr.from = 0;
                
                printf("outmailhdr.to= %d, outmailhdr.from= %d, outPktHdr.to=%d, outPktHdr.from=%d\n", outMailHdr.to, outMailHdr.from, outPktHdr.to, outPktHdr.from);
                outMailHdr.length = strlen(response) + 1;
                printf("Sending packet.\n");
                printf("Response: %s", response);
                //  printf("Data: %s, outPktHdr.to: %d, outMailHrd.to: %d, outPktHdr.from: %d, outMailHdr.length: %d \n", data, outPktHdr.to,outMailHdr.to, outPktHdr.from, outMailHdr.length);
                success = postOffice->Send(outPktHdr, outMailHdr, response);
                printf("Sent packet\n");
                if(!success){
                    printf("postOffice send failed. Terminating...\n");
                    interrupt->Halt();
                }
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
