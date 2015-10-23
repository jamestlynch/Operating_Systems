#include "syscall.h"

int lock;
int cv;
int test;

void t7_t1() {
	test = AcquireLock(lock);
	Write("Thread 1 acquired lock and tried to destroy, but threads on wait queue.", sizeof("Thread 1 acquired lock and tried to destroy, but threads on wait queue."), 1);
	DestroyCV(cv);
	Write("Thread 1 going to wait.", sizeof("Thread 1 going to wait."), 1);
	Wait(lock, cv);
	test = ReleaseLock(lock);
	Exit(0);
}
void t7_t2() {
	test = AcquireLock(lock);
	Write("Thread 2 getting on sleepqueue", sizeof("Thread 2 getting on sleepqueue"), 1);
	Wait(lock, cv);
	Write("Thread 2 getting off sleepqueue", sizeof("Thread 2 getting off sleepqueue"), 1);
	test = ReleaseLock(lock);
	Write("Passses if thread 1 attempts to destroycv, sleeps, thread 2 sleeps, thread 3 broadcasts/releases, thread 1 destroyscv", sizeof("Passses if thread 1 attempts to destroycv, sleeps, thread 2 sleeps, thread 3 broadcasts/releases, thread 1 destroyscv "), 1);
	Exit(0);
}
void t7_t3() {
	test = AcquireLock(lock);
	Write("Thread 3 broadcast", sizeof("Thread 3 broadcast"), 1);
	Broadcast(lock, cv);
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

/*

PASSED

*/