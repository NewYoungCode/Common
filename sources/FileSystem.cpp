#include "FileSystem.h"
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <shlobj.h>      // SHGetSpecialFolderPathW
#pragma comment(lib, "shell32.lib")  // 链接 shell32.lib
//定义....................................................................................................................
namespace FileSystem {
	//移除只读属性和系统属性
	bool __RemoveAttr_OnlyRead_System(const std::wstring& _path) {
		auto wPath = _path.c_str();
		// 获取当前文件或目录的属性
		DWORD currentAttributes = GetFileAttributesW(wPath);
		if (currentAttributes == INVALID_FILE_ATTRIBUTES) {
			return false;
		}
		// 去除只读属性和系统属性
		DWORD newAttributes = currentAttributes & ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM);
		// 设置新的属性
		if (::SetFileAttributesW(wPath, newAttributes)) {
			return true;
		}
		return false;
	}
	size_t Find(const Text::String& directory, std::vector<FileSystem::FileInfo>& result, const Text::String& pattern, bool loopSubDir, FileType fileType) {
		WIN32_FIND_DATAW findData;
		Text::String searchPath = directory + "\\*";
		HANDLE hFind = ::FindFirstFileW(searchPath.unicode().c_str(), &findData);
		if (hFind == INVALID_HANDLE_VALUE) return result.size();
		do {
			Text::String name = findData.cFileName;
			// 忽略当前目录和上级目录
			if (name == "." || name == "..") continue;
			Text::String fullPath = directory + "\\" + name;

			FileSystem::FileInfo fileInfo;
			fileInfo.dwFileAttributes = findData.dwFileAttributes;
			(Text::String&)fileInfo.FileName = fullPath;

			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (::PathMatchSpecW(name.unicode().c_str(), pattern.unicode().c_str()) && ((fileType & FileType::Directory) == FileType::Directory)) {
					result.push_back(fileInfo); // 是文件夹
				}
				if (loopSubDir) {
					Find(fullPath, result, pattern, loopSubDir, fileType);// 递归进入
				}
			}
			else {
				if (::PathMatchSpecW(name.unicode().c_str(), pattern.unicode().c_str()) && ((fileType & FileType::File) == FileType::File)) {
					(ULONGLONG&)fileInfo.FileSize = ((ULONGLONG)findData.nFileSizeHigh << 32) | findData.nFileSizeLow;
					result.push_back(fileInfo); // 是文件
				}
			}
		} while (::FindNextFileW(hFind, &findData));
		::FindClose(hFind);
		return result.size();
	};
};
namespace File {
	bool Exists(const Text::String& filename) {
		DWORD dwAttr = ::GetFileAttributesW(filename.unicode().c_str());
		return (dwAttr != INVALID_FILE_ATTRIBUTES && !(dwAttr & FILE_ATTRIBUTE_DIRECTORY));
	}
	bool Create(const Text::String& filename) {
		if (File::Exists(filename)) {
			return true;
		}
		File::Delete(filename);
		std::ofstream ofs(filename.unicode(), std::ios::binary | std::ios::app);
		ofs.close();
		return ofs.good();
	}
	bool Delete(const Text::String& filename) {
		::DeleteFileW(filename.unicode().c_str());
		if (File::Exists(filename)) {
			auto code = ::GetLastError();
			if (code == 5 && FileSystem::__RemoveAttr_OnlyRead_System(filename.unicode())) {
				return ::DeleteFileW(filename.unicode().c_str());
			}
			wprintf(L"code %d Delete File ERROR %s\n", code, filename.unicode().c_str());
			return false;
		}
		return true;
	}
	bool Move(const Text::String& oldname, const Text::String& newname) {
		if (!File::Delete(newname)) {//进行覆盖
			auto code = ::GetLastError();
			wprintf(L"code %d MoveFile ERROR %s \n", code, oldname.unicode().c_str());
			return false;
		}
		::MoveFileExW(oldname.unicode().c_str(), newname.unicode().c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED);
		if (File::Exists(oldname)) {
			auto code = ::GetLastError();
			if (code == 5 && FileSystem::__RemoveAttr_OnlyRead_System(oldname.unicode())) {
				return Move(oldname, newname);
			}
			return false;
		}
		return true;
	}
	bool ReadFile(const Text::String& filename, FileStream* outFileStream) {
		outFileStream->clear();
		std::ifstream ifs(filename.unicode().c_str(), std::ios::binary);
		if (!ifs.is_open()) {
			return false;
		}
		ifs.seekg(0, std::ios::end);
		std::streamsize size = ifs.tellg();
		if (size == std::streamsize(-1)) {
			return false;
		}
		outFileStream->resize(size);
		ifs.seekg(0);
		ifs.read((char*)outFileStream->c_str(), size);
		return ifs.good();
	}

