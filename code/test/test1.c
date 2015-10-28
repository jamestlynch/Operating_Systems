/* testfiles.c
 *  Simple program to test the file handling system calls
 */

#include "syscall.h"

int test, indexcheck1, indexcheck2, indexcheck3, indexcheck4;

void t1_t1(){
    
    /*checks destroy and create invalid input is handled properly*/
    indexcheck1= CreateLock("abc", -1);
    indexcheck2=DestroyLock(indexcheck1+100);
    indexcheck3=DestroyLock(-100);
    if (indexcheck1!= -1 || indexcheck2!=-1 || indexcheck3!=-1){
        Write("failed: create/destroyLOCK validates input\n", sizeof("failed: create/destroyLOCK validates input\n"), 1);    }
    else{
        Write("passed: create/destroyLOCK validates input\n", sizeof("passed: create/destroyLOCK validates input\n"), 1);
        }
        Exit(0);
}
int main() 
{
    Write("Test 1 start\n", sizeof("Test 1 start\n"), 1);
    Fork("thread1", 7, t1_t1);
    Exit(0);
}