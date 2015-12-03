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

void SendResponse(string response, int machineID, int mailboxID);

struct PendingRequest
{
    int machineID;
    int mailboxID;
    string requestType;

    int indexcv;
    int indexlock;
    int indexmv;
    int indexvar;
    int mvvalue;
    string name;

    bool fulfilled;
    int noCount;

    PendingRequest(string type, int machine, int mailbox)
    {
        requestType = type;
        machineID = machine;
        mailboxID = mailbox;
        fulfilled = false;
        noCount = 0;
    }
};

vector<PendingRequest*> pending = new vector<PendingRequest>();


//========================================================================================================================================
//
// Locks
//  A condition variable does not have a value, but threads may be queued, 
//  waiting on the variable. All operations on a condition variable must 
//  be made while the requesting machine has acquired a lock. Mutual exclusion
//  must be enforced among machines calling the condition variable operations.
//
//========================================================================================================================================

//----------------------------------------------------------------------
// ServerLock
//  Server locks differ from traditional locks in that we do not have a
//  notion of threads, since execution of user programs is done remotely
//  Instead, we hold lock ownership with machineID and mailbox 
//  (corresponding to IP Addresses and Port Numbers), and hold a queue
//  of Messages to put a remote user program to sleep, since programs
//  pause execution until receiving a response.
//----------------------------------------------------------------------

enum lockstate { FREE, BUSY };

struct ServerLock
{
    string name;
    lockstate state;
    int machineID;
    int mailbox;
    bool toDelete;

    ServerLock(string lockname, int netname, int box)
    {
        name = lockname;
        machineID = netname;
        mailbox = box;
        state = FREE;
        toDelete = false;
    }
};

ServerLock* serverlocks[100];

BitMap LockMap(100);

//----------------------------------------------------------------------
// ValidateLockIndex
//  Verifies the input passed in to Lock RPCs (1) correspond to valid
//  lock locations (positive and inbounds) and (2) the lock exists. If 
//  any errors occur, the Server sends a corresponding error code and 
//  the RPC should not continue.
//
//  Returns -1 if an error occured, 0 otherwise.
//
//  "indexlock" -- the index passed in to the Lock RPC being validated
//  "requestType" -- the RPC two-letter code
//  "machineID" -- the requesting machine's netname, used for responses
//  "mailboxID" -- the requesting mailbox number, used for responses
//----------------------------------------------------------------------

int ValidateLockIndex(int indexlock, int machineID, int mailboxID)
{
    // Create the message
    std::stringstream ss;
    bool error = false;

    // (1) Index corresponds to valid location: 400
    if (indexlock >= 100 || indexlock < 0)
    {
        ss << "400" << " " << requestType << " " << indexlock;
        error = true;
    }

    ServerLock *lock = serverlocks[indexlock];

    // (2) Lock not found: 404
    if (!lock)
    {
        ss << "404" << " " << requestType << " " << indexlock;
        error = true;
    }

    // Errors: Send response and wrap up RPC
    if (error)
    {
        string response = ss.str();
        SendResponse(response, machineID, mailboxID);
        return -1;
    }

    // No errors
    return 0;
}

//----------------------------------------------------------------------
// CreateLock
//  TODO: Description
//----------------------------------------------------------------------

void CreateLock(char *name, int machineID, int mailboxID)
{
    std::stringstream ss;

    ServerLock * sl= new ServerLock(name, machineID, mailboxID);
    sl->machineID = machineID;
    sl->mailbox = mailboxID;
    sl->state = 0; // free

    int indexlock = LockMap.find();
    serverlocks[indexlock] = sl;

    ss << "CLSUCCESS " <<  indexlock;
    string response = ss.str();
    SendResponse(response, machineID, mailboxID);
}

//----------------------------------------------------------------------
// AcquireLock
//  TODO: Description
//----------------------------------------------------------------------

