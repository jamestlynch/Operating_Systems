#ifndef CUSTOMER_H
#define CUSTOMER_H

class Customer {
public:
    Customer(char* debugName);		// initialize condition to 
					// "no one waiting"
    ~Customer();			// deallocate the condition
    char* getName() { return (name); }
    int getSSN(){ return SSN; }
    ClerkInteraction();



private:
	char* name;
	int SSN;
	int pictureid;
	int money;
	bool completedApplication;
	
    //int clerkBribeLineCV[5]
};

#endif