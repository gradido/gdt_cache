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
		// without scientific notation
		stream.setf(std::ios_base::fixed, std::ios_base::floatfield);
		// without trailing zeros
		stream.unsetf(std::ios_base::showpoint);		
		// maximal 2 decimal places
		stream.precision(2);
	}

	std::string stringWithoutTrailingZeros(double number)
	{	
		// use mutex because we have only one buffer, and this function can be called from multiple 
		// threads at once
		static std::mutex mutex;
		std::lock_guard _lock(mutex);

		// use string stream buffer for faster string conversion
		// http://cplusplus.bordoon.com/speeding_up_string_conversions.html
		static std::stringstream buffer;
		static bool firstTime = true;		

		if(!firstTime) {
			buffer.seekp(std::ios::beg); 
		} else {
			firstTime = false;
		}
		configNumberFormat(buffer);
		buffer << number << '\0';
		std::string result = buffer.str().c_str();  

		// code from chatGPT 
		// Remove trailing zeros
		size_t pos = result.find_last_not_of('0');
		if (pos != std::string::npos) {
			result.erase(pos + 1);
		}

		// Remove decimal point if no decimal places remaining
		if (result.back() == '.') {
			result.pop_back();
		}
		return std::move(result);
	}

	std::string stringWithoutTrailingZeros(float number)
	{
		return stringWithoutTrailingZeros(static_cast<double>(number));
	}

	template<> std::string toJsonString<model::GdtEntry>(const model::GdtEntry& data)
	{
		std::stringstream out;
		out 
		   << "{" 
		   << "\"id\":" << data.getId() << ","
		   << "\"amount\":" << stringWithoutTrailingZeros(data.getAmountDecimal()) << ","
		   << "\"date\":\"" << data.getDateString() << "\","
		   << "\"email\":\"" << data.getEmail() << "\","
		   << "\"comment\":\"" << data.getComment() << "\","
		   << "\"coupon_code\":\"" << data.getCouponCode() << "\","
		   << "\"gdt_entry_type_id\":" << data.getGdtEntryTypeId() << ","
		   << "\"factor\":" << stringWithoutTrailingZeros(data.getFactor()) << ","
		   << "\"amount2\":" << stringWithoutTrailingZeros(data.getAmount2Decimal()) << ","
		   << "\"factor2\":" << stringWithoutTrailingZeros(data.getFactor2()) << ","
		   << "\"gdt\":" << stringWithoutTrailingZeros(data.getGdt()) 
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
			<< "\"gdtSum\":" << stringWithoutTrailingZeros(data.getGdtSum()) << ","
			<< "\"state\":\"success\","
			<< "}"
		;

		return out.str();
	}
}

