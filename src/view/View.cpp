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

	void configNumberFormat(std::ios& stream)
	{
		// maximal 2 decimal places
		stream.precision(2);
		// without trailing zeros
		stream.unsetf(std::ios_base::showpoint);
		// without scientific notation
		stream.unsetf(std::ios_base::floatfield);
	}

	template<> std::string toJsonString<model::GdtEntry>(const model::GdtEntry& data)
	{
		std::stringstream out;
		configNumberFormat(out);
		out 
		   << "{" 
		   << "\"id\":" << data.getId() << ","
		   << "\"amount\":" << data.getAmountDecimal() << ","
		   << "\"date\":\"" << data.getDateString() << "\","
		   << "\"email\":\"" << data.getEmail() << "\","
		   << "\"comment\":\"" << data.getComment() << "\","
		   << "\"coupon_code\":\"" << data.getCouponCode() << "\","
		   << "\"gdt_entry_type_id\":" << data.getGdtEntryTypeId() << ","
		   << "\"factor\":" << data.getFactor() << ","
		   << "\"amount2\":" << data.getAmount2Decimal() << ","
		   << "\"factor2\":" << data.getFactor2() << ","
		   << "\"gdt\":" << data.getGdt() 
		   << "}"
		;
		return out.str();
	}
    template<> std::string toJsonString<model::GdtEntryList>(const model::GdtEntryList& data)
	{
		std::stringstream out;
		configNumberFormat(out);
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
			<< "\"gdtSum\":" << data.getGdtSum() << ","
			<< "\"state\":\"success\","
			<< "}"
		;

		return out.str();
	}
}

