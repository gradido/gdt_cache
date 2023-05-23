#include "View.h"
#include "../logging/Logging.h"
#include "../GradidoBlockchainException.h"
#include "../main.h"

#include <math.h>
#include <sstream>

using namespace li;

namespace view {

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
