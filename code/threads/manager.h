#ifndef MANAGER_H
#define MANAGER_H

class Manager {
public:
    Manager(char* debugName);		// initialize condition to 
					// "no one waiting"
    ~Manager();			// deallocate the condition
    char* getName() { return (name); }
    void clerkInteraction();
    //wake up clerks when >3 people go on it's line
    //print current total $ made from each clerk type
    //timer thread

private:
	char* name;
	int overalltotal;
	int appclerktotal;
	int picclerktotal;
	int passportclerktotal;
	int cashiertotal;
};


#endif