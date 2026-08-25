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
		Text::String directoryPath = Path::Format(directory);
		WIN32_FIND_DATAW findData;
		Text::String searchPath = Path::Format(directoryPath + "\\*");
		HANDLE hFind = ::FindFirstFileW(searchPath.unicode().c_str(), &findData);
		if (hFind == INVALID_HANDLE_VALUE) return result.size();
		do {
			Text::String name = findData.cFileName;
			// 忽略当前目录和上级目录
			if (name == "." || name == "..") continue;
			Text::String fullPath = Path::Format(directoryPath + "\\" + name);

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
		if (File::Exists(newname) && !File::Delete(newname)) {
			auto code = ::GetLastError();
			wprintf(L"code %d MoveFile Delete ERROR %s\n", code, newname.unicode().c_str());
			return false;
		}

		Text::String dir = Path::GetDirectoryName(newname);
		if (!Directory::Exists(dir)) {
			if (!Directory::Create(dir)) {
				return false;
			}
		}

		if (!::MoveFileExW(oldname.unicode().c_str(), newname.unicode().c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED)) {
			auto code = ::GetLastError();
			wprintf(L"code %d MoveFile ERROR %s\n", code, oldname.unicode().c_str());
			if (code == 5 && FileSystem::__RemoveAttr_OnlyRead_System(oldname.unicode())) {
				// 去除只读属性后直接再调用一次 MoveFileExW，不递归
				return ::MoveFileExW(oldname.unicode().c_str(), newname.unicode().c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED);
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
		return WriteFile(fileStream->data(), fileStream->size(), filename);
	}

	bool WriteFile(const char* fileStream, size_t count, const Text::String& filename) {
		if (File::Exists(filename) && !File::Delete(filename)) {
			return false;
		}
		Text::String dir = Path::GetDirectoryName(filename);
		if (!Directory::Exists(dir)) {
			if (!Directory::Create(dir)) {
				return false;
			}
		}
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
		// 确保目标目录存在
		Text::String dir = Path::GetDirectoryName(des_filename);
		if (!Directory::Exists(dir)) {
			if (!Directory::Create(dir)) {
				return false;
			}
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
		Text::String normalizedPath = Path::Format(path);
		if (normalizedPath.empty()) {
			return false;
		}

		// SHCreateDirectoryExW creates intermediate directories for drive and UNC paths.
		::SHCreateDirectoryExW(nullptr, normalizedPath.unicode().c_str(), nullptr);
		return Directory::Exists(normalizedPath);
	}
	bool Copy(const Text::String& srcPath, const Text::String& desPath, bool overwrite)
	{
		Text::String basePath = Path::Format(srcPath);
		Text::String destinationPath = Path::Format(desPath);
		if (!Directory::Exists(basePath)) {
			return false;
		}

		// Do not recurse into the source when the destination is the source itself
		// or one of its children.
		Text::String sourceRoot = basePath;
		while (sourceRoot.size() > 3 && !sourceRoot.empty() && sourceRoot.back() == '\\') {
			sourceRoot.erase(sourceRoot.size() - 1, 1);
		}
		Text::String destinationRoot = destinationPath;
		while (destinationRoot.size() > 3 && !destinationRoot.empty() && destinationRoot.back() == '\\') {
			destinationRoot.erase(destinationRoot.size() - 1, 1);
		}
		Text::String sourceLower = sourceRoot.toLower();
		Text::String destinationLower = destinationRoot.toLower();
		if (destinationLower == sourceLower ||
			(destinationLower.size() > sourceLower.size() &&
			 destinationLower.rfind(sourceLower + "\\", 0) == 0)) {
			return false;
		}

		if (Directory::Create(destinationPath) == false) {
			return false;
		}

		std::vector<FileSystem::FileInfo>result;
		Directory::Find(basePath, result);
		size_t errCount = 0;
		for (auto& it : result) {
			Text::String fileName = Path::Format(it.FileName).replace(basePath, "");
			if (!fileName.empty() && fileName.front() == '\\') {
				fileName.erase(0, 1);
			}
			if (fileName.empty()) {
				continue;
			}
			if (it.IsFile()) {
				if (File::Copy(it.FileName, destinationPath + "\\" + fileName, overwrite) == false) {
					++errCount;
				}
			}
			else {
				if (Directory::Copy(basePath + "\\" + fileName, destinationPath + "\\" + fileName, overwrite) == false) {
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
					wprintf(L"无法删除文件: %s\n", it.FileName.unicode().c_str());
					allDeleted = false;
				}
			}
			else {
				if (!Directory::Delete(it.FileName)) {
					wprintf(L"无法删除子目录: %s\n", it.FileName.unicode().c_str());
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
	namespace {
		Text::String __GetWindowsTempPath() {
			DWORD capacity = MAX_PATH;
			for (;;) {
				std::vector<wchar_t> buffer(capacity);
				DWORD length = ::GetTempPathW(capacity, buffer.data());
				if (length == 0) {
					return Text::String();
				}
				if (length < capacity) {
					return Text::String(buffer.data());
				}
				capacity = length + 1;
			}
		}

		Text::String __GetWindowsKnownFolder(REFKNOWNFOLDERID folderId) {
			PWSTR path = nullptr;
			if (FAILED(::SHGetKnownFolderPath(folderId, 0, NULL, &path)) || path == nullptr) {
				return Text::String();
			}
			Text::String result(path);
			::CoTaskMemFree(path);
			return result;
		}

		Text::String __CombineWindowsPath(const Text::String& directory, const Text::String& name) {
			if (directory.empty()) {
				return Text::String();
			}
			std::vector<wchar_t> combined(MAX_PATH * 2);
			if (::PathCombineW(combined.data(), directory.unicode().c_str(), name.unicode().c_str()) == nullptr) {
				return Text::String();
			}
			return Text::String(combined.data());
		}
	}

	Text::String GetFileNameWithoutExtension(const Text::String& _filename) {
		Text::String newStr = Path::Format(_filename);
		size_t bPos = newStr.rfind("\\");
		size_t ePos = newStr.rfind(".");
		size_t start = bPos == size_t(-1) ? 0 : bPos + 1;
		return ePos == size_t(-1) || ePos < start ? newStr.substr(start) : newStr.substr(start, ePos - start);
	}

	Text::String GetDirectoryName(const Text::String& _filename) {
		Text::String newPath = Path::Format(_filename);
		size_t pos = newPath.rfind("\\");
		return pos == size_t(-1) ? Text::String() : newPath.substr(0, pos);
	}

	Text::String GetExtension(const Text::String& _filename) {
		Text::String newPath = Path::Format(_filename);
		size_t slash = newPath.rfind("\\");
		size_t dot = newPath.rfind(".");
		size_t start = slash == size_t(-1) ? 0 : slash + 1;
		return dot == size_t(-1) || dot < start ? "" : newPath.substr(dot);
	}

	Text::String GetFileName(const Text::String& filename) {
		return Path::GetFileNameWithoutExtension(filename) + Path::GetExtension(filename);
	}

	Text::String UserDesktop(bool allUsers) {
		WCHAR buf[MAX_PATH]{ 0 };
		int csidl = allUsers ? CSIDL_COMMON_DESKTOPDIRECTORY : CSIDL_DESKTOPDIRECTORY;
		if (SHGetSpecialFolderPathW(nullptr, buf, csidl, FALSE)) {
			return Path::Format(Text::String(buf));
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
		return Path::Format(Text::String(result));
	}

	Text::String StartPath() {
		return Path::GetDirectoryName(StartFileName());
	}

	Text::String __FileSytem_StartFileName;
	const Text::String& StartFileName() {
		if (__FileSytem_StartFileName.empty()) {
			std::vector<wchar_t> wPath(32768);
			DWORD count = ::GetModuleFileNameW(NULL, wPath.data(), (DWORD)wPath.size());
			__FileSytem_StartFileName = Path::Format(wPath.data());
		}
		return __FileSytem_StartFileName;
	}

	Text::String GetTempPath()
	{
		return Path::Format(__GetWindowsTempPath());
	}
	Text::String GetAppTempPath(const Text::String& appName)
	{
		Text::String name = appName.empty() ? Path::GetFileNameWithoutExtension(Path::StartFileName()) : appName;
		Text::String appTempPath = __CombineWindowsPath(GetTempPath(), name);
		appTempPath = Path::Format(appTempPath);
		if (!appTempPath.empty()) {
			Directory::Create(appTempPath);
		}
		return appTempPath;
	}
	Text::String GetAppDataPath(const Text::String& appName)
	{
		Text::String name = appName.empty() ? Path::GetFileNameWithoutExtension(Path::StartFileName()) : appName;
		Text::String appDataPath = __CombineWindowsPath(__GetWindowsKnownFolder(FOLDERID_LocalAppData), name);
		appDataPath = Path::Format(appDataPath);
		if (!appDataPath.empty()) {
			Directory::Create(appDataPath);
		}
		return appDataPath;
	}
	Text::String Format(const Text::String& path)
	{
		// Windows 路径统一使用反斜杠；保留 UNC 路径开头的双反斜杠。
		Text::String convertedPath = path.replace('/', '\\');
		Text::String newPath;
		newPath.reserve(convertedPath.size());
		const bool preserveUncPrefix = convertedPath.size() >= 2 && convertedPath[0] == '\\' && convertedPath[1] == '\\';
		size_t index = 0;
		if (preserveUncPrefix) {
			newPath += "\\\\";
			index = 2;
		}
		for (; index < convertedPath.size(); ++index) {
			if (convertedPath[index] == '\\' && !newPath.empty() && newPath.back() == '\\') {
				continue;
			}
			newPath += convertedPath[index];
		}
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
