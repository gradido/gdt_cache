#include "GlobalModificator.h"

using namespace li;

namespace mysql {
    namespace GlobalModificator {

        model::GlobalModificators getAll(
            mysql_connection<mysql_functions_blocking> connection
        )
        {
            model::GlobalModificators globalModificators;
            auto rows = connection(
                "select id, name, factor, IFNULL(UNIX_TIMESTAMP(start_date),946681200), UNIX_TIMESTAMP(end_date) \
                from global_modificators where end_date < now() order by end_date ASC"
            );
            rows.map([&](int id, std::string name, float factor, std::time_t startDate, std::time_t endDate) {
                globalModificators.push_back(model::GlobalModificator(id, name,factor, startDate, endDate));
            });
            return std::move(globalModificators);
        }
    }
}
