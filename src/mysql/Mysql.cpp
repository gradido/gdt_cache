
#include "Mysql.h"

namespace mysql {
    void lockTables(std::vector<std::string> tableNames,
        li::mysql_connection<li::mysql_functions_blocking> connection
    )
    {
        std::string lockTablesQuery = "LOCK TABLES ";
        for(int i = 0; i < tableNames.size(); i++) {
            if(i) {
                lockTablesQuery +=",";
            }
            lockTablesQuery += tableNames[i] + " WRITE";
        }
        connection(lockTablesQuery);
    }

    void unlockTables(li::mysql_connection<li::mysql_functions_blocking> connection)
    {
        connection("UNLOCK TABLES");
    }

}