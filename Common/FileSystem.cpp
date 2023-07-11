#include "FileSystem.h"
//定义....................................................................................................................
namespace FileSystem {
	void ReadFileInfoWin32(const Text::Utf8String& directory, WIN32_FIND_DATAW& pNextInfo, std::vector<FileSystem::FileInfo>& result) {
		Text::Utf8String filename;
		filename.append(directory);
		filename.append("\\");
		filename.append(pNextInfo.cFileName);
		filename = filename.Replace("\\", "/");
		filename = filename.Replace("//", "/");
		struct FileSystem::FileInfo fileInfo;
		if (pNextInfo.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) { //目录
			fileInfo.FileType = FileType::Directory;
			fileInfo.FullName = filename;
			fileInfo.FileName = filename;
		}
		else if (FILE_ATTRIBUTE_ARCHIVE & pNextInfo.dwFileAttributes) {
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
	size_t  Find(const Text::Utf8String& directory, std::vector<FileSystem::FileInfo>& result, const Text::Utf8String& pattern) {
		HANDLE hFile = INVALID_HANDLE_VALUE;
		WIN32_FIND_DATAW pNextInfo;
		hFile = FindFirstFileW(Text::Utf8String(directory + "\\" + pattern).utf16().c_str(), &pNextInfo);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			return 0;
		}
		if (pNextInfo.cFileName[0] != '.') {
			ReadFileInfoWin32(directory, pNextInfo, result);
		}
		while (FindNextFileW(hFile, &pNextInfo))
		{
			if (pNextInfo.cFileName[0] != '.') {
				ReadFileInfoWin32(directory, pNextInfo, result);
			}
		}
		FindClose(hFile);//避免内存泄漏
		return result.size();
	};
};
namespace File {
	bool Exists(const Text::Utf8String& filename) {
		DWORD dwAttr = GetFileAttributesW(filename.utf16().c_str());
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
		std::ofstream ofs(filename.c_str(), std::ios::app);
		ofs.close();
		return true;
	}
	bool Delete(const Text::Utf8String& filename) {
		::DeleteFileW(filename.utf16().c_str());
		return !File::Exists(filename);
	}
	bool Move(const Text::Utf8String& oldname, const Text::Utf8String& newname) {
		if (!File::Delete(newname)) {
			printf("Move Faild !\n");
			return false;
		}
		::MoveFileExW(oldname.utf16().c_str(), newname.utf16().c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED);
		if (File::Exists(oldname)) {
			return false;
		}
		return true;
	}
	void ReadFile(const  Text::Utf8String& filename, FileStream* outFileStream) {
		outFileStream->clear();
		std::ifstream* ifs = new std::ifstream(filename.c_str(), std::ios::binary);
		std::stringstream ss;
		ss << ifs->rdbuf();
		ifs->close();
		*outFileStream = ss.str();
		delete ifs;
	}
	void WriteFile(const FileStream* fileStream, const Text::Utf8String& filename)
	{
		File::Delete(filename);
		std::ofstream* ofs = new std::ofstream(filename, std::ios::binary);
		ofs->write(fileStream->c_str(), fileStream->size());
		ofs->flush();
		ofs->close();
		delete ofs;
	}
	void Copy(const Text::Utf8String& src_filename, const Text::Utf8String& des_filename, bool cover)
	{
		FileStream fileData;
		File::ReadFile(src_filename, &fileData);//读取源文件
		if (cover) {
			File::Delete(des_filename);
		}
		std::ofstream ofs(des_filename, std::ios::binary | std::ios::app);//写入到新的文件
		ofs.write(fileData.c_str(), fileData.size());
		ofs.flush();
		ofs.close();
	}
	Text::Utf8String CreateTempFile(const Text::Utf8String& filename)
	{
		WCHAR buf[MAX_PATH]{ 0 };
		GetTempPathW(512, buf);
		Text::Utf8String path = Text::Utf8String(buf) + "\\" + Path::GetFileNameWithoutExtension(Path::StartFileName());
		Path::Create(path);

		Text::Utf8String file = path + "\\" + filename;
		File::Create(file);
		return file;
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
		::CreateDirectoryW(path.utf16().c_str(), NULL);
		if (Path::Exists(path)) {
			return true;
		}
		//创建多级目录
		if (path.find(":") != size_t(-1)) {
			Text::Utf8String dir = path + "/";
			dir.Replace("\\", "/");
			dir.Replace("//", "/");
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
						::CreateDirectoryW(root.utf16().c_str(), NULL);
					}
				}
			}
		}
		return Path::Exists(path);
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
		::RemoveDirectoryW(directoryName.utf16().c_str());
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
		hFile = FindFirstFileW(dir.utf16().c_str(), &pNextInfo);
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
			if (pNextInfo.dwFileAttributes == FILE_ATTRIBUTE_ARCHIVE) { //如果是文件才要
				Text::Utf8String filename;
				filename.append(path);
				filename.append("\\");
				filename.append(pNextInfo.cFileName);
				filename = filename.Replace("\\", "/");
				filename = filename.Replace("//", "/");
				files.push_back(filename);
			}
		}
		FindClose(hFile);//避免内存泄漏
		return files;
	}
	bool Exists(const Text::Utf8String& path) {
		DWORD dwAttr = GetFileAttributesW(path.utf16().c_str());
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
		newStr.Replace("\\", "/");
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
	Text::Utf8String StartFileName() {
		WCHAR exeFullPath[MAX_PATH]{ 0 };
		::GetModuleFileNameW(NULL, exeFullPath, MAX_PATH);
		return Text::Utf8String(exeFullPath);
	}
};