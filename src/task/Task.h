#ifndef __GDT_CACHE_TASK_TASK_H
#define __GDT_CACHE_TASK_TASK_H

#include "../controller/GdtEntries.h"

namespace task 
{
    /*!
     * Task Classes Base
     */
    class Task
    {
    public:
        Task();
        virtual ~Task();
        // tasks are ready by default
        virtual bool isReady() const noexcept {return true;}
        virtual void run(
            li::mysql_connection<li::mysql_functions_blocking> connection,
            controller::GdtEntries& gdtEntries
        ) = 0;

        virtual const char* getClass() const = 0;
        virtual const char* getName() const = 0;
    protected:
    };
}

#endif //__GDT_CACHE_TASK_TASK_H