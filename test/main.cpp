#include "WebClient.h"
#include "FileSystem.h"
#include "Text.h"
#include <sstream>

class Res {
public:
	struct ResEntry {
		DWORD offset = 0;
		DWORD size = 0;
		std::string name;
	};
private:
	//编码转换
	static void UnicodeToUtf8(const std::wstring& wstr, std::string* utf8Str) {
		int bytes = ::WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
		utf8Str->resize(bytes);
		::WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.size(), const_cast<char*>(utf8Str->c_str()), utf8Str->size(), NULL, NULL);
	}
	//寻找指定目录以及目录下的所有文件
	static  void FindFilesRecursively(const std::wstring& path, std::list<std::wstring>* result) {
		WIN32_FIND_DATAW findData;
		HANDLE findHandle = FindFirstFileW((path + L"/*").c_str(), &findData);
		if (findHandle == INVALID_HANDLE_VALUE) {
			return;
		}
		do
		{
			if (findData.cFileName[0] == L'.') {
				continue;
			}
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				// 如果是目录，递归查找该目录  
				std::wstring newPath = path + L"/" + findData.cFileName;
				FindFilesRecursively(newPath, result);
			}
			else {
				std::wstring newPath = path + L"/" + findData.cFileName;
				result->push_back(newPath);
			}
		} while (FindNextFileW(findHandle, &findData));
		FindClose(findHandle);
	}
public:
	static void Package(const std::wstring& dir, const std::wstring& outFile) {
		std::list<ResEntry> items;
		std::list<std::wstring> files;
		FindFilesRecursively(dir, &files);
		DWORD headOffset = sizeof(DWORD);
		std::ofstream ofs(outFile, std::ios::binary);
		ofs.seekp(headOffset);
		for (auto& file : files) {
			//读取文件大小
			std::ifstream ifs(file, std::ios::binary);
			ifs.seekg(0, std::ios::end);
			auto size = ifs.tellg();
			//读取文件
			std::string data;
			data.resize(size);
			ifs.seekg(0);
			ifs.read((char*)data.c_str(), size);
			//记录文件名称,偏移,大小
			ResEntry item;
			//转换为utf8
			std::string u8Name;
			UnicodeToUtf8(file, &u8Name);
			item.name = u8Name;
			item.size = size;
			item.offset = headOffset;
			ofs.write(data.c_str(), data.size());
			headOffset += data.size();
			items.push_back(item);
		}
		//将偏移文件头偏移信息写入到文件初始位置
		ofs.seekp(0);
		ofs.write((char*)(&headOffset), 4);
		//设置到文件条目位置
		ofs.seekp(headOffset);
		for (auto& item : items) {
			//写入文件偏移位置
			ofs.write((char*)(&item.offset), 4);
			//写入文件大小
			ofs.write((char*)(&item.size), 4);
			//文件路径名称
			ofs.write(item.name.c_str(), item.name.size() + 1);
		}
		ofs.flush();
		ofs.close();
	}
private:
	std::list<ResEntry> items;
public:
	Res(const std::wstring& resFile) {
		std::ifstream ifs(resFile, std::ios::binary);
		//读取记录文件位置的偏移
		ifs.seekg(0);
		DWORD headOffset;
		ifs.read((char*)&headOffset, 4);
		//获取文件大小
		ifs.seekg(0, std::ios::end);
		std::streampos size = ifs.tellg();
		//开始读取文件剩余条目
		ifs.seekg(headOffset);
		while (ifs.tellg() < size)
		{
			//读取到文件偏移位置
			DWORD fileOffset;
			ifs.read((char*)&fileOffset, 4);
			//读取文件大小
			DWORD fileSize;
			ifs.read((char*)&fileSize, 4);
			//读取文件名称
			char buf[512];
			for (size_t i = 0; i < sizeof(buf); i++)
			{
				char ch;
				ifs.read((char*)&ch, 1);
				buf[i] = ch;
				if (ch == 0) {
					break;
				}
			}
			ResEntry item;
			item.offset = fileOffset;
			item.size = fileSize;
			item.name = buf;
			items.push_back(item);
		}
	}
};

int main() {
	while (true)
	{
		File::Delete(L"d:/test.res");
		Res::Package(L"d:/test", L"d:/test.res");
		Res res(L"d:/test.res");
	}
	std::stringstream ss(std::ios::binary);
	return 0;
}