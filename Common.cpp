#include "Common.h"
DWORD ConfigIni::GetValue(const std::string &section, const std::string &key, const std::string &defaultValue, const std::string &filename, std::string&outResult)const {
	char* buff = new char[buffSize];//数据量
	memset(buff, 0, buffSize);
	long char_count = GetPrivateProfileStringA(section.c_str(), key.c_str(), defaultValue.c_str(), buff, buffSize - 1, filename.c_str());
	outResult = buff;
	delete buff;
	return char_count;
}
bool ConfigIni::SetValue(const std::string & section, const std::string & key, const std::string & Value, const std::string & absoluteFilename)const {
	return WritePrivateProfileStringA(section.c_str(), key.c_str(), Value.c_str(), absoluteFilename.c_str());
}
void ConfigIni::LoadFromFile(const std::string& filename, size_t buffSize = 128) {
	this->buffSize = buffSize + 1;
	this->filename = filename;
	if (!File::Exists(filename)) {
		printf("file not Found ! Created !\n");
		std::ofstream ofs(filename, std::ios::app);
		ofs.close();
	}
}
//FileName //一定要绝对路径
ConfigIni::ConfigIni(const std::string &filename, size_t buffSize = 128) {
	LoadFromFile(filename, buffSize);
}
ConfigIni::ConfigIni() {}
//读取ini中的字符
std::string ConfigIni::ReadString(const std::string &key, const std::string& defaultValue = "", const std::string &section = "setting") const {
	std::string outResult;
	GetValue(section, key, defaultValue, filename, outResult);
	return  outResult;
}
//读取ini中的数字
float ConfigIni::ReadFloat(const std::string &key, float defaultValue = 0, const std::string &section = "setting") const {
	try
	{
		std::string outResult;
		GetValue(section, key, std::to_string(defaultValue), filename, outResult);
		return std::stof(outResult);
	}
	catch (const std::exception& ex)
	{
		printf("%s\n", ex.what());
		return defaultValue;
	}
}
//读取ini中的int数字
int ConfigIni::ReadInt(const std::string &key, int defaultValue = 0, const std::string &section = "setting") const {
	try
	{
		std::string outResult;
		GetValue(section, key, std::to_string(defaultValue), filename, outResult);
		return std::stoi(outResult);
	}
	catch (const std::exception& ex)
	{
		printf("%s\n", ex.what());
		return defaultValue;
	}
}

//写入ini
bool ConfigIni::WriteValue(const std::string &key, const std::string &value, const std::string &section = "setting")const {
	return SetValue(section, key, value, filename);
}

namespace DateTime {
	namespace Now {
		std::string ToString(const std::string &format) {
			SYSTEMTIME time;
			GetLocalTime(&time);
#define ushort WORD
			std::string  year = std::to_string((ushort)time.wYear);//年
			std::string  Month = std::to_string((ushort)time.wMonth);//月
			std::string  Day = std::to_string((ushort)time.wDay);//日
			std::string Hour = std::to_string((ushort)time.wHour);//时
			std::string Minute = std::to_string((ushort)time.wMinute);//分
			std::string Second = std::to_string((ushort)time.wSecond);//秒
			std::string wMilliseconds = std::to_string((ushort)time.wMilliseconds);//毫秒
			Month = Month.size() == 1U ? "0" + Month : Month;
			Day = Day.size() == 1U ? "0" + Day : Day;
			Hour = Hour.size() == 1U ? "0" + Hour : Hour;
			Minute = Minute.size() == 1U ? "0" + Minute : Minute;
			Second = Second.size() == 1U ? "0" + Second : Second;
			std::string formatStr = format;
			formatStr = Text::ReplaceAll(formatStr, "yyyy", year);
			formatStr = Text::ReplaceAll(formatStr, "MM", Month);
			formatStr = Text::ReplaceAll(formatStr, "dd", Day);
			formatStr = Text::ReplaceAll(formatStr, "hh", Hour);
			formatStr = Text::ReplaceAll(formatStr, "mm", Minute);
			formatStr = Text::ReplaceAll(formatStr, "ss", Second);
			formatStr = Text::ReplaceAll(formatStr, "mmmm", wMilliseconds);
			return formatStr;
		}
	}
}
namespace Common {
	template<typename T >
	T** MallocTwoArr(size_t row, size_t col) {
		T **arr = new T*[row];
		for (size_t i = 0; i < row; i++)
		{
			arr[i] = new T[col];
		}
		return arr;
	}
	template<typename T>
	void FreeTwoArr(T**arr, size_t row) {
		for (size_t i = 0; i < row; i++)
		{
			delete arr[i];
		}
		delete arr;
	}
	void Log(const std::string &text) {
		OutputDebugStringA(text.c_str());
		OutputDebugStringA("\n");
		std::string appFilename = Path::GetModuleFileName();
		std::string appName = Path::GetFileNameWithoutExtension(appFilename);
		std::string dir = Path::GetDirectoryName(appFilename) + "/" + appName + "_Log";
		std::string logFilename = dir + "/" + DateTime::Now::ToString("yyyy-MM-dd") + ".txt";
		Path::Create(dir);
		std::string logText = DateTime::Now::ToString("hh:mm:ss ") + text;
		std::ofstream file(logFilename.c_str(), std::ios::app);
		std::cout << logText << std::endl;
		file << logText.c_str() << std::endl;
		file.flush();
		file.close();
	}
	bool GetAutoBootStatus() {
		std::string bootstart = Path::GetModuleFileName();
		std::string appName = Path::GetFileNameWithoutExtension(bootstart);
		bool bResult = false;
		HKEY subKey;
		//DWORD dwError = 0;
		REGSAM dwFlag = KEY_ALL_ACCESS;
		BOOL is64Bit = FALSE;
		::IsWow64Process(::GetCurrentProcess(), &is64Bit);
		if (is64Bit)
		{
			dwFlag |= KEY_WOW64_64KEY;
		}
		if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run\\"), NULL, dwFlag, &subKey))
		{
			return bResult;
		}
		return false;
	}
	bool SetAutoBoot(bool bStatus)
	{
		std::string bootstart = Path::GetModuleFileName();
		std::string appName = Path::GetFileNameWithoutExtension(bootstart);
		bool bResult = false;
		HKEY subKey;
		//DWORD dwError = 0;
		REGSAM dwFlag = KEY_ALL_ACCESS;
		BOOL is64Bit = FALSE;
		::IsWow64Process(::GetCurrentProcess(), &is64Bit);
		if (is64Bit)
		{
			dwFlag |= KEY_WOW64_64KEY;
		}
		if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run\\"), NULL, dwFlag, &subKey))
		{
			return bResult;
		}
		if (bStatus)
		{
			TCHAR szName[MAX_PATH] = { 0 };
			DWORD dwSize = MAX_PATH;
			::GetModuleFileName(NULL, szName, MAX_PATH);
			if (ERROR_SUCCESS == ::RegSetValueExW(subKey, Text::ANSIToUniCode(appName).c_str(), NULL, REG_SZ, (PBYTE)&szName, dwSize))
			{
				bResult = true;
			}
		}
		else
		{
			if (ERROR_SUCCESS == ::RegDeleteValueW(subKey, Text::ANSIToUniCode(appName).c_str()))
			{
				bResult = true;
			}
			//dwError = ::GetLastError();
		}
		::RegCloseKey(subKey);
		return bResult;
	}
}
