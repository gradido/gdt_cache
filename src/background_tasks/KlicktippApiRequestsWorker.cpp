#include "KlicktippApiRequestsWorker.h"


KlicktippApiRequestWorker::KlicktippApiRequestWorker()
: WorkerThread(0)
{

}

KlicktippApiRequestWorker::~KlicktippApiRequestWorker()
{

}

KlicktippApiRequestWorker::TaskControl KlicktippApiRequestWorker::runTask(std::shared_ptr<task::KlicktippApiRequest> task)
{   
    task->run();
    return TaskControl::REMOVE;
}
