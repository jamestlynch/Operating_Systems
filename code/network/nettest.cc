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

#define CL         0
#define SC_Exit         1
#define SC_Exec         2
#define SC_Join         3
#define SC_Create       4

// Test out message delivery, by doing the following:
//	1. send a message to the machine with ID "farAddr", at mail box #0
//	2. wait for the other machine's message to arrive (in our mailbox #0)
//	3. send an acknowledgment for the other machine's message
//	4. wait for an acknowledgement from the other machine to our 
//	    original message

void Server(int farAddr){
    
    stringstream ss;
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char *data= new char;
    char buffer[MaxMailSize];
    stringstream ss;

    while (true){
        printf("Server is started. waiting for message.\n");
        postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
        printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout);

        outPktHdr.to = inPktHdr.from;     
        outMailHdr.to = 0;
        outMailHdr.from = 0;
        outMailHdr.length = strlen(data) + 1;
        ss.clear();
        
        ss<<(char*)buffer; //put buffer object passed into receive, into the stringstream.
        ss.str(buffer); //get the string content

        int syscall;
        ss >> syscall;

        char *name;
        int indexcv, indexlock;


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
       switch (ss) 
            {

                case 'CL': //createlock. what information do we need? the name.
                    printf("Server received Create Lock request from client.\n");
                    ss >> name;
                break;
                /*
                case AL: //acquirelock
                break;

                case RL: //releaselock
                break;

                case DL: //destroylock
                break;

                case CC: //createcv
                break;

                case WC://wait on cv
                break;

                case SC://signal cv
                break;

                case BC://broadcast
                break;

                case DC://destroy
                break;*/

          //process the msg
          //send a reply (maybe)
}}}

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
