#include "GdtEntry.h"
#include "../logging/Logging.h"

namespace mysql {
    namespace GdtEntry {

        const std::string GDT_ENTRY_SELECT_STRING("select id, amount, IFNULL(UNIX_TIMESTAMP(date), 0), LOWER(TRIM(email)), IFNULL(comment, ''), \
                IFNULL(source, ''), IFNULL(project, ''), IFNULL(coupon_code, ''), \
                customer_id, \
                gdt_entry_type_id, factor, amount2, factor2, amount * factor * factor2 + amount2 as gdt from gdt_entries "
        );

        model::GdtEntryListPtr getByCustomer(
            int customerId,
            li::mysql_connection<li::mysql_functions_blocking> connection
        )
        {
            auto gdtEntries = std::make_shared<model::GdtEntryList>();

            auto prepared = connection.prepare(GDT_ENTRY_SELECT_STRING + "where customer_id = ? order by date ASC");
            
            auto rows = prepared(customerId);
            rows.map([gdtEntries](
                int id, long long amount, int date, 
                std::string email, std::string comment, std::string source, std::string project, std::string coupon_code,
                int customer_id,
                int gdt_entry_type_id, double factor, long long amount2, double factor2, double gdt) {
                    gdtEntries->addGdtEntry(model::GdtEntry(
                        id, amount, date, 
                        email, comment, source, project, coupon_code, 
                        customer_id,
                        gdt_entry_type_id, factor, amount2, factor2, gdt
                    ));
            });
            return std::move(gdtEntries);
        }

        model::EmailGdtEntriesListMap getAll(
            li::mysql_connection<li::mysql_functions_blocking> connection
        )
        {
            model::EmailGdtEntriesListMap gdtEntries;

            auto rows = connection(GDT_ENTRY_SELECT_STRING + "order by date ASC");
            rows.map([&](
                int id, long long amount, int date, 
                std::string email, std::string comment, std::string source, std::string project, std::string coupon_code,
                int customer_id,
                int gdt_entry_type_id, double factor, long long amount2, double factor2, double gdt) {

                model::GdtEntry entry(
                    id, amount, date, 
                    email, comment, source, project, coupon_code, 
                    customer_id,
                    gdt_entry_type_id, factor, amount2, factor2, gdt
                );
                
                if(!date) {
                    std::string message = "gdt entry with id: ";
                    message += std::to_string(id);
                    message += " and type: ";
                    message += model::GdtEntry::getGdtEntryTypeString(static_cast<model::GdtEntry::GdtEntryType>(gdt_entry_type_id));
                    message += " has an empty date!";
                    LOG_ERROR(message);
                }
                auto it = gdtEntries.find(entry.getCustomerId());
                // customer is new
                if (it == gdtEntries.end()) {
                    it = gdtEntries.insert({entry.getCustomerId(), std::make_shared<model::GdtEntryList>()}).first;
                }
                it->second->addGdtEntry(entry);
            });
            return std::move(gdtEntries);
        }
    }
}