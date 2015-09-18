#ifndef SENATOR_H
#define SENATOR_H

class Senator {
public:
    Senator(char* debugName);		// initialize condition to 
					// "no one waiting"
    ~Senator();			// deallocate the condition
    char* getName() { return (name); }

private:
	char* name;
};


#endif