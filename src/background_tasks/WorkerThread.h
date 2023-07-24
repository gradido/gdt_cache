#ifndef __GDT_CACHE_BACKGROUND_TASKS_WORKER_THREAD
#define __GDT_CACHE_BACKGROUND_TASKS_WORKER_THREAD

#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>

#include <chrono>
#include <iostream>
#include "../logging/Logging.h"

/*!
 * @author einhornimmond
 * @date 09.05.2023
 * @brief Class using a asynchronous running task, call run every desired time or if a new task is waiting for processing
*/
template <typename T>
class WorkerThread
{
public:
    enum class TaskControl {
        RE_SCHEDULE,
        REMOVE
    };

    WorkerThread(int callFrequencySeconds);
    virtual ~WorkerThread();


    //! start thread
    void startThread();    

    //! work on task
    virtual TaskControl runTask(std::shared_ptr<T> task) {return TaskControl::REMOVE;}

    //! called after cleaning up tasks list
    //! called at least every callFrequencySeconds
    virtual void run() {};

    //! for error reporting    
    virtual const char* getType() const = 0;    

    inline void pushTask(std::shared_ptr<T> task) {
        mTasksMutex.lock();
        mTasks.push(task);
        mTasksMutex.unlock();
        mMainThreadCondition.notify_one();
    }  

protected:
    void threadFunction();

    //! for controlling thread
    int mCallFrequencySeconds;
    std::thread mMainThread;
    std::condition_variable mMainThreadCondition;
    std::mutex mMainThreadMutex;
    bool mExitSignal;

    //! for controlling tasks
    std::queue<std::shared_ptr<T>> mTasks;
    std::mutex mTasksMutex; 
};

template<typename T>
WorkerThread<T>::WorkerThread(int callFrequencySeconds)
: mCallFrequencySeconds(callFrequencySeconds), mExitSignal(false)
{

}

template<typename T>
WorkerThread<T>::~WorkerThread()
{
    mMainThreadMutex.lock();
    mExitSignal = true;
    mMainThreadMutex.unlock();
    mMainThreadCondition.notify_all();
    if(mMainThread.joinable()) mMainThread.join();
}



template<typename T>
void WorkerThread<T>::threadFunction()
{
    std::unique_lock _lock(mMainThreadMutex);
    int errorCount = 0;
    while(true)
    {            
        if(mCallFrequencySeconds) {
            // run every x seconds or if some other thread give us something todo
            mMainThreadCondition.wait_for(_lock, std::chrono::seconds(mCallFrequencySeconds));            
        } else {
            mMainThreadCondition.wait(_lock);
        }
        if(mExitSignal) {return;}
        try {
            run();     
            errorCount = 0;   
        } catch(...) {
            std::string message = "Exception while call run from ";
            message += getType();
            LOG_ERROR(message);
            errorCount++;
            if(errorCount > 100) {
                LOG_ERROR("more than 100 exceptions while calling run consequently, exit thread!");
                break;
            }
        }
        // check if we have some tasks to run        
        while(mTasks.size()) 
        {
            // if exit was signaled we exit and not run anymore tasks
            if(mExitSignal) {return; }
            std::unique_lock _lockTask(mTasksMutex);
            
            std::shared_ptr<T> task;            
            task = mTasks.front();
            mTasks.pop();           
            // while we run task, unlock mutex so other can push new tasks to queue
            _lockTask.unlock();
            
            try {
                auto taskResult = runTask(task);
                if(taskResult == TaskControl::RE_SCHEDULE) {
                   _lockTask.lock();
                    mTasks.push(task);
                }
            } catch(std::exception& ex) {
                std::string message = "exception: ";
                message += ex.what();
                LOG_ERROR(message);
            } catch(...) {
                LOG_ERROR("unspecified exception");
            }
        }        
    }    
}

template<typename T>
void WorkerThread<T>::startThread()
{
    mMainThread = std::thread(&WorkerThread<T>::threadFunction, this);
}

#endif //__GDT_CACHE_BACKGROUND_TASKS_WORKER_THREAD