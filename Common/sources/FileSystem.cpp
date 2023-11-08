#include "FileSystem.h"
//定义....................................................................................................................
namespace FileSystem {
	void ReadFileInfoWin32(const Text::Utf8String& directory, WIN32_FIND_DATAW& pNextInfo, std::vector<FileSystem::FileInfo>& result, const Text::Utf8String& pattern, bool loopSubDir = false) {
		Text::Utf8String filename;
		filename.Append(directory);
		filename.Append("\\");
		filename.Append(pNextInfo.cFileName);
		filename = filename.Replace("\\", "/");
		filename = filename.Replace("//", "/");
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
		}
		if (pNextInfo.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
			fileInfo.ReadOnly = true;
		}
		if (fileInfo.FileType != FileType::None) {
			result.push_back(fileInfo);
		}
	}
	size_t  Find(const Text::Utf8String& directory, std::vector<FileSystem::FileInfo>& result, const Text::Utf8String& pattern, bool loopSubDir) {
		HANDLE hFile = INVALID_HANDLE_VALUE;
		WIN32_FIND_DATAW pNextInfo;
		hFile = FindFirstFileW(Text::Utf8String(directory + "/" + pattern).unicode().c_str(), &pNextInfo);
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
	bool Exists(const Text::Utf8String& filename) {
		DWORD dwAttr = GetFileAttributesW(filename.unicode().c_str());
		if (dwAttr == DWORD(-1)) {
			return false;
		}
		if (dwAttr & FILE_ATTRIBUTE_ARCHIVE) {
			return true;
		}
		return false;

	}
	bool Create(const Text::Utf8String& filename) {
		File::Delete(filename);
		std::ofstream ofs(filename.unicode(), std::ios::binary | std::ios::app);
		ofs.close();
		return true;
	}
	bool Delete(const Text::Utf8String& filename) {
		::DeleteFileW(filename.unicode().c_str());
		return !File::Exists(filename);
	}
	bool Move(const Text::Utf8String& oldname, const Text::Utf8String& newname) {
		if (!File::Delete(newname)) {
			printf("Move Faild !\n");
			return false;
		}
		::MoveFileExW(oldname.unicode().c_str(), newname.unicode().c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED);
		if (File::Exists(oldname)) {
			return false;
		}
		return true;
	}
	void ReadFile(const  Text::Utf8String& filename, FileStream* outFileStream) {
		outFileStream->clear();
		std::ifstream* ifs = new std::ifstream(filename.unicode().c_str(), std::ios::binary);
		std::stringstream ss;
		ss << ifs->rdbuf();
		ifs->close();
		*outFileStream = ss.str();
		delete ifs;
	}
	void WriteFile(const FileStream* fileStream, const Text::Utf8String& filename)
	{
		File::Delete(filename);
		std::ofstream* ofs = new std::ofstream(filename.unicode(), std::ios::binary | std::ios::app);
		ofs->write(fileStream->c_str(), fileStream->size());
		ofs->flush();
		ofs->close();
		delete ofs;
	}
	void Copy(const Text::Utf8String& src_filename, const Text::Utf8String& des_filename)
	{
		FileStream fileData;
		File::ReadFile(src_filename, &fileData);//读取源文件
		File::Delete(des_filename);//直接覆盖
		std::ofstream ofs(des_filename.unicode(), std::ios::binary | std::ios::app);//写入到新的文件
		ofs.write(fileData.c_str(), fileData.size());
		ofs.flush();
		ofs.close();
	}
};
namespace Path {
	void FileWatcher::TaskFunc()
	{
		std::vector<Text::Utf8String> files;//启动加载当前文件
		//std::vector<Text::Utf8String> files = Path::SearchFiles(path, math.c_str());
		for (; exit; )
		{
			//移除不存在的文件
			for (size_t i = 0; i < files.size(); i++)
			{
				if (!File::Exists(files[i]))
				{
					std::vector<Text::Utf8String>::iterator it = std::find(files.begin(), files.end(), files[i]);
					if (it != files.end()) {
						files.erase(it);
					}
				}
			}
			//判断是否新增的文件s
			std::vector<Text::Utf8String> tmp = Path::SearchFiles(path, math.c_str());
			for (auto& item : tmp)
			{
				if (find(files.begin(), files.end(), item) == files.end())
				{
					files.push_back(item);
					if (callback) {
						callback(item);
					}
				}
			}
			//值越小越精准
			Sleep(sleep);
		}
	}
	FileWatcher::FileWatcher(const Text::Utf8String& path, const Text::Utf8String& math, const std::function<void(const Text::Utf8String& filename)>& callback, size_t sleep)
	{
		this->sleep = sleep;
		this->callback = callback;
		this->path = path;
		this->math = math;
		TaskFunc();
	}
	FileWatcher::~FileWatcher()
	{
		ctn = false;
	}
	bool Create(const Text::Utf8String& path) {
		::CreateDirectoryW(path.unicode().c_str(), NULL);
		if (Path::Exists(path)) {
			return true;
		}
		//创建多级目录
		if (path.find(":") != size_t(-1)) {
			Text::Utf8String dir = path + "/";
			dir = dir.Replace("\\", "/");
			dir = dir.Replace("//", "/");
			std::vector<Text::Utf8String> arr = dir.Split("/");
			Text::Utf8String root;
			if (arr.size() > 0) {
				root += arr[0] + "/";
				for (size_t i = 1; i < arr.size(); i++)
				{
					if (arr[i].empty()) {
						continue;
					}
					root += arr[i] + "/";
					if (!Path::Exists(root)) {
						::CreateDirectoryW(root.unicode().c_str(), NULL);
					}
				}
			}
		}
		return Path::Exists(path);
	}
	bool Copy(const Text::Utf8String& srcPath, const Text::Utf8String& desPath)
	{
		Text::Utf8String basePath = srcPath;
		basePath = basePath.Replace("\\", "/");
		basePath = basePath.Replace("//", "/");
		std::vector<FileSystem::FileInfo>result;
		FileSystem::Find(srcPath, result);
		for (auto& it : result) {
			Text::Utf8String fileName = it.FullName;
			fileName = fileName.Replace(basePath, "");
			if (it.FileType == FileSystem::FileType::File) {
				File::Copy(it.FullName, desPath + "/" + fileName);
			}
			if (it.FileType == FileSystem::FileType::Directory) {
				Path::Create(desPath + "/" + fileName);
			}
		}
		return false;
	}
	bool Delete(const Text::Utf8String& directoryName) {
		std::vector<FileSystem::FileInfo>result;
		FileSystem::Find(directoryName, result);
		for (auto& it : result) {
			if (it.FileType == FileSystem::FileType::File) {
				File::Delete(it.FullName);
			}
			if (it.FileType == FileSystem::FileType::Directory) {
				Path::Delete(it.FullName);
			}
		}
		::RemoveDirectoryW(directoryName.unicode().c_str());
		return !Path::Exists(directoryName);
	}
	std::vector<Text::Utf8String> SearchFiles(const Text::Utf8String& path, const Text::Utf8String& pattern)
	{
		std::vector<Text::Utf8String> files;
		HANDLE hFile = INVALID_HANDLE_VALUE;
		WIN32_FIND_DATAW pNextInfo;
		Text::Utf8String dir;
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
			Text::Utf8String filename;
			filename.append(path);
			filename.append("\\");
			filename.append(Text::Utf8String(pNextInfo.cFileName));
			filename = filename.Replace("\\", "/");
			filename = filename.Replace("//", "/");
			files.push_back(filename);
		}
		while (FindNextFileW(hFile, &pNextInfo))
		{
			if (pNextInfo.cFileName[0] == '.')
			{
				continue;
			}
			if (pNextInfo.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) { //如果是文件才要
				Text::Utf8String filename;
				filename.Append(path);
				filename.Append("\\");
				filename.Append(pNextInfo.cFileName);
				filename = filename.Replace("\\", "/");
				filename = filename.Replace("//", "/");
				files.push_back(filename);
			}
		}
		FindClose(hFile);//避免内存泄漏
		return files;
	}
	bool Exists(const Text::Utf8String& path) {
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

	Text::Utf8String GetFileNameWithoutExtension(const Text::Utf8String& _filename) {
		Text::Utf8String str = _filename;
		Text::Utf8String& newStr = str;
		newStr = newStr.Replace("\\", "/");
		int bPos = newStr.rfind("/");
		int ePos = newStr.rfind(".");
		newStr = newStr.substr(bPos + 1, ePos - bPos - 1);
		return newStr;
	}
	Text::Utf8String GetDirectoryName(const Text::Utf8String& _filename) {
		Text::Utf8String str = _filename;
		Text::Utf8String& newStr = str;
		newStr = newStr.Replace("\\", "/");
		int pos = newStr.rfind("/");
		return _filename.substr(0, pos);
	}
	Text::Utf8String GetExtension(const Text::Utf8String& _filename) {
		size_t pos = _filename.rfind(".");
		return pos == size_t(-1) ? "" : _filename.substr(pos);
	}
	Text::Utf8String GetFileName(const Text::Utf8String& filename) {
		return Path::GetFileNameWithoutExtension(filename) + Path::GetExtension(filename);
	}
	Text::Utf8String StartPath() {
		return Path::GetDirectoryName(StartFileName());
	}
	Text::Utf8String __FileSytem_StartFileName;
	const Text::Utf8String& StartFileName() {
		if (__FileSytem_StartFileName.empty()) {
			WCHAR exeFullPath[MAX_PATH]{ 0 };
			::GetModuleFileNameW(NULL, exeFullPath, MAX_PATH);
			__FileSytem_StartFileName = exeFullPath;
		}
		return __FileSytem_StartFileName;
	}
	Text::Utf8String GetTempPath()
	{
		WCHAR user[256]{ 0 };
		DWORD len = 256;
		::GetUserNameW(user, &len);
		WCHAR temPath[256]{ 0 };
		swprintf_s(temPath, L"C:/Users/%s/AppData/Local/Temp", user);
		Path::Create(temPath);
		return Text::Utf8String(temPath);
	}
	Text::Utf8String GetAppTempPath()
	{
		WCHAR user[256]{ 0 };
		DWORD len = 256;
		::GetUserNameW(user, &len);
		WCHAR temPath[256]{ 0 };
		//预防命名冲突
		auto pathMd5 = Text::Utf8String(MD5::FromString(Path::StartFileName())).unicode();
		swprintf_s(temPath, L"C:/Users/%s/AppData/Local/Temp/%s", user, pathMd5.c_str());
		Path::Create(temPath);
		return Text::Utf8String(temPath);
	}
	Text::Utf8String GetAppDataPath()
	{
		WCHAR user[256]{ 0 };
		DWORD len = 256;
		::GetUserNameW(user, &len);
		WCHAR localPath[256]{ 0 };
		swprintf_s(localPath, L"C:/Users/%s/AppData/Local/%s", user, Path::GetFileNameWithoutExtension(Path::StartFileName()).unicode().c_str());
		Path::Create(localPath);
		return Text::Utf8String(localPath);
	}
};