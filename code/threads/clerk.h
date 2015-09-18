#ifndef Clerk_H
#define Clerk_H

class Clerk {
public:
    Clerk(char* debugName);		// initialize condition to 
					// "no one waiting"
    ~Clerk();			// deallocate the condition
    char* getName() { return (name); }
    CustomerInteraction();

private:
	List * waitqueue;
	enum clerkState {BUSY, AVAILABLE, BREAK}; //state of the clerk
	enum clerkType {APPLICATION, PICTURE, PASSPORT, CASHIER}; //type of clerk
};

#endif