	bool WriteFile(const FileStream* fileStream, const Text::String& filename) {
		File::Delete(filename);
		std::ofstream ofs(filename.unicode(), std::ios::binary);
		if (!ofs.is_open()) {
			return false;
		}
		ofs.write(fileStream->c_str(), fileStream->size());
		ofs.flush();
		return ofs.good();
	}

	bool WriteFile(const char* fileStream, size_t count, const Text::String& filename) {
		File::Delete(filename);
		std::ofstream ofs(filename.unicode(), std::ios::binary);
		if (!ofs.is_open()) {
			return false;
		}
		ofs.write(fileStream, count);
		ofs.flush();
		return ofs.good();
	}

	bool Copy(const Text::String& src_filename, const Text::String& des_filename, bool overwrite)
	{
		if (overwrite) {
			File::Delete(des_filename);
		}
		BOOL cancel = FALSE;
		auto ret = ::CopyFileExW(src_filename.unicode().c_str(), des_filename.unicode().c_str(), NULL, NULL, &cancel, 0);
		return ret;
	}
	ULONGLONG GetFileSize(const Text::String& fileName) {
		WIN32_FILE_ATTRIBUTE_DATA fileInfo;
		if (GetFileAttributesExW(fileName.unicode().c_str(), GetFileExInfoStandard, &fileInfo)) {
			ULONGLONG size = ((ULONGLONG)fileInfo.nFileSizeHigh << 32) | fileInfo.nFileSizeLow;
			return size;
		}
		return 0;
	}
};

namespace Directory {
	bool Create(const Text::String& path) {
		::CreateDirectoryW(path.unicode().c_str(), NULL);
		if (Directory::Exists(path)) {
			return true;
		}
		//创建多级目录
		if (path.find(":") != size_t(-1)) {
			Text::String dir = path + "/";
			dir = dir.replace("\\", "/");
			dir = dir.replace("//", "/");
			auto arr = dir.split("/");
			Text::String root;
			if (arr.size() > 0) {
				root += arr[0] + "/";
				for (size_t i = 1; i < arr.size(); i++)
				{
					if (arr[i].empty()) {
						continue;
					}
					root += arr[i] + "/";
					if (!Directory::Exists(root)) {
						if (::CreateDirectoryW(root.unicode().c_str(), NULL) == FALSE) {
							return false;
						}
					}
				}
			}
		}
		return Directory::Exists(path);
	}
	bool Copy(const Text::String& srcPath, const Text::String& desPath, bool overwrite)
	{
		Text::String basePath = srcPath;
		basePath = basePath.replace("\\", "/");
		basePath = basePath.replace("//", "/");

		if (Directory::Create(desPath) == false) {
			return false;
		}

		std::vector<FileSystem::FileInfo>result;
		Directory::Find(srcPath, result);
		size_t errCount = 0;
		for (auto& it : result) {
			Text::String fileName = it.FileName;
			fileName = fileName.replace(basePath, "");
			if (fileName.empty()) {
				continue;
			}
			if (it.IsFile()) {
				if (File::Copy(it.FileName, desPath + "/" + fileName, overwrite) == false) {
					++errCount;
				}
			}
			else {
				if (Directory::Copy(srcPath + "/" + fileName, desPath + "/" + fileName, overwrite) == false) {
					++errCount;
				}
			}
		}
		return errCount == 0;
	}
	bool Move(const Text::String& oldname, const Text::String& newname)
	{
		if (!Directory::Exists(newname)) {
			wprintf(L"Move Faild !\n", oldname.unicode().c_str());
			return false;
		}
		::MoveFileExW(oldname.unicode().c_str(), newname.unicode().c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED);
		if (Directory::Exists(oldname)) {
			return false;
		}
		return true;
	}
	bool Delete(const Text::String& directoryName) {
		if (!Directory::Exists(directoryName)) {
			return true; // 不存在，视为已删除
		}
		std::vector<FileSystem::FileInfo> result;
		Directory::Find(directoryName, result);

		bool allDeleted = true;
		for (auto& it : result) {
			if (it.IsFile()) {
				if (!File::Delete(it.FileName)) {
					wprintf(L"无法删除文件: %s\n", it.FileName.c_str());
					allDeleted = false;
				}
			}
			else {
				if (!Directory::Delete(it.FileName)) {
					wprintf(L"无法删除子目录: %s\n", it.FileName.c_str());
					allDeleted = false;
				}
			}
		}
		if (::RemoveDirectoryW(directoryName.unicode().c_str()) == FALSE) {
			auto code = ::GetLastError();
			if (code == 5 && FileSystem::__RemoveAttr_OnlyRead_System(directoryName.unicode())) {
				return Directory::Delete(directoryName.unicode());
			}
			wprintf(L"RemoveDirectory 错误 %d: %s\n", code, directoryName.unicode().c_str());
			return false;
		}
		return true;
	}

