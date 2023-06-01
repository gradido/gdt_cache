#include "GdtEntry.h"
#include "../logging/Logging.h"

namespace mysql {
    namespace GdtEntry {

        const std::string GDT_ENTRY_SELECT_STRING("select id, amount, UNIX_TIMESTAMP(date), LOWER(TRIM(email)), IFNULL(comment, ''), \
                IFNULL(source, ''), IFNULL(project, ''), IFNULL(coupon_code, ''), \
                gdt_entry_type_id, factor, amount2, factor2, amount * factor * factor2 + amount2 as gdt from gdt_entries "
        );

        model::GdtEntryListPtr getByCustomer(
            model::CustomerPtr customer,
            li::mysql_connection<li::mysql_functions_blocking> connection
        )
        {
            auto gdtEntries = std::make_shared<model::GdtEntryList>();

            auto prepared = connection.prepare(GDT_ENTRY_SELECT_STRING + "where email IN (?) order by date ASC");
            
            auto rows = prepared(customer->getEmailsString());
            rows.map([gdtEntries](
                int id, long long amount, int date, 
                std::string email, std::string comment, std::string source, std::string project, std::string coupon_code,
                int gdt_entry_type_id, double factor, long long amount2, double factor2, double gdt) {
                    gdtEntries->addGdtEntry(model::GdtEntry(
                        id, amount, date, 
                        email, comment, source, project, coupon_code, 
                        gdt_entry_type_id, factor, amount2, factor2, gdt
                    ));
            });
            /*while (auto row = rows.read_optional<int, long long, long long, std::string, std::string, std::string, std::string, std::string, int, double, long long, double>())
            {
                gdtEntries->addGdtEntry(model::GdtEntry(row.value()));
            }*/
            return std::move(gdtEntries);
        }

        model::EmailGdtEntriesListMap getAll(
            model::CustomersMap& customers,
            li::mysql_connection<li::mysql_functions_blocking> connection,
            std::vector<std::string>& emailsNotInCustomer
        )
        {
            model::EmailGdtEntriesListMap gdtEntries;
            for (auto customer : customers)
            {
                auto gdtEntriesList = std::make_shared<model::GdtEntryList>();
                for (auto email : customer.second->getEmails()) {
                    gdtEntries.insert({email, gdtEntriesList});
                }
            }
            auto rows = connection(GDT_ENTRY_SELECT_STRING + "order by date ASC");
            rows.map([&](
                int id, long long amount, int date, 
                std::string email, std::string comment, std::string source, std::string project, std::string coupon_code,
                int gdt_entry_type_id, double factor, long long amount2, double factor2, double gdt) {

                model::GdtEntry entry(
                    id, amount, date, 
                    email, comment, source, project, coupon_code, 
                    gdt_entry_type_id, factor, amount2, factor2, gdt
                );
                // skip gdt entries with empty emails
                if (entry.getEmail().size() == 0)
                    return;

                auto it = gdtEntries.find(entry.getEmail());
                // email isn't already a contact
                if (it == gdtEntries.end())
                {
                    it = gdtEntries.insert({entry.getEmail(), std::make_shared<model::GdtEntryList>()}).first;
                    emailsNotInCustomer.push_back(entry.getEmail());
                }
                it->second->addGdtEntry(entry);
            });
            /*while (auto row = rows.read_optional<int, long long, int, std::string, std::string, std::string, std::string, std::string, int, double, int, double>())
            {
                model::GdtEntry entry(row.value());
                // skip gdt entries with empty emails
                if (entry.getEmail().size() == 0)
                    continue;

                auto it = gdtEntries.find(entry.getEmail());
                // email isn't already a contact
                if (it == gdtEntries.end())
                {
                    it = gdtEntries.insert({entry.getEmail(), std::make_shared<model::GdtEntryList>()}).first;
                    emailsNotInCustomer.push_back(entry.getEmail());
                }
                it->second->addGdtEntry(entry);                
            }
            */
            return std::move(gdtEntries);
        }
    }
}