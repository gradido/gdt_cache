#include "GradidoBlockchainException.h"
#include "rapidjson/error/en.h"

#include <string>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"

using namespace rapidjson;


// ################################ RapidJson Parse Error Exception ###############################################################

RapidjsonParseErrorException::RapidjsonParseErrorException(const char* what, ParseErrorCode parseErrorCode, size_t parseErrorOffset) noexcept
	: GradidoBlockchainException(what), mParseErrorCode(parseErrorCode), mParseErrorOffset(parseErrorOffset)
{

}

RapidjsonParseErrorException& RapidjsonParseErrorException::setRawText(const std::string& rawText)
{
	mRawText = rawText;
	return *this;
}
std::string RapidjsonParseErrorException::getFullString() const
{
	std::string resultString;
	auto parseErrorCodeString = GetParseError_En(mParseErrorCode);
	std::string parseErrorOffsetString = std::to_string(mParseErrorOffset);

	size_t resultSize = strlen(what()) + strlen(parseErrorCodeString) + 15 + parseErrorOffsetString.size() + 14 + 2; 
	if (mRawText.size()) {
		resultSize += mRawText.size() + 7;
	}
	resultString = what();
	resultString += ", parse error: ";
	resultString += parseErrorCodeString;
	resultString += " on position: " + parseErrorOffsetString;
	if (mRawText.size()) {
		resultString += ", src: " + mRawText;
	}
	return resultString;
}

Value RapidjsonParseErrorException::getDetails(Document::AllocatorType& alloc) const
{
	Value details(kObjectType);
	details.AddMember("what", Value(what(), alloc), alloc);
	details.AddMember("parseErrorCode", Value(GetParseError_En(mParseErrorCode), alloc), alloc);
	details.AddMember("parseErrorPosition", mParseErrorOffset, alloc);
	if (mRawText.size()) {
		details.AddMember("src", Value(mRawText.data(), alloc), alloc);
	}
	
	return std::move(details);
}

// ************************** Missing Member in Rapidjson Object ********************
RapidjsonMissingMemberException::RapidjsonMissingMemberException(const char* what, const char* fieldName, const char* fieldType) noexcept
	: GradidoBlockchainException(what), mFieldName(fieldName), mFieldType(fieldType)
{

}

std::string RapidjsonMissingMemberException::getFullString() const
{
	std::string result;
	result = what();
	if (mFieldName.size()) {
		result += ", field name: " + mFieldName;
	}
	if (mFieldType.size()) {
		result += ", field type: " + mFieldType;
	}
	return result;
}

// *************************** Invalid Enum Exception *****************************
GradidoInvalidEnumException::GradidoInvalidEnumException(const char* what, const std::string& enumString) noexcept
	: GradidoBlockchainException(what), mEnumString(enumString)
{

}

std::string GradidoInvalidEnumException::getFullString() const
{
	std::string resultString;
	size_t resultSize = strlen(what()) + mEnumString.size() + 10;
	resultString.reserve(resultSize);
	resultString = what();
	resultString += ", with: " + mEnumString;
	return resultString;
}

// ***************** it an enuam value is unknown by code ***************************
GradidoUnknownEnumException::GradidoUnknownEnumException(const char* what, const char* enumName, int value) noexcept
	: GradidoBlockchainException(what), mEnumName(enumName), mValue(value)
{

}

std::string GradidoUnknownEnumException::getFullString() const
{
	std::string resultString;
	auto valueString = std::to_string(mValue);
	size_t resultSize = strlen(what()) + mEnumName.size() + valueString.size() + 2 + 13 + 9;
	resultString.reserve(resultSize);
	resultString = what();
	resultString += ", enum name: " + mEnumName;
	resultString += ", value: " + valueString;
	return resultString;
}

// **************************** Invalid Base64 ****************************************
GradidoInvalidBase64Exception::GradidoInvalidBase64Exception(const char* what, const std::string& base64, int lastValidCharacter) noexcept
	: GradidoBlockchainException(what), mBase64(base64), mLastValidCharacter(lastValidCharacter)
{

}

std::string GradidoInvalidBase64Exception::getFullString() const
{
	std::string resultString;
	std::string lastValidCharacterString = std::to_string(mLastValidCharacter);
	size_t resultSize = strlen(what()) + mBase64.size() + 10 + lastValidCharacterString.size() + 19;
	resultString.reserve(resultSize);
	resultString = what();
	resultString += ", with: " + mBase64 + ", last valid char: " + lastValidCharacterString;
	return resultString;
}

// ****************************** Null Pointer Exception **********************************************
GradidoNullPointerException::GradidoNullPointerException(const char* what, const char* typeName, const char* functionName) noexcept
	: GradidoBlockchainException(what), mTypeName(typeName), mFunctionName(functionName)
{

}

std::string GradidoNullPointerException::getFullString() const
{
	std::string resulString = what();
	resulString += ", type name: " + mTypeName + ", function name: " + mFunctionName;
	return resulString;
}