#include "SafeConfig.h"
SafeConfig::SafeConfig(const Text::String& fileName)
{
	this->fileName = fileName;
	if (!File::Exists(fileName)) {
		File::Create(fileName);
	}
}
Text::String SafeConfig::ReadValue(const Text::String& key, const Text::String& defaultValue)
{
	File::FileStream fileStream;
	File::ReadFile(fileName, &fileStream);
	WinTool::DeCode(&fileStream, &fileStream);
	Text::String u8Str = fileStream;
	auto list = u8Str.Split("&");
	for (auto& it : list) {
		size_t eq_ = it.find("=");
		if (eq_ != -1) {
			Text::String key_ = it.substr(0, eq_);
			Text::String value_ = it.substr(eq_ + 1);
			if (key == key_) {
				return value_;
			}
		}
	}
	return defaultValue;
}

Text::String SafeConfig::ReadString(const Text::String& key, const Text::String& defaultValue) {

	return ReadValue(key, defaultValue);
}
int SafeConfig::ReadInt(const Text::String& key, int defaultValue) {
	auto result = ReadValue(key, std::to_string(defaultValue));
	return std::atoi(result.c_str());
}

void SafeConfig::WriteValue(const Text::String& key, const Text::String& value)
{
	File::FileStream fileStream;
	File::ReadFile(fileName, &fileStream);
	WinTool::DeCode(&fileStream, &fileStream);
	Text::String u8Str = fileStream;
	std::map<Text::String, Text::String> values;
	auto list = u8Str.Split("&");
	for (auto& it : list) {
		size_t eq_ = it.find("=");
		if (eq_ != -1) {
			Text::String key_ = it.substr(0, eq_);
			Text::String value_ = it.substr(eq_ + 1);
			values.insert(std::pair<Text::String, Text::String>(key_, value));
		}
	}
	Text::String saveValues;
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
	//加密并写入
	WinTool::EnCode(&saveValues, &fileStream);
	File::WriteFile(&fileStream, this->fileName);
}
