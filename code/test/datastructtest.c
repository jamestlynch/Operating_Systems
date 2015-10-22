#include "syscall.h"

struct Person {
	int ssn;
	int money;
};

int numPeople;

struct Person people[50];

void InitializePeopleData ()
{
	int i;
	struct Person newPerson;

	for (i = 0; i < numPeople; i++)
	{
		newPerson.ssn = i;
		newPerson.money = 500;
	}
}

int main () {
	InitializePeopleData ();
}