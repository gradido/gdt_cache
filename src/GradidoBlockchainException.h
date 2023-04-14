#ifndef __GRADIDO_BLOCKCHAIN_EXCEPTION_H
#define __GRADIDO_BLOCKCHAIN_EXCEPTION_H

#include <stdexcept>
#include <rapidjson/error/error.h>

#include "rapidjson/document.h"

class GradidoBlockchainException : public std::runtime_error
{
public:
	explicit GradidoBlockchainException(const char* what) : std::runtime_error(what) {}
	virtual ~GradidoBlockchainException() {};
	virtual std::string getFullString() const = 0;
	virtual rapidjson::Value getDetails(rapidjson::Document::AllocatorType& alloc) const { return rapidjson::Value(rapidjson::kObjectType); }
};


class RapidjsonParseErrorException : public GradidoBlockchainException
{
public:
	explicit RapidjsonParseErrorException(const char* what, rapidjson::ParseErrorCode parseErrorCode, size_t parseErrorOffset) noexcept;

	RapidjsonParseErrorException& setRawText(const std::string& rawText);
	inline const std::string& getRawText() { return mRawText; }
	std::string getFullString() const;
	rapidjson::Value getDetails(rapidjson::Document::AllocatorType& alloc) const;

protected:
	rapidjson::ParseErrorCode mParseErrorCode;
	size_t					  mParseErrorOffset;
	std::string				  mRawText;
};

class RapidjsonMissingMemberException : public GradidoBlockchainException
{
public: 
	explicit RapidjsonMissingMemberException(const char* what, const char* fieldName, const char* fieldType) noexcept;

	std::string getFullString() const;
protected:
	std::string mFieldName;
	std::string mFieldType;

};

class RapidjsonInvalidMemberException : public RapidjsonMissingMemberException
{
public:
	explicit RapidjsonInvalidMemberException(const char* what, const char* fieldName, const char* fieldType) noexcept :
		RapidjsonMissingMemberException(what, fieldName, fieldType) {}
};

class GradidoInvalidEnumException : public GradidoBlockchainException
{
public:
	explicit GradidoInvalidEnumException(const char* what, const std::string& enumString) noexcept;
	std::string getFullString() const;

protected:
	std::string mEnumString;
};

class GradidoUnknownEnumException : public GradidoBlockchainException
{
public:
	explicit GradidoUnknownEnumException(const char* what, const char* enumName, int value) noexcept;
	std::string getFullString() const;

protected:
	std::string mEnumName;
	int mValue;
};

class GradidoUnhandledEnum : public GradidoUnknownEnumException
{
public:
	explicit GradidoUnhandledEnum(const char* what, const char* enumName, int value) noexcept
		: GradidoUnknownEnumException(what, enumName, value) {}
};

class GradidoInvalidBase64Exception : public GradidoBlockchainException
{
public:
	explicit GradidoInvalidBase64Exception(const char* what, const std::string& base64, int lastValidCharacter) noexcept;
	std::string getFullString() const;
protected:
	std::string mBase64;
	int         mLastValidCharacter;

};

class GradidoNullPointerException : public GradidoBlockchainException
{
public: 
	explicit GradidoNullPointerException(const char* what, const char* typeName, const char* functionName) noexcept;
	std::string getFullString() const;

protected:
	std::string mTypeName;
	std::string mFunctionName;

};

class GradidoNodeInvalidDataException : public GradidoBlockchainException
{
public: 
	explicit GradidoNodeInvalidDataException(const char* what) noexcept :GradidoBlockchainException(what) {};
	std::string getFullString() const { return what(); }
};

#endif //__GRADIDO_BLOCKCHAIN_EXCEPTION_H