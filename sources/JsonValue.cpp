#include "JsonValue.h"
#if USEJSONCPP
//JObject::JObject(const Json::Value&right) {
//	Json::Value temp(right);
//	swap(temp);
//}
JsonValue& JsonValue:: operator = (const Json::Value &other)
{
	Value temp(other);
	swap(temp);
	return *this;
}

bool JsonValue::IsJson() {
	return b;
}

JsonValue::JsonValue(const std::string&jsonStr) {
	if (jsonStr.empty())return;
	 this->b=rd.parse(jsonStr, *this);
}
#endif