#include "GdtEntryList.h"
#include "View.h"

using namespace rapidjson;

namespace view {
    namespace GdtEntryList {

        Value toJson(const model::GdtEntryList& data, Document::AllocatorType& alloc, Profiler timeUsed)
        {
            auto result = view::toJson(data, alloc);
            
            result.AddMember("state", "success", alloc);
            result.AddMember("timeUsed", timeUsed.seconds(), alloc);

            return result;
        }

        std::string toJsonString(const model::GdtEntryList& data, Profiler timeUsed)
        {
            std::stringstream out;
            out.precision(2);
            out 
                << "{"
                << "\"count\":" << data.getTotalCount() << ","
                << "\"gdtEntries\":[";
            bool firstEntry = true;
            for (auto& gdtEntry : data.getGdtEntries()) {
                if(!firstEntry) out << ",";
                out << view::toJsonString(gdtEntry);
                firstEntry = false;
            }				
            out << "],"
                << "\"gdtSum\":" << std::fixed << data.getGdtSum() << ","
                << "\"state\":\"success\","
                << "\"timeUsed\":" << timeUsed.seconds() << ","
                << "}"
            ;

            return out.str();
        }

        Value toJson(
            const model::GdtEntryList& data,
            Document::AllocatorType& alloc, 
            Profiler timeUsed, 
            int page, 
            int count, 
            OrderDirections order
        )
        {
            Value listGDTEntries(kObjectType);

            listGDTEntries.AddMember("count", data.getTotalCount(), alloc);
            Value gdtEntries(kArrayType);
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
                    gdtEntries.PushBack(view::toJson(*it, alloc), alloc);	
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

            listGDTEntries.AddMember("gdtEntries", gdtEntries, alloc);
            listGDTEntries.AddMember("gdtSum", data.getGdtSum(), alloc);
            listGDTEntries.AddMember("state", "success", alloc);
            listGDTEntries.AddMember("timeUsed", static_cast<float>(timeUsed.seconds()), alloc);

            return listGDTEntries;
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
            out.precision(2);
            out << "{"
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

            out 
                << "\"gdtSum\":" << std::fixed << data.getGdtSum() << ","
                << "\"state\":\"success\","
                << "\"timeUsed\":" << timeUsed.seconds()
                << "}";

            return out.str();
        }

    }
}