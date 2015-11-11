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
using namespace std;

struct ServerCV
{
    string name;
    bool toDelete;
    int conditionlock;
    std::queue<Mail*> waitqueue;

    ServerCV(string cvname)
    {
        name = cvname;
        toDelete = false;
    }
};

vector<ServerCV*> serverconditions;


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

void SendResponse(string response, int machineID, int mailboxID)
{
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;

    outPktHdr.to = machineID; // Server Machine ID
    outMailHdr.to = mailboxID; // Server Machine ID
    outMailHdr.from = 0; // Client Mailbox ID

    char *message = (char *) response.c_str();
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

int validatecvindeces(unsigned int indexcv, unsigned int indexlock, int machineID, int mailboxID)
{
    // Create the message
    std::stringstream ss;
    bool error = false;

    // Validate input
    if (indexcv >= serverconditions.size() || indexcv < 0)
    {
        ss << "400" << " " << "WC" << " " << indexcv;
        error = true;
    }

    if (indexlock >= slocks.size() || indexlock < 0)
    {
        ss << "400" << " " << "WC" << " " << indexcv;
        error = true;
    }

    // Get the Condition and Wait
    ServerCV *condition = serverconditions.at(indexcv);
    
    if (machineID != slocks.at(condition->conditionlock)->machineID)
    {
        ss << "400" << " " << "WC" << " " << indexcv;
        error = true;
    }

    if (error)
    {
        string response = ss.str();
        SendResponse(response, machineID, mailboxID);
        return -1;
    }

    // No errors
    return 0;
}


void CreateCV(string cvname, int machineID, int mailboxID)
{
    // Create the message
    std::stringstream ss;

    // Create Condition
    ServerCV *condition = new ServerCV(cvname);
    serverconditions.push_back(condition);

    // Create Response and Send
    ss << "201" << " " << "CC" << " " << serverconditions.size() - 1;
    string response = ss.str();
    SendResponse(response, machineID, mailboxID);
}

void WaitCV(unsigned int indexcv, unsigned int indexlock, int machineID, int mailboxID)
{
    // Create the message
    std::stringstream ss;

    // Validate input
    if (validatecvindeces(indexcv, indexlock, machineID, mailboxID) == -1)
    {
        return;
    }
    else
    {
        ss << "200" << " " << "WC" << " " << indexcv;
    }

    // Get the Condition and Wait
    ServerCV *condition = serverconditions.at(indexcv);

    // Prepare Message to send when Signal is called
    PacketHeader outPktHdr;
    MailHeader outMailHdr;

    outPktHdr.to = machineID; // Server Machine ID
    outMailHdr.to = mailboxID; // Server Machine ID
    outMailHdr.from = 0; // Client Mailbox ID
    char *response = (char *) ss.str().c_str();
    outMailHdr.length = strlen(response) + 1;

    Mail *queuedMessage = new Mail(outPktHdr, outMailHdr, response);

    condition->waitqueue.push(queuedMessage);
}

void DeleteCondition(unsigned int indexcv)
{
    ServerCV *condition = serverconditions.at(indexcv);

    delete condition;
    conditions.at(indexcv) = NULL;

    DEBUG('n', "Condition %d was successfully deleted.\n", indexcv);
}

void SignalCV(unsigned int indexcv, unsigned int indexlock, int machineID, int mailboxID)
{
    // Create the message
    std::stringstream ss;

    // Validate input
    if (validatecvindeces(indexcv, indexlock, machineID, mailboxID) == -1)
    {
        return;
    }

    // Get the Condition and Wait
    ServerCV *condition = serverconditions.at(indexcv);

    // "Wake up" One Sleeping Machine
    Mail *response = condition->waitqueue.front();
    condition->waitqueue.pop();

    // Send request
    bool success = postOffice->Send(response->pktHdr, response->mailHdr, response->data);
    if (!success)
    {
        printf("The PostOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
        interrupt->Halt();
    }

    // Marked for deletion and no waiting threads, delete
    if (condition->toDelete && condition->waitqueue.empty())
    {
        DeleteCondition(indexcv);
    }

    // Send response to Signaling machine
    ss << "200" << " " << "SC" << " " << indexcv;
    string signalResponse = ss.str();
    SendResponse(signalResponse, machineID, mailboxID);
}
void BroadcastCV(unsigned int indexcv, unsigned int indexlock, int machineID, int mailboxID)
{
    // Create the message
    std::stringstream ss;

    // Validate input
    if (validatecvindeces(indexcv, indexlock, machineID, mailboxID) == -1)
    {
        return;
    }

    // Get the Condition and Wait
    ServerCV *condition = serverconditions.at(indexcv);

    // "Wake up" ALL Sleeping Machines
    while (!condition->waitqueue.empty())
    {
        Mail *response = condition->waitqueue.front();
        condition->waitqueue.pop();

        // Send response
        bool success = postOffice->Send(response->pktHdr, response->mailHdr, response->data);
        if (!success)
        {
            printf("The PostOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
            interrupt->Halt();
        }
    }

    // Marked for deletion and no waiting threads, delete
    if (condition->toDelete && condition->waitqueue.empty())
    {
        DeleteCondition(indexcv);
    }

    // Send response to Signaling machine
    ss << "200" << " " << "BC" << " " << serverconditions.size() - 1;
    string signalResponse = ss.str();
    SendResponse(signalResponse, machineID, mailboxID);
}
void DestroyCV(unsigned int indexcv, int machineID, int mailboxID)
{
    // Create the message
    std::stringstream ss;
    bool error = false;

    // Validate input
    if (indexcv >= serverconditions.size() || indexcv < 0)
    {
        ss << "400" << " " << "DC" << " " << indexcv;
        error = true;
    }

    // Get the Condition and Wait
    ServerCV *condition = serverconditions.at(indexcv);
    
    if (machineID != slocks.at(condition->conditionlock)->machineID)
    {
        ss << "400" << " " << "DC" << " " << indexcv;
        error = true;
    }

    // Valid CV index, delete or mark for deletion
    if (!error)
    {
        // No waiting threads when Destroy called, delete
        if (condition->waitqueue.empty())
        {
            DeleteCondition(indexcv);
        }

        ss << "200" << " " << "DC" << " " << indexcv;
        condition->toDelete = true;
    }

    // Send Response
    string response = ss.str();
    SendResponse(response, machineID, mailboxID);
}

void CreateMV(string mvname, int mvsize, int machineID, int mailboxID)
{
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

    string response = ss.str();
    SendResponse(response, machineID, mailboxID);
}

void SetMV(unsigned int indexmv, unsigned int indexvar, int value, int machineID, int mailboxID)
{
    // Create the message
    std::stringstream ss;

    // Validate input
    if (indexmv >= monitorvariables.size())
    {
        ss << "400 SM" << " " << indexmv;
    }

    MonitorVariable *mv = monitorvariables.at(indexmv);

    if (indexvar >= mv->size)
    {
        ss << "400 SM" << " " << indexmv;
    }
    else if (indexmv < monitorvariables.size() && indexvar < mv->size)
    {
        mv->values[indexvar] = value;
        ss << "200 SM" << " " << indexmv;
    }

    string response = ss.str();
    SendResponse(response, machineID, mailboxID);
}

void GetMV(unsigned int indexmv, unsigned int indexvar, int machineID, int mailboxID)
{
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

    string response = ss.str();
    SendResponse(response, machineID, mailboxID);
}


void DestroyMV(unsigned int indexmv, int machineID, int mailboxID)
{
    // Create the message
    std::stringstream ss;

    // Validate input
    if (indexmv >= monitorvariables.size())
    {
        ss << "400" << " " << "GM" << " " << indexmv;
    }
    // Destroy MV
    else
    {
        MonitorVariable *mv = monitorvariables.at(indexmv);
        delete mv->values;
        monitorvariables.at(indexmv) = NULL;
        ss << "200" << " " << "DM" << " " << indexmv;
    }

    // Send Request
    string response = ss.str();
    SendResponse(response, machineID, mailboxID);
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

    char buffer[MaxMailSize];
    char *lockname= new char;
    char *cvname= new char;
    char* type= new char;
    char* response= new char;
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
        char *response = "0";
        int length, lockindex, cvindex;
        bool success;
        ss >> requestType;

        DEBUG('n', "Type of RPC: %s\n", requestType);

       if (strcmp(requestType, "CL") == 0)
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
        else if (strcmp(requestType, "AL") == 0) //need to pass index
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
        else if (strcmp(requestType, "RL") == 0) //need to pass index
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
        else if (strcmp(requestType, "DL") == 0)
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
                else
                {
                //printf("lockname is: %d", lockname);
                doDestroyLock(lockindex, inPktHdr, inMailHdr);
                printf("the lock at specified index has been destroyed\n");
        
                }
            }
        else if (strcmp(requestType, "CC") == 0)
        {
            string cvname;
            ss >> cvname;

            CreateCV(lockname, inPktHdr.from, inMailHdr.from);
        }
        else if (strcmp(requestType, "WC") == 0)
        {
            unsigned int indexcv, indexlock;
            ss >> indexcv >> indexlock;
            
            WaitCV(indexcv, indexlock, inPktHdr.from, inMailHdr.from);
        }
        else if (strcmp(requestType, "SC") == 0)
        {
            unsigned int indexcv, indexlock;
            ss >> indexcv >> indexlock;
            
            SignalCV(indexcv, indexlock, inPktHdr.from, inMailHdr.from);
        }
        else if (strcmp(requestType, "BC") == 0)
        {
            unsigned int indexcv, indexlock;
            ss >> indexcv >> indexlock;
            
            BroadcastCV(indexcv, indexlock, inPktHdr.from, inMailHdr.from);
        }
        else if (strcmp(requestType, "DC") == 0)
        {
            unsigned int indexcv;
            ss >> indexcv;

            DestroyCV(indexcv, inPktHdr.from, inMailHdr.from);
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
        else if (strcmp(requestType, "DM") == 0)
        {
            unsigned int indexmv;
            ss >> indexmv;

            DestroyMV(indexmv, inPktHdr.from, inMailHdr.from);
        }
    }
}




//========================================================================================================================================
//
// MailTest
//  Tests simple message passing between a Client and Server Machine
//
//========================================================================================================================================

//----------------------------------------------------------------------
// Test out message delivery, by doing the following:
//  (1) send a message to the machine with ID "farAddr", at mail box #0
//  (2) wait for the other machine's message to arrive (in our mailbox #0)
//  (3) send an acknowledgment for the other machine's message
//  (4) wait for an acknowledgement from the other machine to our 
//      original 
//----------------------------------------------------------------------


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
