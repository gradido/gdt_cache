#ifndef __GDT_CACHE_MYSQL_GLOBAL_MODIFICATOR_H
#define __GDT_CACHE_MYSQL_GLOBAL_MODIFICATOR_H

#include <vector>
#include "../model/GlobalModificator.h"

#include <lithium_mysql.hh>

namespace mysql {
    namespace GlobalModificator {
        //! load all global modificators from db, return as vector because usually we don't need the ids for fast access 
        model::GlobalModificators getAll(
            li::mysql_connection<li::mysql_functions_blocking> connection
        );
    }
}

#endif //__GDT_CACHE_MYSQL_GLOBAL_MODIFICATOR_H