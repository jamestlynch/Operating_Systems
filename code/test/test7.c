#include "syscall.h"

int lock;
int cv;
int test;


void t7_t1() {
	test = AcquireLock(lock); 		/*ACQUIRES LOCK*/
	Wait(lock, cv);					/*WILL WAIT ON CV UNTIL BROADCAST IS CALLED IN T3*/
	test = ReleaseLock(lock);
	Write("", sizeof(""), 1);
	Exit(0);
}
void t7_t2() {
	test = AcquireLock(lock);
	DestroyCV(cv); /*ATTEMPTS TO DESTROY A CV THAT HAS A THREAD 1 ON WAIT QUEUE, SET DELETE BECOMES TRUE*/
	Wait(lock, cv); /*WAITS ON THE LOCK AND CV*/
	test = ReleaseLock(lock);
	Exit(0);
}
void t7_t3() {
	test = AcquireLock(lock);
	Write("Thread 3 broadcast", sizeof("Thread 3 broadcast"), 1);
	Broadcast(lock, cv); /*WAKES UP THREAD 1 AND THREAD 2*/
	Write("Thread 3 releases lock", sizeof("Thread 3 releases lock"), 1);
	test = ReleaseLock(lock);
	Exit(0);
}
int main(){
	Write("Test 7 start\n", sizeof("Test 7 start\n"), 1);
	lock= CreateLock("abc", 3);
	cv= CreateCV("def", 3);
    Fork("thread1", 7, t7_t1);
    Fork("thread2", 7, t7_t2);
    Fork("thread3", 7, t7_t3);
}