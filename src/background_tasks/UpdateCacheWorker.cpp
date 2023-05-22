
#include "UpdateCacheWorker.h"
#include "../CacheServer.h"
#include "../model/Config.h"

UpdateCacheWorker::UpdateCacheWorker()
: WorkerThread(60), mDBConnection(g_Config->database->connect())
{

}

UpdateCacheWorker::~UpdateCacheWorker()
{

}

void UpdateCacheWorker::run()
{
    auto cs = CacheServer::getInstance();
    // should we update allowed ips? (maybe some dynamic ip has changed)
    // update if it is older than 1 hour
    cs->updateAllowedIps();

    // reload all gdt entries and contacts every maxCacheTimeout seconds (default 4h)
    cs->reloadCacheAfterTimeout(mDBConnection);
}

UpdateCacheWorker::TaskControl UpdateCacheWorker::runTask(std::shared_ptr<task::UpdateGdtEntryList> task)
{
    task->run(mDBConnection);
    return TaskControl::REMOVE;
}