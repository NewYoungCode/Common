#include <iostream>
#include "ConfigIni.h"
#include "FileSystem.h"

//读取ini中的字符
Text::String ConfigIni::ReadString(const Text::String& key, const Text::String& defaultValue) {
	Text::String outResult;
	GetValue(key, filename, outResult);
	if (outResult.empty()) {
		outResult = defaultValue;
		SetValue(key, outResult, filename);
	}
	return outResult;
}
//读取ini中的数字
float ConfigIni::ReadFloat(const Text::String& key, float defaultValue) {
	try
	{
		Text::String outResult;
		GetValue(key, filename, outResult);
		if (outResult.empty()) {
			outResult = std::to_string(defaultValue);
			WriteString(key, outResult);
		}
		return std::stoi(outResult);
	}
	catch (const std::exception& ex)
	{
		printf("%s\n", ex.what());
		return defaultValue;
	}
}
//读取ini中的int数字
int  ConfigIni::ReadInt(const Text::String& key, int defaultValue) {
	try
	{
		Text::String outResult;
		GetValue(key, filename, outResult);
		if (outResult.empty()) {
			outResult = std::to_string(defaultValue);
			WriteString(key, outResult);
		}
		return std::stoi(outResult);
	}
	catch (const std::exception& ex)
	{
		printf("%s\n", ex.what());
		return defaultValue;
	}
}
bool ConfigIni::ReadBool(const Text::String& key, bool defaultValue)
{
	Text::String outResult;
	GetValue(key, filename, outResult);
	if (outResult.empty()) {
		outResult = defaultValue ? "true" : "false";
		WriteString(key, outResult);
	}
	return outResult == "true" ? true : false;
}


//写入ini

bool ConfigIni::WriteString(const Text::String& key, const Text::String& value) {
	return SetValue(key, value, filename);
}
bool ConfigIni::WriteFloat(const Text::String& key, float value)
{
	return WriteString(key, std::to_string(value));
}
bool ConfigIni::WriteInt(const Text::String& key, int value)
{
	return WriteString(key, std::to_string(value));
}
bool ConfigIni::WriteBool(const Text::String& key, bool value)
{
	return WriteString(key, Text::String(value ? "true" : "false"));
}


//其他操作

DWORD ConfigIni::GetValue(const Text::String& key, const Text::String& filename, Text::String& outResult) {
	WCHAR* buff = new WCHAR[buffSize]{ 0 };//数据量
	long char_count = ::GetPrivateProfileStringW(section.unicode().c_str(), key.unicode().c_str(), NULL, buff, buffSize - 1, filename.unicode().c_str());
	outResult = buff;
	delete[] buff;
	return char_count;
}
bool ConfigIni::SetValue(const Text::String& key, const Text::String& Value, const Text::String& absoluteFilename) {
	if (!File::Exists(filename)) {
		File::Create(filename);
	}
	return ::WritePrivateProfileStringW(section.unicode().c_str(), key.unicode().c_str(), Value.unicode().c_str(), absoluteFilename.unicode().c_str()) == 0 ? false : true;
}
ConfigIni::ConfigIni(const Text::String& filename, const Text::String& defaultSection, size_t buffSize) {
	this->buffSize = buffSize;
	//非绝对路径
	if (filename.find(":") != 1) {
		this->filename = Path::StartPath() + "/" + filename;
	}
	else {
		this->filename = filename;
	}
	//不存在则创建
	if (!File::Exists(this->filename)) {
		File::Create(this->filename);
	}
	this->section = defaultSection;
}
void ConfigIni::SetSection(const Text::String& section) {
	this->section = section;
}
std::vector<Text::String>  ConfigIni::GetSections() {
	std::vector<Text::String> list;

	ULONGLONG fSize = File::GetFileSize(filename) + 2;
	WCHAR* chSectionNames = new WCHAR[fSize]{ 0 };
	GetPrivateProfileSectionNamesW(chSectionNames, fSize, filename.unicode().c_str());
	size_t pos = 0;
	do
	{
		wchar_t buf[1024]{ 0 };
		::lstrcpyW(buf, chSectionNames + pos);
		size_t len = lstrlenW(buf);
		if (len == 0) {
			break;
		}
		list.push_back(buf);
		pos += len + 1;
	} while (true);
	delete chSectionNames;
	return list;
}
void ConfigIni::DeleteSection(const Text::String& section) {
	Text::String outData;
	File::ReadFile(filename, &outData);
	Text::String id = "[" + section + "]";
	auto pos = outData.find(id);
	if (pos == size_t(-1)) {
		return;
	}
	size_t count = 0;
	auto pos2 = outData.find("\r\n[", pos);
	count = pos2 - pos;
	if (pos2 != size_t(-1)) {
		outData.erase(pos, count);
	}
	else {
		outData = outData.substr(0, pos);
	}
	outData = outData.replace("\r\n\r\n", "\r\n", true);
	std::ofstream ofs(filename.unicode(), std::ios::binary);
	ofs.seekp(0);
	ofs.write(outData.c_str(), outData.size());
	ofs.flush();
	ofs.close();
	return;
}
