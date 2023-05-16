#ifndef __GDT_CACHE_BACKGROUND_TASKS_KLICKTIPP_API_REQUESTS_WORKER_H
#define __GDT_CACHE_BACKGROUND_TASKS_KLICKTIPP_API_REQUESTS_WORKER_H

#include "WorkerThread.h"
#include "KlicktippApiRequest.h"

class KlicktippApiRequestWorker: public WorkerThread<task::KlicktippApiRequest> 
{
public:
    KlicktippApiRequestWorker();
    ~KlicktippApiRequestWorker();

    TaskControl runTask(std::shared_ptr<task::KlicktippApiRequest> task) override;
    const char* getType() const override {return "KlicktippApiRequestWorker";}

protected:    
};


#endif 