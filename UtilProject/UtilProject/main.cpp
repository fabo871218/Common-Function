#include <iostream>
#include "SmartBuffer.h"
#include "SpinLock.h"
#include <Windows.h>

void test_lock(){
    SpinLock lock;

    CRITICAL_SECTION    cs;
    ::InitializeCriticalSection(&cs);
    int index = 0;
    for (int i = 0; i < 10; i++){
        std::thread* pthread = new std::thread([&]()->void{
            bool bret = false;
            while (!bret){
                //::EnterCriticalSection(&cs);
                lock.lock();
                if (index == 0){
                    std::cout << "Begin:" << clock() << std::endl;
                }
                else if (index >= 10000000){
                    std::cout << "End:" << clock() << std::endl;
                    bret = true;
                }
                index++;
                //std::cout << "index:" << index << std::endl;
                //::LeaveCriticalSection(&cs);
                lock.unlock();
            }
        });
    }
}

void test_buffer(){

}

int main(){
    test_buffer();

    system("pause");
    return 0;
}