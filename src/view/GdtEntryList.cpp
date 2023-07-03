#include "GdtEntryList.h"
#include "View.h"


namespace view {
    namespace GdtEntryList {     

        std::string toJsonString(const model::GdtEntryList& data, Profiler timeUsed)
        {
            std::stringstream out;
            out << "{"
                << "\"count\":" << data.getTotalCount() << ","
                << "\"gdtEntries\":[";
            bool firstEntry = true;
            for (auto& gdtEntry : data.getGdtEntries()) {
                if(!firstEntry) out << ",";
                out << view::toJsonString(gdtEntry);
                firstEntry = false;
            }				
            out << "],"
                << "\"gdtSum\":" << stringWithoutTrailingZeros(data.getGdtSum()) << ","
                << "\"state\":\"success\","
                << "\"timeUsed\":" << timeUsed.seconds()
                << "}"
            ;

            return out.str();
        }

        std::string toJsonString(
            const model::GdtEntryList& data,
			Profiler timeUsed, 
			int page, 
			int count, 
			OrderDirections order
		)
        {
            std::stringstream out;
            out << "{"
                << "\"state\":\"success\","
                << "\"count\":" << data.getTotalCount() << ","
                << "\"gdtEntries\":["
                ;
            
            auto gdtEntriesList = data.getGdtEntries();

            // only go into loop if enough gdt entries exist for return at least one
            if(gdtEntriesList.size() > (page-1)*count) 
            {
                // start at the beginning of list (smallest date) by order == ASC
                auto it = gdtEntriesList.begin();
                // start at the end of list (largest date) by order == DESC
                if(order == OrderDirections::DESC) {
                    it = gdtEntriesList.end();
                    // end is actually after the last entry so we need to move on step back
                    it--;
                }
                
                // move iterator to position where to start collecting gdt entries
                // if client request at least page 2
                if(page > 1) {					
                    // for every entry to skip
                    for(int i = 0; i < (page-1) * count; i++) {
                        if(order == OrderDirections::ASC) {
                            // increment iterator by order == ASC
                            it++;
                        } else {
                            // decrement iterator by order == DESC
                            it--;
                        }
                    }					
                }
                // collect max count gdt entries
                for(int i = 0; i < count; i++) 
                {
                    if(i) {
                        out << ",";
                    }
                    out << view::toJsonString(*it);
                    if(order == OrderDirections::ASC) {
                        // increment iterator by order == ASC
                        it++;
                        // end is already past the last entry so we exit here
                        if(it == gdtEntriesList.end()) break;
                    } else {
                        // begin is the first entry, we cannot further decrement here
                        if(it == gdtEntriesList.begin()) break;
                        // decrement iterator by order == DESC
                        it--;					
                    }
                }
            }

            out << "],\"gdtSum\":"
                << stringWithoutTrailingZeros(data.getGdtSum()) << ","
                << "\"timeUsed\":" << timeUsed.seconds()
                << "}"
                ;

            return out.str();
        }

    }
}