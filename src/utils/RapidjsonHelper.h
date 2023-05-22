#ifndef __GRADIDO_BLOCKCHAIN_LIB_RAPIDJSON_HELPER_H
#define __GRADIDO_BLOCKCHAIN_LIB_RAPIDJSON_HELPER_H

#include "rapidjson/document.h"

namespace rapidjson_helper
{
	enum class MemberType : uint8_t {
		STRING,
		INTEGER,
		FLOAT,
		NUMBER,
		DATETIME,
		ARRAY
	};

	const char* getMemberTypeString(MemberType type);

	//! throw exception if member is missing or wrong type
	//! \param objectName will included in exception
	void checkMember(const rapidjson::Value& obj, const char* memberName, MemberType type, const char* objectName = nullptr);

	std::string jsonToString(const rapidjson::Document& json);
}

#endif //__GRADIDO_BLOCKCHAIN_LIB_RAPIDJSON_HELPER_H