#include "FileSystem.h"
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
//定义....................................................................................................................
namespace FileSystem {
	void ReadFileInfoWin32(const Text::String& directory, WIN32_FIND_DATAW& pNextInfo, std::vector<FileSystem::FileInfo>& result, const Text::String& pattern, bool loopSubDir = false) {
		Text::String filename;
		filename.append(directory);
		filename.append("/");
		filename.append(Text::String(pNextInfo.cFileName));
		filename = filename.replace("\\", "/");
		filename = filename.replace("//", "/");
		struct FileSystem::FileInfo fileInfo;
		if (pNextInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { //目录
			fileInfo.FileType = FileType::Directory;
			fileInfo.FullName = filename;
			fileInfo.FileName = filename;
			if (loopSubDir) {
				Find(filename, result, pattern, loopSubDir);
			}
		}
		else if (pNextInfo.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
			fileInfo.FileType = FileType::File;
			fileInfo.FileName = pNextInfo.cFileName;
			fileInfo.FullName = filename;
			fileInfo.Extension = Path::GetExtension(filename);
			fileInfo.FileSize = ((ULONGLONG)pNextInfo.nFileSizeHigh << 32) | pNextInfo.nFileSizeLow;
		}
		if (pNextInfo.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
			fileInfo.ReadOnly = true;
		}
		if (fileInfo.FileType != FileType::None) {
			result.push_back(fileInfo);
		}
	}
	size_t  Find(const Text::String& directory, std::vector<FileSystem::FileInfo>& result, const Text::String& pattern, bool loopSubDir) {
		HANDLE hFile = INVALID_HANDLE_VALUE;
		WIN32_FIND_DATAW pNextInfo;
		hFile = FindFirstFileW(Text::String(directory + "/" + pattern).unicode().c_str(), &pNextInfo);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			return 0;
		}
		if (pNextInfo.cFileName[0] != '.') {
			ReadFileInfoWin32(directory, pNextInfo, result, pattern, loopSubDir);
		}
		while (FindNextFileW(hFile, &pNextInfo))
		{
			if (pNextInfo.cFileName[0] != '.') {
				ReadFileInfoWin32(directory, pNextInfo, result, pattern, loopSubDir);
			}
		}
		FindClose(hFile);//避免内存泄漏
		return result.size();
	};
};
namespace File {
	bool Exists(const Text::String& filename) {
		DWORD dwAttr = GetFileAttributesW(filename.unicode().c_str());
		if (dwAttr == DWORD(-1)) {
			return false;
		}
		if (dwAttr & FILE_ATTRIBUTE_ARCHIVE) {
			return true;
		}
		return false;

	}
	bool Create(const Text::String& filename) {
		File::Delete(filename);
		std::ofstream ofs(filename.unicode(), std::ios::binary | std::ios::app);
		ofs.close();
		return true;
	}
	bool Delete(const Text::String& filename) {
		::DeleteFileW(filename.unicode().c_str());
		if (File::Exists(filename)) {
			wprintf(L"Delete File ERROR %s\n", filename.unicode().c_str());
			return false;
		}
		return true;
	}
	bool Move(const Text::String& oldname, const Text::String& newname) {
		if (!File::Delete(newname)) {
			wprintf(L"MoveFile ERROR %s \n", oldname.unicode().c_str());
			return false;
		}
		::MoveFileExW(oldname.unicode().c_str(), newname.unicode().c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED);
		if (File::Exists(oldname)) {
			return false;
		}
		return true;
	}
	void ReadFile(const  Text::String& filename, FileStream* outFileStream) {
		outFileStream->clear();
		std::ifstream* ifs = new std::ifstream(filename.unicode().c_str(), std::ios::binary);
		ifs->seekg(0, std::ios::end);
		auto size = ifs->tellg();
		outFileStream->resize(size);
		ifs->seekg(0);
		ifs->read((char*)outFileStream->c_str(), size);
		ifs->close();
		delete ifs;
	}
	void WriteFile(const FileStream* fileStream, const Text::String& filename)
	{
		File::Delete(filename);
		std::ofstream* ofs = new std::ofstream(filename.unicode(), std::ios::binary | std::ios::app);
		ofs->write(fileStream->c_str(), fileStream->size());
		ofs->flush();
		ofs->close();
		delete ofs;
	}
	void WriteFile(const char* fileStream, size_t count, const Text::String& filename)
	{
		File::Delete(filename);
		std::ofstream* ofs = new std::ofstream(filename.unicode(), std::ios::binary | std::ios::app);
		ofs->write(fileStream, count);
		ofs->flush();
		ofs->close();
		delete ofs;
	}
	bool Copy(const Text::String& src_filename, const Text::String& des_filename, bool overwrite)
	{
		if (overwrite) {
			File::Delete(des_filename);
		}
		BOOL cancel = FALSE;
		return CopyFileExW(src_filename.unicode().c_str(), des_filename.unicode().c_str(), NULL, NULL, &cancel, 0);
	}
};
namespace Path {
	bool Create(const Text::String& path) {
		::CreateDirectoryW(path.unicode().c_str(), NULL);
		if (Path::Exists(path)) {
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
					if (!Path::Exists(root)) {
						if (::CreateDirectoryW(root.unicode().c_str(), NULL) == FALSE) {
							return false;
						}
					}
				}
			}
		}
		return Path::Exists(path);
	}
	bool Copy(const Text::String& srcPath, const Text::String& desPath, bool overwrite)
	{
		Text::String basePath = srcPath;
		basePath = basePath.replace("\\", "/");
		basePath = basePath.replace("//", "/");
		std::vector<FileSystem::FileInfo>result;
		FileSystem::Find(srcPath, result);
		size_t errCount = 0;
		for (auto& it : result) {
			Text::String fileName = it.FullName;
			fileName = fileName.replace(basePath, "");
			if (it.FileType == FileSystem::FileType::File) {
				if (File::Copy(it.FullName, desPath + "/" + fileName, overwrite) == false) {
					++errCount;
				}
			}
			if (it.FileType == FileSystem::FileType::Directory) {
				if (Path::Create(desPath + "/" + fileName) == false) {
					++errCount;
				}
			}
		}
		return errCount == 0;
	}
	bool Move(const Text::String& oldname, const Text::String& newname)
	{
		if (!Path::Exists(newname)) {
			wprintf(L"Move Faild !\n", oldname.unicode().c_str());
			return false;
		}
		::MoveFileExW(oldname.unicode().c_str(), newname.unicode().c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED);
		if (Path::Exists(oldname)) {
			return false;
		}
		return true;
	}
	bool Delete(const Text::String& directoryName) {
		std::vector<FileSystem::FileInfo>result;
		FileSystem::Find(directoryName, result);
		for (auto& it : result) {
			if (it.FileType == FileSystem::FileType::File) {
				if (!File::Delete(it.FullName)) {
					return false;
				}
			}
			else if (it.FileType == FileSystem::FileType::Directory) {
				if (!Path::Delete(it.FullName)) {
					return false;
				}
			}
		}
		if (::RemoveDirectoryW(directoryName.unicode().c_str()) == FALSE) {
			wprintf(L"RemoveDirectory ERROR %s\n", directoryName.unicode().c_str());
			return false;
		}
		return !Path::Exists(directoryName);
	}
	std::vector<Text::String> SearchFiles(const Text::String& path, const Text::String& pattern)
	{
		std::vector<Text::String> files;
		HANDLE hFile = INVALID_HANDLE_VALUE;
		WIN32_FIND_DATAW pNextInfo;
		Text::String dir;
		dir.append(path);
		dir.append("\\");
		dir.append(pattern);
		hFile = FindFirstFileW(dir.unicode().c_str(), &pNextInfo);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			return files;
		}
		WCHAR infPath[MAX_PATH] = { 0 };
		if (pNextInfo.cFileName[0] != '.')
		{
			Text::String filename;
			filename.append(path);
			filename.append("\\");
			filename.append(Text::String(pNextInfo.cFileName));
			filename = filename.replace("\\", "/");
			filename = filename.replace("//", "/");
			files.push_back(filename);
		}
		while (FindNextFileW(hFile, &pNextInfo))
		{
			if (pNextInfo.cFileName[0] == '.')
			{
				continue;
			}
			if (pNextInfo.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) { //如果是文件才要
				Text::String filename;
				filename.append(path);
				filename.append("\\");
				filename.append(Text::String(pNextInfo.cFileName));
				filename = filename.replace("\\", "/");
				filename = filename.replace("//", "/");
				files.push_back(filename);
			}
		}
		FindClose(hFile);//避免内存泄漏
		return files;
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
	Text::String StartPath() {
		return Path::GetDirectoryName(StartFileName());
	}
	Text::String __FileSytem_StartFileName;
	const Text::String& StartFileName() {
		if (__FileSytem_StartFileName.empty()) {
			WCHAR exeFullPath[MAX_PATH]{ 0 };
			::GetModuleFileNameW(NULL, exeFullPath, MAX_PATH);
			__FileSytem_StartFileName = exeFullPath;
		}
		return __FileSytem_StartFileName;
	}
	Text::String GetTempPath()
	{
		WCHAR user[256]{ 0 };
		DWORD len = 256;
		::GetUserNameW(user, &len);
		WCHAR temPath[256]{ 0 };
		swprintf_s(temPath, L"C:/Users/%s/AppData/Local/Temp", user);
		Path::Create(temPath);
		return Text::String(temPath);
	}
	Text::String GetAppTempPath()
	{
		WCHAR user[256]{ 0 };
		DWORD len = 256;
		::GetUserNameW(user, &len);
		WCHAR temPath[256]{ 0 };
		auto appName = Path::GetFileNameWithoutExtension(Path::StartFileName()).unicode();
		swprintf_s(temPath, L"C:/Users/%s/AppData/Local/Temp/%s", user, appName.c_str());
		Path::Create(temPath);
		return Text::String(temPath);
	}
	Text::String GetAppDataPath()
	{
		WCHAR user[256]{ 0 };
		DWORD len = 256;
		::GetUserNameW(user, &len);
		WCHAR localPath[256]{ 0 };
		swprintf_s(localPath, L"C:/Users/%s/AppData/Local/%s", user, Path::GetFileNameWithoutExtension(Path::StartFileName()).unicode().c_str());
		Path::Create(localPath);
		return Text::String(localPath);
	}

	bool Equal(const Text::String& path1, const Text::String& path2)
	{
		WCHAR canonicalPath1[MAX_PATH], canonicalPath2[MAX_PATH];
		if (!PathCanonicalizeW(canonicalPath1, path1.unicode().c_str())) {
			return false; // 规范化第一个路径失败
		}
		if (!PathCanonicalizeW(canonicalPath2, path2.unicode().c_str())) {
			return false; // 规范化第二个路径失败
		}
		return std::wcscmp(canonicalPath1, canonicalPath2) == 0;

		//auto a = str1;
		//a = a.replace("\\", "/");
		//a = a.replace("//", "/");

		//auto b = str1;
		//b = b.replace("\\", "/");
		//b = b.replace("//", "/");
		//return (a == b);
	}
};