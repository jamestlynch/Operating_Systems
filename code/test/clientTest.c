#include "copyright.h"

#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"
#include <sstream>

int indexcheck1;

void ClientTest(){
	indexcheck1= CreateLock("abc", -1);
	printf(indexcheck1);
}