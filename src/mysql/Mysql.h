#ifndef __GDT_CACHE_MYSQL_MYSQL_H
#define __GDT_CACHE_MYSQL_MYSQL_H

#include <string>
#include <vector>

#include <lithium_mysql.hh>

namespace mysql {
    void lockTables(std::vector<std::string> tableNames,
        li::mysql_connection<li::mysql_functions_blocking> connection
    );
    void unlockTables(li::mysql_connection<li::mysql_functions_blocking> connection);
}

#endif 