#include "GdtEntry.h"
#include "../logging/Logging.h"

namespace mysql {
    namespace GdtEntry {

        const std::string GDT_ENTRY_SELECT_STRING("select id, amount, UNIX_TIMESTAMP(date), LOWER(TRIM(email)), IFNULL(comment, ''), \
                IFNULL(source, ''), IFNULL(project, ''), IFNULL(coupon_code, ''), \
                gdt_entry_type_id, factor, amount2, factor2 from gdt_entries "
        );

        model::GdtEntryListPtr getByCustomer(
            model::CustomerPtr customer,
            li::mysql_connection<li::mysql_functions_blocking> connection
        )
        {
            auto gdtEntries = std::make_shared<model::GdtEntryList>();

            auto prepared = connection.prepare(GDT_ENTRY_SELECT_STRING + "where email IN (?) order by date ASC");
            
            auto rows = prepared(customer->getEmailsString());
            while (auto row = rows.read_optional<int, long long, int, std::string, std::string, std::string, std::string, std::string, int, float, int, float>())
            {
                gdtEntries->addGdtEntry(model::GdtEntry(row.value()));
            }
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
            while (auto row = rows.read_optional<int, long long, int, std::string, std::string, std::string, std::string, std::string, int, float, int, float>())
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
            return std::move(gdtEntries);
        }
    }
}