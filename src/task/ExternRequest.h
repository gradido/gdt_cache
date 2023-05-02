#ifndef __GDT_CACHE_TASK_EXTERN_REQUEST_H
#define __GDT_CACHE_TASK_EXTERN_REQUEST_H

namespace task {
    class ExternRequest
    {
    public:
        ExternRequest();
        virtual ~ExternRequest();
        virtual void run() = 0;
        
    protected:
    };
}

#endif // __GDT_CACHE_TASK_EXTERN_REQUEST_H