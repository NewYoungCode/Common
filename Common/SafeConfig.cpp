#include "SafeConfig.h"
SafeConfig::SafeConfig(const Text::Utf8String& fileName)
{
	this->fileName = fileName;
	if (!File::Exists(fileName)) {
		File::Create(fileName);
	}
}
Text::Utf8String SafeConfig::ReadValue(const Text::Utf8String& key, const Text::Utf8String& defaultValue)
{
	File::FileStream fileStream;
	File::ReadFile(fileName, &fileStream);
	WinTool::DeCode(&fileStream, &fileStream);
	Text::Utf8String u8Str = fileStream;
	auto list = u8Str.Split("&");
	for (auto& it : list) {
		size_t eq_ = it.find("=");
		if (eq_ != -1) {
			Text::Utf8String key_ = it.substr(0, eq_);
			Text::Utf8String value_ = it.substr(eq_ + 1);
			if (key == key_) {
				return value_;
			}
		}
	}
	return defaultValue;
}

Text::Utf8String SafeConfig::ReadString(const Text::Utf8String& key, const Text::Utf8String& defaultValue) {

	return ReadValue(key, defaultValue);
}
int SafeConfig::ReadInt(const Text::Utf8String& key, int defaultValue) {
	auto result = ReadValue(key, std::to_string(defaultValue));
	return std::atoi(result.c_str());
}

void SafeConfig::WriteValue(const Text::Utf8String& key, const Text::Utf8String& value)
{
	File::FileStream fileStream;
	File::ReadFile(fileName, &fileStream);
	WinTool::DeCode(&fileStream, &fileStream);
	Text::Utf8String u8Str = fileStream;
	std::map<Text::Utf8String, Text::Utf8String> values;
	auto list = u8Str.Split("&");
	for (auto& it : list) {
		size_t eq_ = it.find("=");
		if (eq_ != -1) {
			Text::Utf8String key_ = it.substr(0, eq_);
			Text::Utf8String value_ = it.substr(eq_ + 1);
			values.insert(std::pair<Text::Utf8String, Text::Utf8String>(key_, value));
		}
	}
	Text::Utf8String saveValues;
	bool first = true;
	bool find = false;
	for (auto& it : values) {
		if (key == it.first) {
			it.second = value;
			find = true;
		}
		if (first) {
			saveValues += it.first + "=" + it.second;
			first = false;
		}
		else {
			saveValues += "&" + it.first + "=" + it.second;
		}
	}
	if (find == false) {
		if (saveValues.size() == 0) {
			saveValues += key + "=" + value;
		}
		else {
			saveValues += "&" + key + "=" + value;
		}
	}
	//¼ÓÃÜ²¢Ð´Èë
	WinTool::EnCode(&saveValues, &fileStream);
	File::WriteFile(&fileStream, this->fileName);
}