void AcquireLock(int indexlock, int machineID, int mailboxID)
{
    if (ValidateLockIndex(lockindex, machineID, mailboxID) == -1)
    {
        return;
    }

    std::stringstream ss;
    ss << "ALSUCCESS " << indexlock;

    if (serverlocks[indexlock]->state == 1)
    { 
        // lock busy -- go on wait queue
        serverlocks[indexlock]->waitqueue.push(m);
    }
    else 
    {  // lock valid and available -- acquire the lock
        printf("About to acquire the lock\n");
        serverlocks[indexlock]->state = 1;
        serverlocks[indexlock]->machineID = machineID;
        serverlocks[indexlock]->mailbox = mailboxID;
        string response = ss.str();
        SendResponse(response, machineID, mailboxID);
    }
}

//----------------------------------------------------------------------
// ReleaseLock
//  TODO: Description
//----------------------------------------------------------------------

void ReleaseLock(int indexlock, int machineID, int mailboxID)
{
    if (ValidateLockIndex(lockindex, machineID, mailboxID) == -1)
    {
        return;
    }

    std::stringstream ss;
    char *response;

    if (serverlocks[indexlock]->machineID != machineID || serverlocks[indexlock]->mailbox != mailboxID)
    {
        ss << "404" << " RL " << indexlock;
    }
    else
    {
        ss << "RLSUCCESS " <<  indexlock;

        if (!serverlocks[indexlock]->waitqueue.empty())
        {
            response = const_cast<char*>( ss.str().c_str());
            
            Mail *temp = serverlocks[indexlock]->waitqueue.front();
            bool success = postOffice->Send(temp->pktHdr, temp->mailHdr, temp->data);
            serverlocks[indexlock]->waitqueue.pop();

            serverlocks[indexlock]->machineID = machineID;
            serverlocks[indexlock]->mailbox = mailboxID;
        }
        else
        {
            serverlocks[indexlock]->state = 0;

            if (serverlocks[indexlock]->toDelete)
            {
                delete serverlocks[indexlock];
                LockMap.clear(indexlock);
                serverlocks[indexlock] = NULL;
            }
        }
    }
        
    response = const_cast<char*>(ss.str().c_str());

    SendResponse(response, machineID, mailboxID);
}

//----------------------------------------------------------------------
// DestroyLock
//  TODO: Description
//----------------------------------------------------------------------

void DestroyLock(int indexlock, int machineID, int mailboxID)
{
    if (ValidateLockIndex(lockindex, machineID, mailboxID) == -1)
    {
        return;
    }

    char * response= "DLSUCCESS\n";

    ServerLock * sl = serverlocks[indexlock];

    // if lock is free and nobody is waiting
    if(sl->state == 0 &&  sl->waitqueue.empty()) 
    {
        delete sl;
        LockMap.clear(indexlock);
        serverlocks[indexlock] = NULL;

        SendResponse(response, machineID, mailboxID);
    }
    else
    {
        sl->toDelete=true;
    }

}

//========================================================================================================================================
//
// Condition Variables
//  A condition variable does not have a value, but threads may be queued, 
//  waiting on the variable. All operations on a condition variable must 
//  be made while the requesting machine has acquired a lock. Mutual exclusion
//  must be enforced among machines calling the condition variable operations.
//
//========================================================================================================================================

struct ServerCV
{
    string name;
    bool toDelete;
    int conditionlock;
    bool cvlockSet;
    std::queue<Mail*> waitqueue;

    ServerCV(string cvname)
    {
        name = cvname;
        toDelete = false;
        cvlockSet = false;
    }
};

ServerCV* serverconditions[100];

BitMap CVMap(100);

//----------------------------------------------------------------------
// ValidateCVIndeces
//  Verifies the input passed in to CV RPCs (1) correspond to valid CV,
//  (2) the CV exists, (3) the lock is valid and exists, and (4) the 
//  machine owns the Lock corresponding to the CV's critical section. If 
//  any errors occur, the Server sends a corresponding error code and 
//  the RPC should not continue.
//
//  Returns -1 if an error occured, 0 otherwise.
//
//  "indexcv" -- CV index passed in to the CV RPC being validated
//  "indexlock" -- Lock index passed in to the CV RPC being validated
//  "requestType" -- the RPC two-letter code
//  "machineID" -- the requesting machine's netname, used for responses
//  "mailboxID" -- the requesting mailbox number, used for responses
//----------------------------------------------------------------------

