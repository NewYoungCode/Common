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
	//����ת��
	static void UnicodeToUtf8(const std::wstring& wstr, std::string* utf8Str) {
		int bytes = ::WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
		utf8Str->resize(bytes);
		::WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.size(), const_cast<char*>(utf8Str->c_str()), utf8Str->size(), NULL, NULL);
	}
	//Ѱ��ָ��Ŀ¼�Լ�Ŀ¼�µ������ļ�
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
				// �����Ŀ¼���ݹ���Ҹ�Ŀ¼  
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
			//��ȡ�ļ���С
			std::ifstream ifs(file, std::ios::binary);
			ifs.seekg(0, std::ios::end);
			auto size = ifs.tellg();
			//��ȡ�ļ�
			std::string data;
			data.resize(size);
			ifs.seekg(0);
			ifs.read((char*)data.c_str(), size);
			//��¼�ļ�����,ƫ��,��С
			ResEntry item;
			//ת��Ϊutf8
			std::string u8Name;
			UnicodeToUtf8(file, &u8Name);
			item.name = u8Name;
			item.size = size;
			item.offset = headOffset;
			ofs.write(data.c_str(), data.size());
			headOffset += data.size();
			items.push_back(item);
		}
		//��ƫ���ļ�ͷƫ����Ϣд�뵽�ļ���ʼλ��
		ofs.seekp(0);
		ofs.write((char*)(&headOffset), 4);
		//���õ��ļ���Ŀλ��
		ofs.seekp(headOffset);
		for (auto& item : items) {
			//д���ļ�ƫ��λ��
			ofs.write((char*)(&item.offset), 4);
			//д���ļ���С
			ofs.write((char*)(&item.size), 4);
			//�ļ�·������
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
		//��ȡ��¼�ļ�λ�õ�ƫ��
		ifs.seekg(0);
		DWORD headOffset;
		ifs.read((char*)&headOffset, 4);
		//��ȡ�ļ���С
		ifs.seekg(0, std::ios::end);
		std::streampos size = ifs.tellg();
		//��ʼ��ȡ�ļ�ʣ����Ŀ
		ifs.seekg(headOffset);
		while (ifs.tellg() < size)
		{
			//��ȡ���ļ�ƫ��λ��
			DWORD fileOffset;
			ifs.read((char*)&fileOffset, 4);
			//��ȡ�ļ���С
			DWORD fileSize;
			ifs.read((char*)&fileSize, 4);
			//��ȡ�ļ�����
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