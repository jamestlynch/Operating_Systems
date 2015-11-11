// nettest.cc 
//  Test out message delivery between two "Nachos" machines,
//  using the Post Office to coordinate delivery.
//
//  Two caveats:
//    1. Two copies of Nachos must be running, with machine ID's 0 and 1:
//      ./nachos -m 0 -o 1 &
//      ./nachos -m 1 -o 0 &
//
//    2. You need an implementation of condition variables,
//       which is *not* provided as part of the baseline threads 
//       implementation.  The Post Office won't work without
//       a correct implementation of condition variables.
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
//  1. send a message to the machine with ID "farAddr", at mail box #0
//  2. wait for the other machine's message to arrive (in our mailbox #0)
//  3. send an acknowledgment for the other machine's message
//  4. wait for an acknowledgement from the other machine to our 
//      original 

int dovalidatelockindex(int index, PacketHeader inPktHdr, MailHeader inMailHdr)
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
    if (inPktHdr.to <0 || inPktHdr.from <0 || inMailHdr.from < 0 || inMailHdr.to < 0 ){
        printf("Invalid. inmailhdr.to= %d, inmailhdr.from= %d, inPktHdr.to=%d, inPktHdr.from=%d\n", inMailHdr.to, inMailHdr.from, inPktHdr.to, inPktHdr.from);
        return -1;
    }

    return 0;
}
void doCreateLock(char *name, PacketHeader inPktHdr, MailHeader inMailHdr){
    std::stringstream ss;
    //char *response = new char;
    int x= slocks.size();
    ss << "CLSUCCESS " <<  x;
    //ss >> response;
    //printf("response: %s", response);
    PacketHeader outPktHdr;
    MailHeader outMailHdr;

    outPktHdr.to = inPktHdr.from; 
    outPktHdr.from= inPktHdr.to;  
    outMailHdr.to = inMailHdr.from;
    outMailHdr.from = 0;

    outMailHdr.length = ss.str().length() + 1;
    //printf("Response: %s", response);
    char *response = const_cast<char*>( ss.str().c_str());
    Mail *m = new Mail(outPktHdr, outMailHdr, response);

    ServerLock *sl= new ServerLock(name, inPktHdr.from, 0);
    sl->machineID= inPktHdr.from;
    sl->mailbox= inMailHdr.from;
    sl->state= 0; //free
    slocks.push_back(sl);
    printf("outmailhdr.to= %d, outmailhdr.from= %d, outPktHdr.to=%d, outPktHdr.from=%d\n", m->mailHdr.to, m->mailHdr.from, m->pktHdr.to, m->pktHdr.from);

    bool success = postOffice->Send(m->pktHdr, m->mailHdr, m->data);
    if(!success){
        printf("postOffice send failed. Terminating...\n");
        interrupt->Halt();
        }
}
void doAcquireLock(int indexlock, PacketHeader inPktHdr, MailHeader inMailHdr){
    std::stringstream ss;
    //char *response = new char;
    int x= slocks.size()-1;
    ss << "ALSUCCESS " <<  x;
    //ss >> response;
    //printf("response: %s", response);
    PacketHeader outPktHdr;
    MailHeader outMailHdr;

    outPktHdr.to = inPktHdr.from; 
    outPktHdr.from= inPktHdr.to;  
    outMailHdr.to = inMailHdr.from;
    outMailHdr.from = 0;

    outMailHdr.length = ss.str().length() + 1;
    //printf("Response: %s", response);
    char *response = const_cast<char*>( ss.str().c_str());
    Mail *m = new Mail(outPktHdr, outMailHdr, response);

    if (slocks.at(indexlock)->state == 1){ //lock busy so go on wait queue
            slocks.at(indexlock)->waitqueue.push(m);
        }
    else {  // lock is valid and available so get the lock
            printf("About to acquire the lock\n");
            slocks.at(indexlock)->state = 1;
            bool success = postOffice->Send(m->pktHdr, m->mailHdr, m->data);
            if(!success){
                printf("postOffice send failed. Terminating...\n");
                interrupt->Halt();
            }
        }
}
void doReleaseLock(int indexlock, PacketHeader inPktHdr, MailHeader inMailHdr){
    std::stringstream ss;
    //char *response = new char;
    int x= slocks.size()-1;
    ss << "RLSUCCESS " <<  x;
    //ss >> response;
    //printf("response: %s", response);
    PacketHeader outPktHdr;
    MailHeader outMailHdr;

    outPktHdr.to = inPktHdr.from; 
    outPktHdr.from= inPktHdr.to;  
    outMailHdr.to = inMailHdr.from;
    outMailHdr.from = 0;

    outMailHdr.length = ss.str().length() + 1;
    //printf("Response: %s", response);
    char *response = const_cast<char*>( ss.str().c_str());

    printf("outmailhdr.to= %d, outmailhdr.from= %d, outPktHdr.to=%d, outPktHdr.from=%d\n", outMailHdr.to, outMailHdr.from, outPktHdr.to, outPktHdr.from);
    outMailHdr.length = strlen(response) + 1;
    printf("Response: %s", response);
    Mail *m = new Mail(outPktHdr, outMailHdr, response);

    if (!slocks.at(indexlock)->waitqueue.empty()){ // 
            Mail *temp = slocks.at(indexlock)->waitqueue.front();
            bool success = postOffice->Send(temp->pktHdr, temp->mailHdr, temp->data);
            slocks.at(indexlock)->waitqueue.pop();
    }
    else{
        slocks.at(indexlock)->state = 0;
        }
        bool success = postOffice->Send(m->pktHdr, m->mailHdr, m->data);
        if(!success){
            printf("postOffice send failed. Terminating...\n");
            interrupt->Halt();
    }
}
void doDestroyLock(int indexlock, PacketHeader inPktHdr, MailHeader inMailHdr){
    char * response= "DLSUCCESS\n";
    PacketHeader outPktHdr;
    MailHeader outMailHdr;

    outPktHdr.to = inPktHdr.from; 
    outPktHdr.from= inPktHdr.to;  
    outMailHdr.to = inMailHdr.from;
    outMailHdr.from = 0;

    printf("outmailhdr.to= %d, outmailhdr.from= %d, outPktHdr.to=%d, outPktHdr.from=%d\n", outMailHdr.to, outMailHdr.from, outPktHdr.to, outPktHdr.from);
    outMailHdr.length = strlen(response) + 1;
    printf("Response: %s", response);
    Mail *m = new Mail(outPktHdr, outMailHdr, response);

    ServerLock * sl = slocks.at(indexlock);

        if(sl->state == 0 &&  sl->waitqueue.empty()) // if lock is free and nobody is waiting
         {
            delete sl;
            bool success = postOffice->Send(m->pktHdr, m->mailHdr, m->data);
            if(!success){
                printf("postOffice send failed. Terminating...\n");
                interrupt->Halt();
            }
        }
        else{
            sl->toDelete=true;
        }

}
int dovalidatecvindeces(int indexcv, int indexlock, PacketHeader inPktHdr, MailHeader inMailHdr)
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
void doCreateCV(char* name, PacketHeader inPktHdr, MailHeader inMailHdr){

    char * response= "CCSUCCESS 0\n";
    PacketHeader outPktHdr;
    MailHeader outMailHdr;

    outPktHdr.to = inPktHdr.from;   
    outMailHdr.to = 0;
    outMailHdr.from = 0;

    outMailHdr.length = strlen(response) + 1;
    printf("Response: %s", response);
    Mail *m = new Mail(outPktHdr, outMailHdr, response);

    ServerCV *sc= new ServerCV(name, inPktHdr.from, 0);
    sc->mailbox= inMailHdr.from;
    sc->state= 0; //free
    sconditions.push_back(sc);
    printf("outmailhdr.to= %d, outmailhdr.from= %d, outPktHdr.to=%d, outPktHdr.from=%d\n", m->mailHdr.to, m->mailHdr.from, m->pktHdr.to, m->pktHdr.from);

    bool success = postOffice->Send(m->pktHdr, m->mailHdr, m->data);
    if(!success){
        printf("postOffice send failed. Terminating...\n");
        interrupt->Halt();
        }
}
void doWaitCV(int indexlock, int indexcv, PacketHeader inPktHdr, MailHeader inMailHdr){

}
void doSignalCV(int indexlock, int indexcv, PacketHeader inPktHdr, MailHeader inMailHdr){

}
void doBroadcastCV(int indexlock, int indexcv, PacketHeader inPktHdr, MailHeader inMailHdr){

}
void doDestroyCV(int indexlock, int indexcv, PacketHeader inPktHdr, MailHeader inMailHdr){

}
void Server(){
    
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;

    char buffer[MaxMailSize];
    char *lockname= new char;
    char *cvname= new char;
    char* type= new char;
    char* response= new char;

    while (true){
        stringstream ss;
        printf("Server is started. waiting for message.\n");
        postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
        printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout);

        ss.str(buffer);
        

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
                printf("Server received Create Lock request from client.\n");
                ss >> lockname;
                ss >> length;
                if (length <= 0 || length > 24)
                {
                    printf("%s","Length for locks's identifier is invalid.\n");
                    ss << "CL_FAILURE " <<  "-1";

                    outPktHdr.to = inPktHdr.from; 
                    outPktHdr.from= inPktHdr.to;  
                    outMailHdr.to = inMailHdr.from;
                    outMailHdr.from = 0;

                    outMailHdr.length = ss.str().length() + 1;
                    //printf("Response: %s", response);
                    response = const_cast<char*>( ss.str().c_str());
                    success = postOffice->Send(outPktHdr, outMailHdr, response);
                    break;
                }
                else{
                    printf("inmailhdr.to= %d, inmailhdr.from= %d, inPktHdr.to=%d, inPktHdr.from=%d\n", inMailHdr.to, inMailHdr.from, inPktHdr.to, inPktHdr.from);
                    doCreateLock(lockname, inPktHdr, inMailHdr);
                    printf("After creating the lock\n");
                }
            }
        else if (strcmp(type, "AL") == 0) //need to pass index
             {
                printf("Server received Acquire Lock request from client.\n");
                ss >> lockindex;
                int size = slocks.size();
                if (dovalidatelockindex(lockindex, inPktHdr, inMailHdr)==-1){
                    printf("%s","invalid index or wrong machine.\n");
                    ss << "AL_FAILURE " <<  "-1";

                    outPktHdr.to = inPktHdr.from; 
                    outPktHdr.from= inPktHdr.to;  
                    outMailHdr.to = inMailHdr.from;
                    outMailHdr.from = 0;

                    outMailHdr.length = ss.str().length() + 1;
                    response = const_cast<char*>( ss.str().c_str());
                    success = postOffice->Send(outPktHdr, outMailHdr, response);
                    break;
                }
                else{
                    doAcquireLock(lockindex, inPktHdr, inMailHdr);
                    printf("After acquiring the lock\n");
                }
            }
        else if (strcmp(type, "RL") == 0) //need to pass index
             {
                printf("Server received Release Lock request from client.\n");
                ss >> lockindex;
                int size = slocks.size();
                if (dovalidatelockindex(lockindex, inPktHdr, inMailHdr)==-1){
                    printf("%s","invalid index or wrong machine.\n");
                    ss << "RL_FAILURE " <<  "-1";
                    outPktHdr.to = inPktHdr.from; 
                    outPktHdr.from= inPktHdr.to;  
                    outMailHdr.to = inMailHdr.from;
                    outMailHdr.from = 0;

                    outMailHdr.length = ss.str().length() + 1;
                    response = const_cast<char*>(ss.str().c_str());
                    success = postOffice->Send(outPktHdr, outMailHdr, response);
                    break;
                }
                else
                {
                    doReleaseLock(lockindex, inPktHdr, inMailHdr);
                    printf("After releasing the lock\n");
                }
            }
        else if (strcmp(type, "DL") == 0)
             {
                printf("Server received Destroy Lock request from client.\n");
                ss >> lockindex;
                int size = slocks.size();
                if (dovalidatelockindex(lockindex, inPktHdr, inMailHdr)==-1){
                    printf("%s","invalid index or wrong machine.\n");
                    ss << "DL_FAILURE " <<  "-1";
                    outPktHdr.to = inPktHdr.from; 
                    outPktHdr.from= inPktHdr.to;  
                    outMailHdr.to = inMailHdr.from;
                    outMailHdr.from = 0;
                    outMailHdr.length = ss.str().length() + 1;
                    response = const_cast<char*>( ss.str().c_str());
                    success = postOffice->Send(outPktHdr, outMailHdr, response);
                    break;
                }
                else{
                //printf("lockname is: %d", lockname);
                doDestroyLock(lockindex, inPktHdr, inMailHdr);
                printf("the lock at specified index has been destroyed\n");
        
                }
            }
        else if (strcmp(type, "CC") == 0)
             {
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
                doCreateCV(lockname, inPktHdr, inMailHdr);
                printf("After creating the cv\n");
                }
            }
        else if (strcmp(type, "WC") == 0)
             {
                if (dovalidatecvindeces(lockindex, cvindex, inPktHdr, inMailHdr)==-1){
                    printf("%s","Wait condition: incorrect index\n");
                    response="-1";
                    //bigServerCV->Release();
                    break;
                }
                printf("Server received Wait Condition request from client.\n");
                ss >> lockindex;
                ss >>cvindex;
                doWaitCV(lockindex, cvindex, inPktHdr, inMailHdr);
                
            }
        else if (strcmp(type, "SC") == 0)
             {
                if (dovalidatecvindeces(lockindex, cvindex, inPktHdr, inMailHdr)==-1){
                    printf("%s","Wait condition: incorrect index\n");
                    response="-1";
                    //bigServerCV->Release();
                    break;
                }

                printf("Server received Signal Condition request from client.\n");
                ss >> lockindex;
                ss >>cvindex;
                doSignalCV(lockindex, cvindex, inPktHdr, inMailHdr);
            }
        else if (strcmp(type, "BC") == 0)
             {
                if (dovalidatecvindeces(lockindex, cvindex, inPktHdr, inMailHdr)==-1){
                    printf("%s","Wait condition: incorrect index\n");
                    response="-1";
                    //bigServerCV->Release();
                    break;
                }
                 printf("Server received Broadcast Condition request from client.\n");
                 ss >> lockindex;
                ss >>cvindex;
                doBroadcastCV(lockindex, cvindex, inPktHdr, inMailHdr);
        
            }
        else if (strcmp(type, "DC") == 0)
             {
                if (dovalidatecvindeces(lockindex, cvindex, inPktHdr, inMailHdr)==-1){
                    printf("%s","Wait condition: incorrect index\n");
                    response="-1";
                    //bigServerCV->Release();
                    break;
                }
                printf("Server received Destroy Condition request from client.\n");
                ss >> lockindex;
                ss >>cvindex;
                doDestroyCV(lockindex, cvindex, inPktHdr, inMailHdr);
        
            }
        else{
            
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