#include "ThreadLoop.h"
#include <chrono>
#include <iostream>

ThreadLoop::ThreadLoop(int callFrequencySeconds)
: mCallFrequencySeconds(callFrequencySeconds), mMainThread(&ThreadLoop::threadFunction, this), mExitSignal(false)
{

}

ThreadLoop::~ThreadLoop()
{
    mMainThreadMutex.lock();
    mExitSignal = true;
    mMainThreadMutex.unlock();
    mMainThreadCondition.notify_all();
    if(mMainThread.joinable()) mMainThread.join();
}

void ThreadLoop::threadFunction()
{
    std::unique_lock _lock(mMainThreadMutex);
    int errorCount = 0;
    while(true)
    {            
        // run every x seconds or if some other thread give us something todo
        mMainThreadCondition.wait_for(_lock, std::chrono::seconds(mCallFrequencySeconds));            
        if(mExitSignal) {return;}
        try {
            run();     
            errorCount = 0;   
        } catch(...) {
            std::clog << "Exception while call run from " << getType() << " with name: " << getName() << std::endl;
            errorCount++;
            if(errorCount > 100) {
                std::clog << "more than 100 failed calling from run consequently, exit thread!" << std::endl;
                break;
            }
        }
    }
    
}