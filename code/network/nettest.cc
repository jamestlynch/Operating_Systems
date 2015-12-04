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

void RespondToClient(int responseCode, string type, int indexresource, int machineID, int mailboxID);
void RespondToServer(int responseCode, int pendingNumber, int machineID, int mailboxID);
void SendResponse(string response, int machineID, int mailboxID);

struct PendingRequest
{
    int machineID;
    int mailboxID;
    string requestType;

    int indexresource
    string name;

    int indexlock;
    bool lockvalidated;

    bool fulfilled;
    int noCount;

    PendingRequest(string type, int machine, int mailbox)
    {
        requestType = type;
        machineID = machine;
        mailboxID = mailbox;
        fulfilled = false;
        noCount = 0;
        lockvalidated = false;
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

struct ClientResponse
{
    string requestType;
    int indexresource;
    int machineID;
    int mailboxID;

    ClientResponse(string type, int index, int machine, int mailbox)
    {
        requestType = type;
        indexresource = index;
        machineID = machine;
        mailbox = mailboxID;
    }
};

enum lockstate { FREE, BUSY };

struct ServerLock
{
    string name;
    lockstate state;
    bool isOwned;
    int machineID;
    int mailbox;
    bool toDelete;
    std::queue<ClientResponse*> waitqueue;
    std::queue<ClientResponse*> busyqueue;

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
// CreateLock
//  TODO: Description
//----------------------------------------------------------------------

int CreateLock(string lockname, int clientMachine, int clientMailbox)
{
    ServerLock *lock = new ServerLock(name, clientMachine, clientMailbox);

    int indexlock = LockMap.find();
    serverlocks[indexlock] = lock;

    return indexlock + (netname * 100);
}

//----------------------------------------------------------------------
// AcquireLock
//  TODO: Description
//----------------------------------------------------------------------

bool AcquireLock(int indexlock, lockstate state, ClientResponse response)
{
    ServerLock *lock = serverlocks[indexlock];

    if (lock->state == BUSY)
    {
        ClientResponse *cr;
        if (state == FREE) cr = new ClientResponse("AL", indexlock, clientMachine, clientMailbox);
        if (state == BUSY) cr = new ClientResponse("WAL", indexlock, clientMachine, clientMailbox);
        lock->busyqueue.push(cr);
        return false;
    }
    else
    {
        if (lock->isOwned)
        {
            lock->waitqueue.push(response);
            return false;
        }
        else 
        {
            lock->isOwned = true;
            lock->machineID = inPktHdr.from;
            lock->mailbox = inMailHdr.from;

            return true;
        }

        lock->state = BUSY;
    }
}

//----------------------------------------------------------------------
// ReleaseLock
//  TODO: Description
//----------------------------------------------------------------------

bool ReleaseLock(int indexlock, lockstate state, int clientMachine, int clientMailbox)
{
    ServerLock *lock = serverlocks[indexlock];

    if (lock->state == BUSY)
    {
        ClientResponse *cr;
        if (state == FREE) cr = new ClientResponse("RL", indexlock, clientMachine, clientMailbox);
        if (state == BUSY) cr = new ClientResponse("WRL", indexlock, clientMachine, clientMailbox);
        lock->busyqueue.push(cr);
        return false;
    }
    else
    {
        if (! lock->waitqueue.empty())
        {
            ClientResponse *cr = lock->waitqueue.front();

            RespondToClient(200, cr->requestType, cr->indexresource, cr->machineID, cr->mailboxID);

            lock->waitqueue.pop();
            lock->machineID = cr->machineID;
            lock->mailbox = cr->mailboxID;
        }
        else
        {
            lock->isOwned = false;

            if (lock->toDelete)
            {
                delete serverlocks[indexlock];
                LockMap.clear(indexlock);
                serverlocks[indexlock] = NULL;
            }
        }

        lock->state = BUSY;
    }

    return true;
}

int FreeLock(int indexlock)
{
    ServerLock *lock = serverlocks[indexlock];

    lock->state = FREE;

    while (! lock->busyqueue.empty())
    {
        ClientResponse *cr = lock->busyqueue.front();

        if (cr->requestType == "AL")
        {
            if (AcquireLock(cr->indexresource, FREE, cr))
            {
                RespondToClient(200, cr->requestType, cr->indexresource, cr->clientMachine, cr->clientMailbox);
            }
        }
        if (cr->requestType == "RL")
        {
            if (ReleaseLock(cr->indexresource, FREE, cr))
            {
                RespondToClient(200, cr->requestType, cr->indexresource, cr->clientMachine, cr->clientMailbox);
            }
        }
        if (cr->requestType == "DL")
        {
            if (DestroyLock(cr->indexresource, FREE, cr))
            {
                RespondToClient(200, cr->requestType, cr->indexresource, cr->clientMachine, cr->clientMailbox);
            }
        }
        if (cr->requestType == "WAL")
        {
            if (AcquireLock(cr->indexresource, BUSY, cr))
            {
                // Wait is COMPLETE!
                RespondToClient(200, cr->requestType, cr->indexresource, cr->clientMachine, cr->clientMailbox);
            }
        }
        if (cr->requestType == "WRL")
        {
            ReleaseLock(cr->indexresource, BUSY, cr);
        }

        lock->busyqueue.pop();
    }
}

//----------------------------------------------------------------------
// DestroyLock
//  TODO: Description
//----------------------------------------------------------------------

void DestroyLock(int indexlock, int clientMachine, int clientMailbox)
{
    ServerLock *lock = serverlocks[indexlock];

    if(!lock->isOwned && lock->waitqueue.empty())
    {
        delete lock;
        LockMap.clear(indexlock);
        serverlocks[indexlock] = NULL;
    }
    else
    {
        lock->toDelete = true;
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
    std::queue<ClientResponse*> waitqueue;

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
// DeleteCondition
//  TODO: Description
//----------------------------------------------------------------------

void DeleteCondition(unsigned int indexcv)
{
    ServerCV *condition = serverconditions[indexcv];

    delete condition;
    CVMap.clear(indexcv);
    serverconditions[indexcv] = NULL;
}

//----------------------------------------------------------------------
// CreateCV
//  TODO: Description
//----------------------------------------------------------------------

int CreateCV(string cvname, int machineID, int mailboxID)
{
    // Create Condition
    ServerCV *condition = new ServerCV(cvname);
    int indexcv = CVMap.find();
    serverconditions[indexcv] = condition;

    return indexcv + (netname * 100);
}

//----------------------------------------------------------------------
// WaitCV
//  TODO: Description
//----------------------------------------------------------------------

void WaitCV(unsigned int indexcv, int machineID, int mailboxID)
{
    ServerCV *condition = serverconditions[indexcv];

    ClientResponse waitingMachine = new ClientResponse("WC", indexcv, machineID, mailboxID);

    condition->waitqueue.push(waitingMachine);
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
        if (serverlocks[indexlock]->isOwned)
        {
            serverlocks[indexlock]->waitqueue.push(response);
        }
        // Lock is valid and available so get the lock
        else
        {
            printf("About to acquire the lock\n");
            serverlocks[indexlock]->isOwned = true; // Make Lock Busy
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

        return indexmv + (netname * 100);
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

    outPktHdr.to = machineID; // Client Machine ID
    outMailHdr.to = mailboxID; // Client Machine ID
    outMailHdr.from = 0; // Server Mailbox ID

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

//========================================================================================================================================
//
// Task Server
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

void TaskServer()
{
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char request[MaxMailSize];

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
        
        postOffice->Receive(1, &inPktHdr, &inMailHdr, request); // Check Mailbox 0 until Message sent
        DEBUG('n', "Received \"%s\" from %d, box %d\n", request, inPktHdr.from, inMailHdr.from);

        // Clear stringstream to parse request
        stringstream ss;
        fflush(stdout);
        ss.str(request);

        // Message Format: <ResponseType> <ClientMachine> <ClientMailbox> <... TaskSpecificParams>
        string requestType;
        int clientMachine, clientMailbox;
        ss >> requestType >> clientMachine >> clientMailbox;

        DEBUG('n', "Type of RPC: %s\n", requestType);

        if (requestType == "CL")
        {
            string cvname;
            ss >> cvname;

            int indexlock = CreateCV(lockname, clientMachine, clientMailbox);
            RespondToClient(201, requestType, indexlock, clientMachine, clientMailbox);
        }
        if (requestType == "AL")
        {
            unsigned int indexlock;
            ss >> indexlock;

            // Prepare Message before-hand because Acquire May either be completing
            //  a Wait call or an Acquire call.
            ClientResponse response = new ClientResponse(requestType, indexlock, clientMachine, clientMailbox);
            
            // If able to Acquire Lock
            if (AcquireLock(indexlock, FREE, response))
            {
                RespondToClient(200, requestType, indexlock, clientMachine, clientMailbox);
            }
        }
        if (requestType == "RL")
        {
            unsigned int indexlock;
            ss >> indexlock;
            
            if (ReleaseLock(indexlock, clientMachine, clientMailbox))
            {
                RespondToClient(200, requestType, indexlock, clientMachine, clientMailbox);
            }
        }
        if (requestType == "DL")
        {
            unsigned int indexlock;
            ss >> indexlock;
            
            DestroyLock(indexlock, clientMachine, clientMailbox);
            RespondToClient(200, requestType, indexlock, clientMachine, clientMailbox);
        }
        if (requestType == "WFL")
        {
            unsigned int indexlock;
            ss >> indexlock;

            FreeLock(indexlock);
        }
        if (requestType == "SFL")
        {
            unsigned int indexlock;
            ss >> indexlock;

            FreeLock(indexlock);
        }
        if (requestType == "CC")
        {
            string cvname;
            ss >> cvname;

            int indexcv = CreateCV(lockname, clientMachine, clientMailbox);
            RespondToClient(201, requestType, indexcv, clientMachine, clientMailbox);
        }
        if (requestType == "WC")
        {
            unsigned int indexcv;
            ss >> indexcv;
            
            WaitCV(indexcv, clientMachine, clientMailbox);
            
            NextStep("WFL", pendingNumber, clientMachine, clientMailbox);
        }
        if (requestType == "WAL")
        {
            unsigned int indexcv, indexlock;
            ss >> indexcv >> indexlock;

            // Prepare Message before-hand because Acquire May either be completing
            //  a Wait call or an Acquire call.
            ClientResponse response = new ClientResponse("WC", indexcv, clientMachine, clientMailbox);
            
            // If able to Acquire Lock
            if (AcquireLock(indexlock, BUSY, response))
            {
                RespondToClient(200, requestType, indexcv, clientMachine, clientMailbox);
            }
        }
        if (requestType == "WRL")
        {
            unsigned int indexlock, pendingNumber;
            ss >> indexlock >> pendingNumber;
            
            if (ReleaseLock(indexlock, BUSY, clientMachine, clientMailbox))
            {
                NextStep("WC2", pendingNumber, clientMachine, clientMailbox);
            }
        }
        if (requestType == "WR") // Release Lock from Wait to preserve requestType
        {
            unsigned int indexcv, indexlock;
            ss >> indexcv >> indexlock;
            
            ReleaseLock(indexlock, clientMachine, clientMailbox);
            RespondToClient(200, "WC", indexcv, clientMachine, clientMailbox);
        }
        if (requestType == "WA") // Re-Acquire Lock from Wait to preserve requestType
        {
            unsigned int indexcv, indexlock;
            ss >> indexcv >> indexlock;

            // Prepare Message before-hand because Acquire May either be completing
            //  a Wait call or an Acquire call.
            fflush(stdout);
            string response;
            ss << 200 << " " << requestType << " " << indexcv;
            
            // If able to Acquire Lock; Else Response was Queued.
            if (AcquireLock(indexlock, response, clientMachine, clientMailbox))
            {
                RespondToClient(200, "WC", indexcv, clientMachine, clientMailbox);
            }            
        }
        if (requestType == "SC")
        {
            unsigned int indexcv;
            ss >> indexcv;

            SignalCV(indexcv, clientMachine, clientMailbox);
            RespondToClient(200, requestType, indexcv, clientMachine, clientMailbox);
        }
        else if (strcmp(requestType, "BC") == 0)
        {
            unsigned int indexcv;
            ss >> indexcv;
            
            BroadcastCV(indexcv, clientMachine, clientMailbox);
            RespondToClient(200, requestType, indexcv, clientMachine, clientMailbox);
        }
        else if (strcmp(requestType, "DC") == 0)
        {
            unsigned int indexcv;
            ss >> indexcv;

            DestroyCV(indexcv, clientMachine, clientMailbox);
            RespondToClient(200, requestType, indexcv, clientMachine, clientMailbox);
        }
        else if (strcmp(requestType, "CM") == 0)
        {
            string mvname;
            int mvsize;
            ss >> mvname >> mvsize;

            int indexmv = CreateMV(mvname, mvsize, clientMachine, clientMailbox);
            RespondToClient(201, requestType, indexmv, clientMachine, clientMailbox);
        }
        else if (strcmp(requestType, "SM") == 0)
        {
            unsigned int indexmv, indexvar;
            int value;
            ss >> indexmv >> indexvar >> value;

            SetMV(indexmv, indexvar, value, clientMachine, clientMailbox);
            RespondToClient(200, requestType, indexmv, clientMachine, clientMailbox);
        }
        else if (strcmp(requestType, "GM") == 0)
        {
            unsigned int indexmv, indexvar;
            ss >> indexmv >> indexvar;

            GetMV(indexmv, indexvar, clientMachine, clientMailbox);
            RespondToClient(200, requestType, indexmv, clientMachine, clientMailbox);
        }
        else if (strcmp(requestType, "DM") == 0)
        {
            unsigned int indexmv;
            ss >> indexmv;

            DestroyMV(indexmv, clientMachine, clientMailbox);
            RespondToClient(200, requestType, indexmv, clientMachine, clientMailbox);
        }
    }
}


//========================================================================================================================================
//
// Router
//
//========================================================================================================================================

void PerformTask (string request, int clientMachine, int clientMailbox);
void ScheduleTask (string request, int clientMachine, int clientMailbox);

bool LookupResource (string request, int machineID, int mailboxID);
int LookupLock(string request, int machineID, int mailboxID);
int LookupCV(string request, int machineID, int mailboxID);

void ForwardRequest (char *request, int machineID, int mailboxID);
void RespondToServer (int resourceStatus, int pendingNumber, int serverMachine, int serverMailbox);

void Router ()
{
    PacketHeader inPktHdr;
    MailHeader inMailHdr;
    char request[MaxMailSize];

    DEBUG('n', "Router is started.\n");

    while (true)
    {
        DEBUG('n', "Waiting for message.\n");
        
        postOffice->Receive(0, &inPktHdr, &inMailHdr, request);
        DEBUG('n', "Received \"%s\" from %d, box %d\n", request, inPktHdr.from, inMailHdr.from);

        // Clear stringstream to parse request
        stringstream ss;
        fflush(stdout);
        ss.str(request);

        string requestType;
        ss >> requestType;

        // Message format: REQ <PendingNumber> <clientMachine> <clientMailbox> <originalRequest>
        if (requestType == "REQ")
        {
            unsigned int pendingNumber, clientMachine, clientMailbox;
            ss >> pendingNumber >> clientMachine >> clientMailbox;

            string originalRequest;
            getline(ss, originalRequest);

            int lookupResponse = LookupResource(forwardedRequest);

            // If we have the resource
            if (lookupResponse == 302)
            {
                RespondToServer(302, pendingNumber, inPktHdr.from, inMailHdr.from); // YES

                PerformTask(originalRequest, clientMachine, clientMailbox);
            }
            // Validating Lock for CV Call:
            if (lookupResponse == 401)
            {
                RespondToServer(401, pendingNumber, inPktHdr.from, inMailHdr.from); // Machine does not own CV's Lock
            }
            if (lookupResponse == 404)
            {
                RespondToServer(404, pendingNumber, inPktHdr.from, inMailHdr.from); // NO
            }
        }

        // Message format: RES <ResponseCode> <PendingNumber>
        else if (requestType == "RES")
        {
            int responseCode, pendingNumber;
            ss >> responseCode >> pendingNumber;

            PendingRequest *pendingReq = pending.at(indexrequest);

            // Server Found Resource
            if (responseCode == 302)
            {
                pendingReq->fulfilled = true;
            }

            // Unauthorized: Do Not Own Lock Corresponding to CV
            if (responseCode == 401)
            {
                RespondToClient(401, pendingReq->requestType, pendingReq->indexresource, clientMachine, clientMailbox);
                pendingReq->fulfilled = true;
            }

            // Not Found
            else if (responseCode == 404)
            {
                pendingReq->noCount++;
            }

            // If All Responses and Still Unfulfilled
            if (pendingReq->noCount == numServers && !pendingReq->fulfilled)
            {
                if (pendingReq->requestType == "CL" || pendingReq->requestType == "CC" || pendingReq->requestType == "CM")
                {
                    string createRequest;
                    fflush(stdout);
                    ss << pendingReq->requestType << " " << pendingReq->name;
                    getline(ss, createRequest);

                    ScheduleTask(createRequest, pendingReq->machineID, pendingReq->mailboxID); // Create Resource
                }
                else
                {
                    RespondToClient(400, pendingReq->requestType, pendingReq->indexresource, clientMachine, clientMailbox);
                }

                pending.at(indexrequest)->fulfilled = true;
            }

            if (pendingRequest.at(indexrequest)->fulfilled)
            {
                delete pending.at(indexpending);
                pending.at(indexpending) = NULL;
            }
        }

        // Client Message
        // Message Format: <RequestType> <... TaskSpecificParams>
        else
        {
            int lookupResponse = LookupResource(string(request));

            // If we have the resource
            if (lookupResponse == 302)
            {
                PerformTask(string(request), clientMachine, clientMailbox);
            }

            // Invalid 
            if (lookupResponse == 400 || lookupResponse == 401)
            {
                int indexresource;
                ss >> indexresource;

                RespondToClient(lookupResponse, requestType, indexresource, clientMachine, clientMailbox);
            }

            // Not Found
            if (lookupResponse == 404)
            {
                ForwardRequest(request, clientMachine, clientMailbox);
            }
        }
    }
}

bool LookupResource (string request, int machineID, int mailboxID)
{
    stringstream ss;
    ss.str(request);

    string requestType;
    ss >> requestType;

    if (requestType == "CL" || requestType == "AL" || requestType == "RL" || requestType == "DL" 
        || requestType == "WC" || requestType == "SC" || requestType == "BC" || requestType == "WFL" || requestType == "SFL")
    {
        return LookupLock(request, machineID, mailboxID);
    }

    if (requestType == "CC" || requestType == "WC2" || requestType == "SC2" || requestType == "BC2" || requestType == "DC")
    {
        return LookupCV(request, machineID, mailboxID);
    }

    if (requestType == "CM" || requestType == "SM" || requestType == "GM" || requestType == "DM")
    {
        return LookupMV(request, machineID, mailboxID);
    }
}

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

int LookupLock(string request, int machineID, int mailboxID)
{
    fflush(stdout);
    ss.str(request);

    string requestType;
    ss >> requestType;

    if (requestType == "CL")
    {
        string lockname;
        ss >> lockname;

        for (int index = 0; index < 100; index++)
        {
            if (serverlocks[index])
            {
                if (serverlocks[index]->name == lockname)
                {
                    RespondToClient(201, requestType, index, machineID, mailboxID);
                    return 302; // FOUND Lock
                }
            }
        }

        return 404;
    }

    int indexlock;
    ss >> indexlock;

    indexlock = indexlock % (netname * 100);

    // (1) Index corresponds to valid location: 400
    if (indexlock >= 100 || indexlock < 0)
    {
        return 400;
    }

    ServerLock *lock = serverlocks[indexlock];

    // (2) Lock not found: 404
    if (!lock)
    {
        return 404;
    }

    // UNAUTHORIZED: Trying to release a Lock client does not own
    if (requestType == "RL" || requestType == "VL")
    {
        if (lock->machineID != clientMachine || lock->mailbox != clientMailbox))
        {
            return 401;
        }
    }

    // No Errors; FOUND Lock
    return 302;
}

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

int LookupCV(string request, int machineID, int mailboxID)
{
    // (1) Corresponds to valid condition location: 400
    if (indexcv >= 100 || indexcv < 0)
    {
        return 400;
    }

    // Create the message
    std::stringstream ss;
    fflush(stdout);
    ss.str(request);

    string requestType;
    ss >> requestType;

    // Create: heck if Server Owns CV Already
    if (requestType == "CC")
    {
        string cvname;
        ss >> cvname;

        for (int index = 0; index < 100; index++)
        {
            if (serverconditions[index])
            {
                if (serverlocks[index]->name == cvname)
                {
                    RespondToClient(201, requestType, index, machineID, mailboxID);
                    return 302; // FOUND Lock
                }
            }
        }

        return 404;
    }

    int indexcv;
    ss >> indexcv;

    indexcv = indexcv % (netname * 100);

    // Get the Condition and Wait
    ServerCV *condition = serverconditions[indexcv];

    // (2) Condition exists: 404
    if (!condition)
    {
        return 404;
    }
    else
    {
        int indexlock;
        ss >> indexlock;

        // UNAUTHORIZED: If lock passed in belongs to CV's C.S.
        if (condition->cvlockSet && condition->conditionlock != indexlock)
        {
            return 401; 
        }

    
        // If CV has no corresponding lock, set its lock
        if (!condition->cvlockSet)
        {
            condition->conditionlock = indexlock;
            condition->cvlockSet = true;
        }
    }

    // No errors
    return 0;
}


void ForwardRequest (char *request, int machineID, int mailboxID)
{
    stringstream ss;
    fflush(stdout);
    ss.str(request);

    string requestType;
    ss >> requestType;

    PendingRequest pendingRequest = new PendingRequest(requestType, machineID, mailboxID);

    if (requestType == "CL" || requestType == "CC" || requestType == "CM")
    {
        string name;
        ss >> name;
        pendingRequest.name = name;
    }
    else
    {
        int indexresource;
        ss >> indexresource;
        pendingRequest.indexresource = indexresource;

        if (requestType == "WC" || requestType == "SC" || requestType == "BC")
        {
            int indexlock;
            ss >> indexlock;
            pendingRequest.indexlock = indexlock;
        }
    }

    pending.push_back(pendingRequest);

    stringstream ss;
    ss << "REQ" << " ";
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

        string forwardedRequest = ss.str();

        SendResponse(forwardedRequest, indexserver, 0);
    }
}

void RespondToServer (int resourceStatus, int pendingNumber, int serverMachine, int serverMailbox)
{
    stringstream ss;
    ss << "RES" << " " << resourceStatus << " " << pendingNumber;

    string response = ss.str();

    SendResponse(response, serverMachine, serverMailbox);
}

void ScheduleTask (string request, int clientMachine, int clientMailbox)
{
    stringstream ss;
    fflush(stdout);
    ss.str(request);

    string requestType, requestParams;
    ss >> requestType;
    getline(ss, requestParams);

    fflush(stdout);
    
    if (requestType == "WC") requestType = "WRL";
    if (requestType == "WC2") requestType = "WC";

    // Message Format: <requestType> <ClientMachine> <ClientMailbox> <... TaskSpecificParams>
    ss << requestType << " " << clientMachine << " " << clientMailbox << " " << requestParams;

    string task = ss.str();

    SendResponse(task, netname, 1);
}

void PerformTask (string request, int clientMachine, int clientMailbox)
{
    stringstream ss;
    fflush(stdout);
    ss.str(request);

    string requestType;
    ss >> requestType;

    // Step 1 of Wait/Signal/Broadcast: Release Lock
    if (requestType == "WC" || requestType == "SC" || requestType == "BC")
    {
        unsigned int indexcv, indexlock;
        ss >> indexcv >> indexlock;

        PendingRequest cvRequest = new PendingRequest(requestType, inPktHdr.from, inMailHdr.from);
        cvRequest.indexresource = indexcv;
        cvRequest.indexlock = indexlock;
        pending.push_back(cvRequest);

        string taskRequest;
        fflush(stdout);
        ss << requestType << " " << (pending.size() - 1) << " " << cvRequest->indexlock;
        getline(ss, taskRequest);

        ScheduleTask(taskRequest, inPktHdr.from, inMailHdr.from));
    }

    else if (requestType != "CL" && requestType != "CC" && requestType != "CM")
    {
        ScheduleTask(request, inPktHdr.from, inMailHdr.from));
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
