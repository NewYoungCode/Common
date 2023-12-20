#include "ConfigIni.h"
DWORD ConfigIni::GetValue(const Text::Utf8String& section, const Text::Utf8String& key, const Text::Utf8String& defaultValue, const Text::Utf8String& filename, Text::Utf8String& outResult)const {
	Text::Utf8String _section = section;
	if (section.empty()) {
		_section = this->section;
	}
	WCHAR* buff = new WCHAR[buffSize]{ 0 };//数据量
	long char_count = ::GetPrivateProfileStringW(_section.unicode().c_str(), key.unicode().c_str(), defaultValue.unicode().c_str(), buff, buffSize - 1, filename.unicode().c_str());
	outResult = buff;
	delete[] buff;
	return char_count;
}
bool ConfigIni::SetValue(const Text::Utf8String& section, const Text::Utf8String& key, const Text::Utf8String& Value, const Text::Utf8String& absoluteFilename)const {
	if (!File::Exists(filename)) {
		File::Create(filename);
	}
	Text::Utf8String _section = section;
	if (section.empty()) {
		_section = this->section;
	}
	return ::WritePrivateProfileStringW(_section.unicode().c_str(), key.unicode().c_str(), Value.unicode().c_str(), absoluteFilename.unicode().c_str()) == 0 ? false : true;
}

//FileName //一定要绝对路径
ConfigIni::ConfigIni(const Text::Utf8String& filename, const Text::Utf8String& defaultSection, size_t buffSize) {
	this->buffSize = buffSize;
	this->filename = filename;
	this->section = defaultSection;
}

void ConfigIni::SetDefaultSection(const Text::Utf8String section) {
	this->section = section;
}


//读取ini中的字符
Text::Utf8String  ConfigIni::ReadString(const Text::Utf8String& key, const Text::Utf8String& defaultValue, const Text::Utf8String& section) const {
	Text::Utf8String outResult;
	GetValue(section, key, defaultValue, filename, outResult);
	return  outResult;
}
//读取ini中的数字
float  ConfigIni::ReadFloat(const Text::Utf8String& key, float defaultValue, const Text::Utf8String& section) const {
	try
	{
		Text::Utf8String outResult;
		GetValue(section, key, std::to_wstring(defaultValue), filename, outResult);
		return std::stof(outResult);
	}
	catch (const std::exception& ex)
	{
		printf("%s\n", ex.what());
		return defaultValue;
	}
}
//读取ini中的int数字
int  ConfigIni::ReadInt(const Text::Utf8String& key, int defaultValue, const Text::Utf8String& section) const {
	try
	{
		Text::Utf8String outResult;
		GetValue(section, key, std::to_wstring(defaultValue), filename, outResult);
		return std::stoi(outResult);
	}
	catch (const std::exception& ex)
	{
		printf("%s\n", ex.what());
		return defaultValue;
	}
}

//写入ini
bool  ConfigIni::WriteValue(const Text::Utf8String& key, const Text::Utf8String& value, const Text::Utf8String& section)const {
	return SetValue(section, key, value, filename);
}

std::vector<Text::Utf8String>  ConfigIni::GetSections() {
	std::vector<Text::Utf8String> list;

	size_t maxSize = 1024 * 1024 * 20;//20M
	WCHAR* chSectionNames = new WCHAR[maxSize]{ 0 };
	GetPrivateProfileSectionNamesW(chSectionNames, maxSize, filename.unicode().c_str());
	size_t pos = 0;
	do
	{
		wchar_t buf[128]{ 0 };
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

void  ConfigIni::DeleteSection(const Text::Utf8String& section) {
	Text::Utf8String outData;
	File::ReadFile(filename, &outData);
	Text::Utf8String id = "[" + section + "]";
	auto pos = outData.find(id);
	if (pos == -1) {
		return;
	}
	size_t count = 0;
	auto pos2 = outData.find("\r\n[", pos);
	count = pos2 - pos;
	if (pos2 != -1) {
		outData.erase(pos, count);
	}
	else {
		outData = outData.substr(0, pos);
	}
	outData = outData.Replace("\r\n\r\n", "\r\n");
	std::ofstream ofs(filename, std::ios::binary | std::ios::app);
	ofs.seekp(0);
	ofs.write(outData.c_str(), outData.size());
	ofs.flush();
	ofs.close();
	return;
}
//
//SafeConfigIni::SafeConfigIni(const Text::Utf8String & filename, const Text::Utf8String & defaultSection, bool create, size_t buffSize)
//{
//	//oldFilename = filename;
//
//	//TCHAR buf[256]{ 0 };
//	//::GetTempPath(255, buf);
//	//Text::Utf8String tempFile = buf + Path::GetFileNameWithoutExtension(Path::StartFileName()) + std::to_string(time(NULL));
//	//File::Delete(tempFile);
//	//DeCode(oldFilename, tempFile);
//
//	//this->buffSize = buffSize;
//	//this->filename = tempFile;
//	//this->section = defaultSection;
//}
