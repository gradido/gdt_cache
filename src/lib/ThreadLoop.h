#ifndef __GDT_CACHE_LIB_THREAD_LOOP
#define __GDT_CACHE_LIB_THREAD_LOOP

#include <thread>
#include <condition_variable>
#include <mutex>

class ThreadLoop
{
public:
    ThreadLoop(int callFrequencySeconds);
    virtual ~ThreadLoop();

    virtual void run() = 0;
    virtual const char* getName() const = 0;
    virtual const char* getType() const = 0;

    inline void notify() {mMainThreadCondition.notify_one();}

protected:
    void threadFunction();

    int mCallFrequencySeconds;
    std::thread mMainThread;
    std::condition_variable mMainThreadCondition;
    std::mutex mMainThreadMutex;
    bool mExitSignal;
};

#endif //__GDT_CACHE_LIB_THREAD_LOOP