#include "View.h"
#include "../logging/Logging.h"
#include "../GradidoBlockchainException.h"
#include "../main.h"
#include <lithium_metamap.hh>
#include <lithium_json.hh>

#include <math.h>
#include <sstream>

using namespace rapidjson;
using namespace li;

namespace view {

	float twoDecimals(float number) {
		return roundf(number * 100.0f) / 100.0f;
	}

	double twoDecimals(double number) {
		return round(number * 100.0) / 100.0;
	}

    OrderDirections orderDirectionFromString(const std::string& orderDirection)
	{
		if(orderDirection == "ASC") return OrderDirections::ASC;
		if(orderDirection == "DESC") return OrderDirections::DESC;
        LOG_INFORMATION("invalid order direction: " + orderDirection);
		return OrderDirections::ASC;
	}
	const char* orderDirectionsToString(OrderDirections dir)
	{
		switch(dir) {
			case OrderDirections::ASC: return "ASC";
			case OrderDirections::DESC: return "DESC";
		}
		throw GradidoUnhandledEnum("view::orderDirectionsToString", "OrderDirections", static_cast<int>(dir));
	}

    template<> Value toJson<model::GdtEntry>(const model::GdtEntry& data, Document::AllocatorType& alloc)
    {
        Value gdtEntry(kObjectType);

		gdtEntry.AddMember("id", data.getId(), alloc);
		gdtEntry.AddMember("amount", twoDecimals(data.getAmountDecimal()), alloc);
		gdtEntry.AddMember("date", Value(data.getDateString().data(), alloc), alloc);
		gdtEntry.AddMember("email", Value(data.getEmail().data(), alloc), alloc);
		gdtEntry.AddMember("comment", Value(data.getComment().data(), alloc), alloc);
		gdtEntry.AddMember("coupon_code", Value(data.getCouponCode().data(), alloc), alloc);
		gdtEntry.AddMember("gdt_entry_type_id", data.getGdtEntryTypeId(), alloc);
		gdtEntry.AddMember("factor", twoDecimals(data.getFactor()), alloc);
		gdtEntry.AddMember("amount2", twoDecimals(data.getAmount2Decimal()), alloc);
		gdtEntry.AddMember("factor2", twoDecimals(data.getFactor2()), alloc);
		gdtEntry.AddMember("gdt", twoDecimals(data.getGdt()), alloc);

		return gdtEntry;		
    }
    template<> Value toJson<model::GdtEntryList>(const model::GdtEntryList& data, Document::AllocatorType& alloc)
    {
        Value listGDTEntries(kObjectType);

		listGDTEntries.AddMember("count", data.getTotalCount(), alloc);
		Value gdtEntries(kArrayType);

		for (auto& gdtEntry : data.getGdtEntries()) {
			gdtEntries.PushBack(toJson(gdtEntry, alloc), alloc);
		}			

		listGDTEntries.AddMember("gdtEntries", gdtEntries, alloc);
		listGDTEntries.AddMember("gdtSum", twoDecimals(data.getGdtSum()), alloc);
        listGDTEntries.AddMember("state", "success", alloc);

		return listGDTEntries;
    }    

	template<> std::string toJsonString<model::GdtEntry>(const model::GdtEntry& data)
	{
		std::stringstream out;
		out.precision(2);
		out 
		   << "{" 
		   << "\"id\":" << data.getId() << ","
		   << "\"amount\":" << std::fixed << data.getAmountDecimal() << ","
		   << "\"date\":\"" << data.getDateString() << "\","
		   << "\"email\":\"" << data.getEmail() << "\","
		   << "\"comment\":\"" << data.getComment() << "\","
		   << "\"coupon_code\":\"" << data.getCouponCode() << "\","
		   << "\"gdt_entry_type_id\":" << data.getGdtEntryTypeId() << ","
		   << "\"factor\":" << std::fixed << data.getFactor() << ","
		   << "\"amount2\":" << std::fixed << data.getAmount2Decimal() << ","
		   << "\"factor2\":" << std::fixed << data.getFactor2() << ","
		   << "\"gdt\":" << std::fixed << data.getGdt() 
		   << "}"
		;
		return out.str();
	}
    template<> std::string toJsonString<model::GdtEntryList>(const model::GdtEntryList& data)
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
			out << toJsonString(gdtEntry);
			firstEntry = false;
		}				
		out << "],"
			<< "\"gdtSum\":" << std::fixed << data.getGdtSum() << ","
			<< "\"state\":\"success\","
			<< "}"
		;

		return out.str();
	}
}

