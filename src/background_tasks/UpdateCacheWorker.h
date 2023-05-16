#ifndef __GDT_CACHE_BACKGROUND_TASKS_UPDATE_CACHE_WORKER_H
#define __GDT_CACHE_BACKGROUND_TASKS_UPDATE_CACHE_WORKER_H

#include "WorkerThread.h"
#include "UpdateGdtEntryList.h"

class UpdateCacheWorker: public WorkerThread<task::UpdateGdtEntryList> 
{
public:
    UpdateCacheWorker();
    ~UpdateCacheWorker();
    
    void run() override;
    TaskControl runTask(std::shared_ptr<task::UpdateGdtEntryList> task) override;

    const char* getType() const override {return "UpdateCacheWorker";}

protected:    
    li::mysql_connection<li::mysql_functions_blocking> mDBConnection;
};

#endif //__GDT_CACHE_BACKGROUND_TASKS_UPDATE_CACHE_WORKER_H