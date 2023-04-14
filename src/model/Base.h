#ifndef __GRADIDO_DESKTOP_MODEL_GRAPHQL_BASE_H
#define __GRADIDO_DESKTOP_MODEL_GRAPHQL_BASE_H

#include "rapidjson/document.h"

namespace model {
	
	class Base
	{
	public:
		virtual ~Base() {};
		virtual rapidjson::Value toJson(rapidjson::Document::AllocatorType& alloc) = 0;
		virtual const char* getTypename() = 0;

	protected:
	};
}

#endif //__GRADIDO_DESKTOP_MODEL_GRAPHQL_BASE_H