	size_t Find(const Text::String& path, std::vector<FileSystem::FileInfo>& result, const Text::String& pattern, bool loopSubDir, FileSystem::FileType fileType)
	{
		return FileSystem::Find(path, result, pattern, loopSubDir, fileType);
	}
	bool Exists(const Text::String& path) {
		DWORD dwAttr = GetFileAttributesW(path.unicode().c_str());
		if (dwAttr == DWORD(-1)) {
			return false;
		}
		if (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
		{
			return true;
		}
		return false;
	}
};
namespace Path {
	Text::String GetFileNameWithoutExtension(const Text::String& _filename) {
		Text::String str = _filename;
		Text::String& newStr = str;
		newStr = newStr.replace("\\", "/");
		int bPos = newStr.rfind("/");
		int ePos = newStr.rfind(".");
		newStr = newStr.substr(bPos + 1, ePos - bPos - 1);
		return newStr;
	}

	Text::String GetDirectoryName(const Text::String& _filename) {
		Text::String str = _filename;
		Text::String& newStr = str;
		newStr = newStr.replace("\\", "/");
		int pos = newStr.rfind("/");
		return _filename.substr(0, pos);
	}

	Text::String GetExtension(const Text::String& _filename) {
		size_t pos = _filename.rfind(".");
		return pos == size_t(-1) ? "" : _filename.substr(pos);
	}

	Text::String GetFileName(const Text::String& filename) {
		return Path::GetFileNameWithoutExtension(filename) + Path::GetExtension(filename);
	}

	Text::String UserDesktop(bool allUsers) {
		WCHAR buf[MAX_PATH]{ 0 };
		int csidl = allUsers ? CSIDL_COMMON_DESKTOPDIRECTORY : CSIDL_DESKTOPDIRECTORY;
		if (SHGetSpecialFolderPathW(nullptr, buf, csidl, FALSE)) {
			return Text::String(buf);
		}
		return L""; // 获取失败
	}

	Text::String StartPrograms(bool allUsers) {
		PWSTR path = nullptr;
		std::wstring result;
		HRESULT hr = SHGetKnownFolderPath(allUsers ? FOLDERID_CommonPrograms : FOLDERID_Programs, 0, NULL, &path);
		if (SUCCEEDED(hr)) {
			result = path;
			CoTaskMemFree(path); // 释放内存
		}
		return result;
	}

	Text::String StartPath() {
		return Path::GetDirectoryName(StartFileName());
	}

	Text::String __FileSytem_StartFileName;
	const Text::String& StartFileName() {
		if (__FileSytem_StartFileName.empty()) {
			std::vector<wchar_t> wPath(32768);
			DWORD count = ::GetModuleFileNameW(NULL, wPath.data(), (DWORD)wPath.size());
			__FileSytem_StartFileName = wPath.data();
		}
		return __FileSytem_StartFileName;
	}

	Text::String GetTempPath()
	{
		WCHAR user[MAX_PATH]{ 0 };
		DWORD len = MAX_PATH;
		::GetUserNameW(user, &len);
		WCHAR temPath[MAX_PATH]{ 0 };
		swprintf_s(temPath, L"C:/Users/%s/AppData/Local/Temp", user);
		Directory::Create(temPath);
		return Text::String(temPath);
	}
	Text::String GetAppTempPath(const Text::String& appName)
	{
		WCHAR user[MAX_PATH]{ 0 };
		DWORD len = MAX_PATH;
		::GetUserNameW(user, &len);
		WCHAR temPath[MAX_PATH]{ 0 };
		swprintf_s(temPath, L"C:/Users/%s/AppData/Local/Temp/%s", user, appName.empty() ? Path::GetFileNameWithoutExtension(Path::StartFileName()).unicode().c_str() : appName.unicode().c_str());
		Directory::Create(temPath);
		return Text::String(temPath);
	}
	Text::String GetAppDataPath(const Text::String& appName)
	{
		WCHAR user[MAX_PATH]{ 0 };
		DWORD len = MAX_PATH;
		::GetUserNameW(user, &len);
		WCHAR localPath[MAX_PATH]{ 0 };
		swprintf_s(localPath, L"C:/Users/%s/AppData/Local/%s", user, appName.empty() ? Path::GetFileNameWithoutExtension(Path::StartFileName()).unicode().c_str() : appName.unicode().c_str());
		Directory::Create(localPath);
		return Text::String(localPath);
	}
	Text::String Format(const Text::String& path)
	{
		Text::String newPath = path.replace("\\", "/", true);
		newPath = newPath.replace("//", "/", true);
		return newPath;
	}
	bool Equal(const Text::String& path1, const Text::String& path2)
	{
		Text::String a = path1.trim();
		a.erase('\\');
		a.erase('/');

		Text::String b = path2.trim();
		b.erase('\\');
		b.erase('/');
		return (a == b);
	}
};