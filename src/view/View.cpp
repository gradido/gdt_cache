#include "View.h"
#include "../logging/Logging.h"
#include "../GradidoBlockchainException.h"
#include "../main.h"
#include <lithium_metamap.hh>
#include <lithium_json.hh>

#include <math.h>

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

    template<> Value toJson<model::KlicktippEntry>(const model::KlicktippEntry& data, Document::AllocatorType& alloc)
    {
        Value json(kObjectType);
        json.AddMember("email", Value(data.email.data(), alloc), alloc);
        json.AddMember("euroSum", data.euroSum, alloc);
        json.AddMember("gdtSum", data.gdtSum, alloc);
        return json;
    }
}

