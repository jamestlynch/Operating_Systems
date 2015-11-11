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
using namespace std;


struct MonitorVariable
{
    string name;
    int *values;
    unsigned int size;

    MonitorVariable(string id, unsigned int arraysize)
    {
        name = id;
        size = arraysize;
        values = new int[arraysize];
    }
};

vector<MonitorVariable*> monitorvariables;

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
// void doCreateLock(char* name, PacketHeader inPktHdr){
//     ServerLock *sl= new ServerLock(name);
//     sl->machineID= inPktHdr.from;
//     sl->state= 0; //free
//     printf("create lock name: %s\n", name);
//     slocks.push_back(sl);
// }
// void doAcquireLock(int indexlock, PacketHeader inPktHdr){
//     if (slocks.at(indexlock)->state == 1){ //lock is valid but busy so go on wait queue
//             slocks.at(indexlock)->waitqueue.push((char*)inPktHdr.from); //(void*)inPktHdr.from)
//         }
//     else { // lock is valid and available so get the lock
//             slocks.at(indexlock)->state = 1;
//         }
// }
void doReleaseLock(int indexlock, PacketHeader inPktHdr){
        //check that the machineID trying to release the lock is the correct machine
        //id that owns the lock
        //slocks.at(indexlock)->waitqueue.pop_front();
    if (!slocks.at(indexlock)){
        printf("This lock index is null\n");
    }
    else if (!slocks.at(indexlock)->waitqueue.empty()){ // 
        //slocks.at(indexlock)->waitqueue.pop_front((char*)inPktHdr.from);
    }
    else{
        slocks.at(indexlock)->state = 0;
    }
    
}
void doDestroyLock(int indexlock){
        if(slocks.at(indexlock)->state == 0 ) // if lock is free and nobody is waiting
         {
            ServerLock * sl = slocks.at(indexlock);
            delete sl;
        }

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
// void doCreateCV(char* name, PacketHeader inPktHdr){

//     ServerCV *sc= new ServerCV(name);
//     printf("create server name: %s\n", name);
//     //sc->machineID= inPktHdr.from;
//     sc->waitqueue= new List();
//     sc->state= 0; //free
//     sconditions.push_back(sc);
// }
void doWaitCV(int indexlock, int indexcv){

}
void doSignalCV(int indexlock, int indexcv){

}
void doBroadcastCV(int indexlock, int indexcv){

}
void doDestroyCV(int indexlock, int indexcv){

}

void CreateMV(string mvname, int mvsize, int machineID, int mailboxID)
{
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;

    outPktHdr.to = machineID; // Server Machine ID
    outMailHdr.to = mailboxID; // Server Machine ID
    outMailHdr.from = 0; // Client Mailbox ID

    // Create the message
    std::stringstream ss;

    // Validate input
    if (mvsize <= 0)
    {
        ss << "400 CM" << " " << "-1";
    }
    else
    {
        MonitorVariable *mv = new MonitorVariable(mvname, mvsize);
        monitorvariables.push_back(mv);
        ss << "201 CM" << " " << monitorvariables.size() - 1;
    }

    char *message = (char *) ss.str().c_str();
    outMailHdr.length = strlen(message) + 1;

    DEBUG('n', "Server Sending Message to %d: %s\n", outPktHdr.to, message);

    // Send request
    bool success = postOffice->Send(outPktHdr, outMailHdr, message);
    if (!success)
    {
        printf("The PostOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
        interrupt->Halt();
    }
}

void SetMV(unsigned int indexmv, unsigned int indexvar, int value, int machineID, int mailboxID)
{
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;

    outPktHdr.to = machineID; // Server Machine ID
    outMailHdr.to = mailboxID; // Server Machine ID
    outMailHdr.from = 0; // Client Mailbox ID

    // Create the message
    std::stringstream ss;

    // Validate input
    if (indexmv >= monitorvariables.size())
    {
        ss << "400 SM";
    }

    MonitorVariable *mv = monitorvariables.at(indexmv);

    if (indexvar >= mv->size)
    {
        ss << "400 SM";
    }
    else if (indexmv < monitorvariables.size() && indexvar < mv->size)
    {
        mv->values[indexvar] = value;
        ss << "200 SM";
    }

    char *message = (char *) ss.str().c_str();
    outMailHdr.length = strlen(message) + 1;

    DEBUG('n', "Server Sending Message to %d: %s\n", outPktHdr.to, message);

    // Send request
    bool success = postOffice->Send(outPktHdr, outMailHdr, message);
    if (!success)
    {
        printf("The PostOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
        interrupt->Halt();
    }
}

void GetMV(unsigned int indexmv, unsigned int indexvar, int machineID, int mailboxID)
{
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;

    outPktHdr.to = machineID; // Server Machine ID
    outMailHdr.to = mailboxID; // Server Machine ID
    outMailHdr.from = 0; // Client Mailbox ID

    // Create the message
    std::stringstream ss;

    // Validate input
    if (indexmv >= monitorvariables.size())
    {
        ss << "400" << " " << "GM" << " " << "-1";
    }

    MonitorVariable *mv = monitorvariables.at(indexmv);

    if (indexvar >= mv->size)
    {
        ss << "400" << " " << "GM" << " " << "-1";
    }
    else if (indexmv < monitorvariables.size() && indexvar < mv->size)
    {
        ss << "200" << " " << "GM" << " " << mv->values[indexvar];
    }

    char *message = (char *) ss.str().c_str();
    outMailHdr.length = strlen(message) + 1;

    DEBUG('n', "Server Sending Message to %d: %s\n", outPktHdr.to, message);

    // Send request
    bool success = postOffice->Send(outPktHdr, outMailHdr, message);
    if (!success)
    {
        printf("The PostOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
        interrupt->Halt();
    }
}

//========================================================================================================================================
//
// Server
//  Client machines send request, Server handles requests and sends responses.
//
//  See also:   exception.cc   client networked system calls
//
//========================================================================================================================================

//----------------------------------------------------------------------
// Server
//  Handle Client requests by doing the following:
//  (1) Parse message:
//      (a) Determine which server method to call
//      (b) Get corresponding input
//  (2) Validate input
//  (3) Call method
//  (4) (If Machine does not need to wait for resource) Send response
//----------------------------------------------------------------------

void Server()
{
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char request[MaxMailSize];

    DEBUG('n', "Server is started.\n");

    while (true)
    {
        DEBUG('n', "Waiting for message.\n");
        postOffice->Receive(0, &inPktHdr, &inMailHdr, request); // Check Mailbox 0 until Message sent
        DEBUG('n', "Received \"%s\" from %d, box %d\n", request, inPktHdr.from, inMailHdr.from);

        // Clear stringstream to parse request
        stringstream ss;
        fflush(stdout);
        ss.str(request);

        char *requestType;
        char *lockname = new char;
        char *cvname = new char;
        char *response = "0";
        int length, lockindex, cvindex;
        bool success;
        ss >> requestType;

        DEBUG('n', "Type of RPC: %s\n", requestType);

        if (strcmp(requestType, "CL") == 0)
        {
            // char *lockname = new char;
            // int length

            // DEBUG('n', "Server received Create Lock request from client.\n");
            // ss >> lockname;
            // ss >> length;
            // if (length <= 0)
            // {
            //     printf("%s","Length for locks's identifier name must be nonzero and positive\n");
            //     response = "-1";
            //     break;
            // }
            // else
            // {
            // doCreateLock(lockname, inPktHdr);
            // printf("After creating the lock\n");

            // }
        }
        else if (strcmp(requestType, "AL") == 0) //need to pass index
             {
                // printf("Server received Acquire Lock request from client.\n");
                // ss >> lockindex;
                // int size = slocks.size();
                // if (dovalidatelockindex(lockindex, inPktHdr)==-1){
                //     printf("%s","acquire lock: lock index invalid or wrong machine.\n");
                //     response="-1";
                //     break;
                // }
                // else{
                //     doAcquireLock(lockindex, inPktHdr);
                //     printf("After acquiring the lock\n");
                // }
            }
        else if (strcmp(requestType, "RL") == 0) //need to pass index
             {
                printf("Server received Release Lock request from client.\n");
                ss >> lockindex;
                int size = slocks.size();
                if (dovalidatelockindex(lockindex, inPktHdr)==-1){
                    printf("%s","release lock: lock index invalid or wrong machine.\n");
                    response="-1";
                    break;
                }
                else
                {
                    //printf("lockname is: %d", lockname);
                    doReleaseLock(lockindex, inPktHdr);
                    printf("After creating the lock\n");
                }
            }
        else if (strcmp(requestType, "DL") == 0)
             {
                printf("Server received Destroy Lock request from client.\n");
                ss >> lockindex;
                int size = slocks.size();
                if (dovalidatelockindex(lockindex, inPktHdr)==-1){
                    printf("%s","destroy lock: lock index invalid or wrong machine.\n");
                    response="-1";
                    break;
                }
                else{
                //printf("lockname is: %d", lockname);
                doDestroyLock(lockindex);
                printf("the lock at specified index has been destroyed\n");
        
                }
            }
        else if (strcmp(requestType, "CC") == 0)
             {
                // printf("Server received Create Condition request from client.\n");
                
                // ss >> cvname;
                // ss >> length;
                // int locksize = sconditions.size();
                // if (length <= 0)
                // {
                //     printf("%s","Length for cv's identifier name must be nonzero and positive\n");
                //     response="-1";
                //     //bigServerCV->Release();
                //     break;
                // }
                // else{
                // //printf("lockname is: %d", lockname);
                // doCreateCV(lockname, inPktHdr);
                // printf("After creating the cv\n");
                // }
            }
        else if (strcmp(requestType, "WC") == 0)
             {
                printf("Inside if else statement\n");
        
            }
        else if (strcmp(requestType, "SC") == 0)
             {
                printf("Inside if else statement\n");
        
            }
        else if (strcmp(requestType, "BC") == 0)
             {
                printf("Inside if else statement\n");
        
            }
        else if (strcmp(requestType, "DC") == 0)
             {
                printf("Inside if else statement\n");
        
            }
        else if (strcmp(requestType, "CM") == 0)
        {
            string mvname;
            int mvsize;

            ss >> mvname >> mvsize;

            CreateMV(mvname, mvsize, inPktHdr.from, inMailHdr.from);
        }
        else if (strcmp(requestType, "SM") == 0)
        {
            unsigned int indexmv, indexvar;
            int value;

            ss >> indexmv >> indexvar >> value;

            SetMV(indexmv, indexvar, value, inPktHdr.from, inMailHdr.from);
        }
        else if (strcmp(requestType, "GM") == 0)
        {
            unsigned int indexmv, indexvar;

            ss >> indexmv >> indexvar;

            GetMV(indexmv, indexvar, inPktHdr.from, inMailHdr.from);
        }
    }
}




//========================================================================================================================================
//
// MailTest
//  Tests simple message passing between a Client and Server Machine
//
//========================================================================================================================================

void MailTest(int farAddr)
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