int ValidateCVIndeces(unsigned int indexcv, unsigned int indexlock, string requestType, int machineID, int mailboxID)
{
    // Create the message
    std::stringstream ss;
    bool error = false;

    // (1) Corresponds to valid condition location: 400
    if (indexcv >= 100 || indexcv < 0)
    {
        ss << "400" << " " << requestType << " " << indexcv;
        error = true;
    }

    // Get the Condition and Wait
    ServerCV *condition = serverconditions[indexcv];

    // (2) Condition exists: 404
    if (!condition)
    {
        ss << "404" << " " << requestType << " " << indexcv;
        error = true;
    }
    else
    {
        // If lock passed in belongs to CV's C.S.
        if (condition->cvlockSet && condition->conditionlock != indexlock)
        {
            ss << "407" << " " << requestType << " " << indexcv;
            error = true;  
        }
    
        // If CV has no corresponding lock, set its lock
        if (!condition->cvlockSet)
        {
            condition->conditionlock = indexlock;
            condition->cvlockSet = true;
        }
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

//----------------------------------------------------------------------
// DeleteCondition
//  TODO: Description
//----------------------------------------------------------------------

void DeleteCondition(unsigned int indexcv)
{
    ServerCV *condition = serverconditions[indexcv];

    delete condition;

    serverconditions[indexcv] = NULL;
    CVMap.clear(indexcv);

    DEBUG('n', "Condition %d was successfully deleted.\n", indexcv);
}

//----------------------------------------------------------------------
// CreateCV
//  TODO: Description
//----------------------------------------------------------------------

void CreateCV(string cvname, int machineID, int mailboxID)
{
    // Create the message
    std::stringstream ss;

    // Create Condition
    ServerCV *condition = new ServerCV(cvname);
    int indexcv = CVMap.find();
    serverconditions[indexcv] = condition;

    // Create Response and Send
    ss << "201" << " " << "CC" << " " << indexcv;
    string response = ss.str();
    SendResponse(response, machineID, mailboxID);
}

//----------------------------------------------------------------------
// WaitCV
//  TODO: Description
//----------------------------------------------------------------------

void WaitCV(unsigned int indexcv, unsigned int indexlock, int machineID, int mailboxID)
{
    // Create the message
    std::stringstream ss;

    // Validate input
    if (ValidateCVIndeces(indexcv, indexlock, machineID, mailboxID) == -1)
    {
        return;
    }
    else
    {
        ss << "200" << " " << "WC" << " " << indexcv;
    }

    // Get the Condition and Wait
    ServerCV *condition = serverconditions[indexcv];

    // Prepare Message to send when Signal is called
    PacketHeader outPktHdr;
    MailHeader outMailHdr;

    outPktHdr.to = machineID; // Server Machine ID
    outMailHdr.to = mailboxID; // Server Machine ID
    outMailHdr.from = 0; // Client Mailbox ID
    char *response = (char *) ss.str().c_str();
    outMailHdr.length = strlen(response) + 1;

    Mail *queuedMessage = new Mail(outPktHdr, outMailHdr, response);

    // Release the Lock before going to sleep: 
    //  Wake up one thread waiting for lock
    // ^ ReleaseLock should be own method
    if (!serverlocks[indexlock]->waitqueue.empty())
    { 
        Mail *waitingLock = serverlocks[indexlock]->waitqueue.front();
        serverlocks[indexlock]->waitqueue.pop();

        serverlocks[indexlock]->state = 1; // Set Lock to Busy
        serverlocks[indexlock]->machineID = waitingLock->pktHdr.to;
        serverlocks[indexlock]->mailbox = waitingLock->mailHdr.to;

        bool success = postOffice->Send(waitingLock->pktHdr, waitingLock->mailHdr, waitingLock->data);
        if(!success)
        {
            printf("postOffice send failed. Terminating...\n");
            interrupt->Halt();
        }
    }
    else
    {
        serverlocks[indexlock]->state = 0; // Set Lock to Free
    }

    condition->waitqueue.push(queuedMessage);
}

//----------------------------------------------------------------------
// SignalCV
//  TODO: Description
//----------------------------------------------------------------------

void SignalCV(unsigned int indexcv, unsigned int indexlock, int machineID, int mailboxID)
{
    // Create the message
    std::stringstream ss;

    // Validate input
    if (ValidateCVIndeces(indexcv, indexlock, machineID, mailboxID) == -1)
    {
        return;
    }

    // Get the Condition and Signal
    ServerCV *condition = serverconditions[indexcv];

    // "Wake up" One Sleeping Machine
    Mail *response = condition->waitqueue.front();
    condition->waitqueue.pop();

    // Need to Forward Message to machine with Lock
    ForwardMessage(task = "AL", indexlock, response); // Client Machine ID, Mailbox ID, Request Code/Status 

        {
                // Add Client Machine ID/Mailbox ID to request
                ss << task; // Task to Perform
                ss << response.outPktHdr.to; // Client Machine ID
                ss << response.outMailHdr.to; // Client Mailbox ID 
                ss << response.data; // TODO: How do you load a char[] into a ss? <-- Response to Original Request (i.e. asking to AcquireLock, but originally called Wait)

                // Put in Resource Mailbox
                PacketHeader outPktHdr;
                MailHeader outMailHdr;

                outPktHdr.to = ;// Server Machine ID
                outMailHdr.to = 1; // Put in Resource Mailbox
                outMailHdr.from = 0;
                return;
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

//----------------------------------------------------------------------
// BroadcastCV
//  TODO: Description
//----------------------------------------------------------------------

void BroadcastCV(unsigned int indexcv, unsigned int indexlock, int machineID, int mailboxID)
{
    // Create the message
    std::stringstream ss;

    // Validate input
    if (ValidateCVIndeces(indexcv, indexlock, machineID, mailboxID) == -1)
    {
        return;
    }

    // Get the Condition and Wait
    ServerCV *condition = serverconditions[indexcv];

    // "Wake up" ALL Sleeping Machines
    while (!condition->waitqueue.empty())
    {
        Mail *response = condition->waitqueue.front();
        condition->waitqueue.pop();

        // Reaquire the Lock when you are woken back up
        // Lock is busy so go on wait queue
        if (serverlocks[indexlock]->state == 1)
        {
            serverlocks[indexlock]->waitqueue.push(response);
        }
        // Lock is valid and available so get the lock
        else
        {
            printf("About to acquire the lock\n");
            serverlocks[indexlock]->state = 1; // Make Lock Busy
            serverlocks[indexlock]->machineID = response->pktHdr.to;
            serverlocks[indexlock]->mailbox = response->mailHdr.to;
            // Send response
            bool success = postOffice->Send(response->pktHdr, response->mailHdr, response->data);
            if (!success)
            {
                printf("The PostOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
                interrupt->Halt();
            }
        }
    }

    // Marked for deletion and no waiting threads, delete
    if (condition->toDelete && condition->waitqueue.empty())
    {
        DeleteCondition(indexcv);
    }

    // Send response to Signaling machine
    ss << "200" << " " << "BC" << " " << "100";
    string signalResponse = ss.str();
    SendResponse(signalResponse, machineID, mailboxID);
}

//----------------------------------------------------------------------
// DestroyCV
//  TODO: Description
//----------------------------------------------------------------------

void DestroyCV(unsigned int indexcv, int machineID, int mailboxID)
{
    // Create the message
    std::stringstream ss;
    bool error = false;

    // Validate input
    if (indexcv >= 100 || indexcv < 0)
    {
        ss << "400" << " " << "DC" << " " << indexcv;
        error = true;
    }

    // Get the Condition and Wait
    ServerCV *condition = serverconditions[indexcv];
    
    // if (machineID != serverlocks.at(condition->conditionlock)->machineID)
    // {
    //     ss << "400" << " " << "DC" << " " << indexcv;
    //     error = true;
    // }

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

//========================================================================================================================================
//
// Monitor Variables
//  Monitor Variables are shared data used by remote user programs.
//
//========================================================================================================================================

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

MonitorVariable* monitorvariables[100];

BitMap MVMap(100);

//----------------------------------------------------------------------
// CreateMV
//  TODO: Description
//----------------------------------------------------------------------

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
        int indexmv = MVMap.find();
        monitorvariables[indexmv] = mv;
        ss << "201 CM" << " " << indexmv;
    }

    string response = ss.str();
    SendResponse(response, machineID, mailboxID);
}

//----------------------------------------------------------------------
// SetMV
//  TODO: Description
//----------------------------------------------------------------------

void SetMV(unsigned int indexmv, unsigned int indexvar, int value, int machineID, int mailboxID)
{
    // Create the message
    std::stringstream ss;

    // Validate input
    if (indexmv >= 100 && indexmv < 0)
    {
        ss << "400 SM" << " " << indexmv;
    }

    MonitorVariable *mv = monitorvariables[indexmv];

    if (indexvar >= mv->size)
    {
        ss << "400 SM" << " " << indexmv;
    }
    else if (indexmv < 100 && indexvar < mv->size)
    {
        mv->values[indexvar] = value;
        ss << "200 SM" << " " << indexmv;
    }

    string response = ss.str();
    SendResponse(response, machineID, mailboxID);
}

//----------------------------------------------------------------------
// GetMV
//  TODO: Description
//----------------------------------------------------------------------

void GetMV(unsigned int indexmv, unsigned int indexvar, int machineID, int mailboxID)
{
    // Create the message
    std::stringstream ss;

    // Validate input
    if (indexmv >= 100 && indexmv < 0)
    {
        ss << "400" << " " << "GM" << " " << "-1";
    }

    MonitorVariable *mv = monitorvariables[indexmv];

    if (indexvar >= mv->size)
    {
        ss << "400" << " " << "GM" << " " << "-1";
    }
    else if (indexmv < 100 && indexvar < mv->size)
    {
        ss << "200" << " " << "GM" << " " << mv->values[indexvar];
    }

    string response = ss.str();
    SendResponse(response, machineID, mailboxID);
}

//----------------------------------------------------------------------
// DestroyMV
//  TODO: Description
//----------------------------------------------------------------------

void DestroyMV(unsigned int indexmv, int machineID, int mailboxID)
{
    // Create the message
    std::stringstream ss;

    // Validate input
    if (indexmv >= 100 && indexmv < 0)
    {
        ss << "400" << " " << "GM" << " " << indexmv;
    }
    // Destroy MV
    else
    {
        MonitorVariable *mv = monitorvariables[indexmv];
        delete mv->values;
        MVMap.clear(indexmv);
        monitorvariables[indexmv] = NULL;
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
// SendResponse
//  TODO: Description
//----------------------------------------------------------------------

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

void ForwardRequest (char *request, int machineID, int mailboxID)
{
    PendingRequest pendingRequest = new PendingRequest(request, machineID, mailboxID);

    pending.push_back(pendingRequest);

    stringstream ss;
    ss << (pending.size() - 1) << " ";
    ss << machineID << " ";
    ss << mailboxID << " ";
    ss << request;

    for (int indexserver = 0; indexserver < 5; indexserver++)
    {
        if (indexserver == netname)
        {
            continue;
        }

        string response = ss.str();

        SendResponse(response, indexserver, 1);
    }
}

bool LookupResource (char *request, int machineID, int mailboxID)
{
    stringstream ss;
    ss.str(request);

    string requestType;

    ss >> requestType;

    if (requestType == "CL")
    {
        string lockname;
        ss >> lockname;

        for (int indexlock = 0; indexlock < 100; indexlock++)
        {
            if (serverlocks[indexlock])
            {
                if (serverlocks[indexlock]->name == lockname)
                {
                    // Create the message
                    std::stringstream ss;
                    ss << "200" << " " << "CL" << " " << indexlock;
                    string response = ss.str();
                    SendResponse(response, machineID, mailboxID);
                    
                    break;
                }
            }
        }

        return false;
    }

    if (requestType == "CC")
    {
        string cvname;
        ss >> cvname;

        for (int indexcv = 0; indexcv < 100; indexcv++)
        {
            if (serverconditions[indexcv])
            {
                if (serverconditions[indexcv]->name == cvname)
                {
                    // Create the message
                    std::stringstream ss;
                    ss << "200" << " " << "CC" << " " << indexcv;
                    string response = ss.str();
                    SendResponse(response, machineID, mailboxID);
                    
                    break;
                }
            }
        }

        return false;
    }

    if (requestType == "CM")
    {
        string mvname;
        ss >> mvname;

        for (int indexmv = 0; indexmv < 100; indexmv++)
        {
            if (serverlocks[indexmv])
            {
                if (serverlocks[indexmv]->name == mvname)
                {
                    // Create the message
                    std::stringstream ss;
                    ss << "200" << " " << "CL" << " " << indexmv;
                    string response = ss.str();
                    SendResponse(response, machineID, mailboxID);
                    
                    break;
                }
            }
        }

        return false;
    }

    if (requestType == "AL" || requestType == "RL" || requestType == "DL")
    {
        unsigned int indexlock;
        ss >> indexlock;

        indexlock = indexlock % (netname * 100);

        if (indexlock > 100)
        {
            return false;
        }

        if (!serverlocks[indexlock])
        {
            return false;
        }

        return true;
    }

    if (requestType == "WC" || requestType == "SC" || requestType == "BC" || requestType == "DC")
    {
        unsigned int indexcv;
        ss >> indexcv;

        indexcv = indexcv % (netname * 100);

        if (indexcv > 100)
        {
            return false;
        }

        if (!serverconditions[indexcv])
        {
            return false;
        }

        return true;
    }

    if (requestType == "GM" || requestType == "SM" || requestType == "DM")
    {
        unsigned int indexmv;
        ss >> indexmv;

        indexmv = indexmv % (netname * 100);

        if (indexmv > 100)
        {
            return false;
        }

        if (!monitorvariables[indexmv])
        {
            return false;
        }

        return true;
    }
} 

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

void ClientToServer()
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

    for (int i = 0; i < 100; i++)
    {
        serverlocks[i] = NULL;
        serverconditions[i] = NULL;
        monitorvariables[i] = NULL;
    }

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

        if (!LookupResource(request, inPktHdr.from, inMailHdr.from))
        {
            ForwardRequest(request, inPktHdr.from, inMailHdr.from);
            continue;
        }

        if (strcmp(requestType, "CL") == 0)
        {
            ss >> lockname;

            CreateLock(lockname, inPktHdr.from, inMailHdr.from);
        }
        else if (strcmp(requestType, "AL") == 0) //need to pass index
        {
            ss >> lockindex;
            
            AcquireLock(lockindex, inPktHdr.from, inMailHdr.from);
        }
        else if (strcmp(requestType, "RL") == 0) //need to pass index
        {
            ss >> lockindex;
            
            ReleaseLock(lockindex, inPktHdr.from, inMailHdr.from);
        }
        else if (strcmp(requestType, "DL") == 0)
        {
            ss >> lockindex;
           
            DestroyLock(lockindex, inPktHdr.from, inMailHdr.from);
        }
        else if (strcmp(requestType, "CC") == 0)
        {
            string cvname;
            ss >> cvname;

            CreateCV(cvname, inPktHdr.from, inMailHdr.from);
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




void ServerToServer ()
{
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
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
        postOffice->Receive(1, &inPktHdr, &inMailHdr, request); // Check Mailbox 0 until Message sent
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

        if (strcmp(requestType, "SCL") == 0)
        {
            if (response == "NO")
            {
                pending.at(indexrequest)->noCount++;
            }

            if (response == "YES")
            {
                pending.at(indexrequest)->fulfilled = true;
            }

            if (pending.at(indexrequest)->numResponse == numServers && !pending.at(indexrequest)->fulfilled)
            {
                bool foundLock = false;

                for (int indexlock = 0; indexlock < 100; indexlock++)
                {
                    if (serverlocks[indexlock]->name == lockname)
                    {
                        // Create the message
                        std::stringstream ss;
                        ss << "200" << " " << "CL" << " " << indexlock;
                        string response = ss.str();
                        SendResponse(response, machineID, mailboxID);
                        foundLock = true;
                        break;
                    }
                }

                if (!foundLock)
                {
                    CreateLock();
                }

                pending.at(indexrequest)->fulfilled = true;
            }
        }

        if (strcmp(requestType, "SCC") == 0)
        {
            if (response == "NO")
            {
                pending.at(indexrequest)->noCount++;
            }

            if (response == "YES")
            {
                pending.at(indexrequest)->fulfilled = true;
            }

            if (pending.at(indexrequest)->numResponse == numServers && !pending.at(indexrequest)->fulfilled)
            {
                bool foundLock = false;

                for (int indexcv = 0; indexcv < 100; indexcv++)
                {
                    if (serverconditions[indexcv]->name == lockname)
                    {
                        // Create the message
                        std::stringstream ss;
                        ss << "200" << " " << "CC" << " " << indexcv;
                        string response = ss.str();
                        SendResponse(response, machineID, mailboxID);
                        foundLock = true;
                        break;
                    }
                }

                if (!foundLock)
                {
                    CreateCV();
                }

                pending.at(indexrequest)->fulfilled = true;
            }
        }

        if (strcmp(requestType, "SCM") == 0)
        {
            if (response == "NO")
            {
                pending.at(indexrequest)->noCount++;
            }

            if (response == "YES")
            {
                pending.at(indexrequest)->fulfilled = true;
            }

            if (pending.at(indexrequest)->numResponse == numServers && !pending.at(indexrequest)->fulfilled)
            {
                bool foundLock = false;

                for (int indexmv = 0; indexmv < 100; indexmv++)
                {
                    if (monitorvariables[indexmv]->name == mvname)
                    {
                        // Create the message
                        std::stringstream ss;
                        ss << "200" << " " << "CM" << " " << indexmv;
                        string response = ss.str();
                        SendResponse(response, machineID, mailboxID);
                        foundLock = true;
                        break;
                    }
                }

                if (!foundLock)
                {
                    CreateMV();
                }

                pending.at(indexrequest)->fulfilled = true;
            }
        }

        if (requestType == "SAL" || requestType == "SRL" || requestType == "SDL" 
            || requestType == "SWC" || requestType == "SSC" || requestType == "SBC" || requestType == "SDC"
            || requestType == "SGM" || requestType == "SSM" || requestType == "SDM")
        {
            if (response == "NO")
            {
                pending.at(indexrequest).noCount++;
            }

            if (response == "YES")
            {
                pending.at(indexrequest).fulfilled = true;
            }

            if (pending.at(indexrequest).noCount == numServers && !pending.at(indexrequest).fulfilled)
            {
                std::stringstream ss;
                ss << "400" << " " << requestType << " " << indexresource;
                string response = ss.str();
                SendResponse(response, machineID, mailboxID);
                pending.at(indexrequest).fulfilled = true;
            }
        }

        



        if (pendingRequest.at(indexrequest)->fulfilled)
        {
            delete pending.at(indexpending);
            pending.at(indexpending) = NULL;
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
