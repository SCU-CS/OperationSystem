#include <windows.h>

#include <fstream>
#include <iostream>
void test(){
    std::cout<<"HelloWorld"<<std::endl;
}
int main(){
    HANDLE thread=CreateThread(0,0,(LPTHREAD_START_ROUTINE)test,0,0,0);
    WaitForSingleObject(thread,-1);
    return 0;
}