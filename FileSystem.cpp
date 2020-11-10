#include "FileSystem.h"
//定义....................................................................................................................
namespace FileSystem {
	  void ReadFileInfo(const std::string& directory, WIN32_FIND_DATAA&pNextInfo, std::vector<FileSystem::FileInfo>&result) {
		std::string filename;
		filename.append(directory);
		filename.append("\\");
		filename.append(pNextInfo.cFileName);
		Text::Replace(filename, "\\", "/");
		Text::Replace(filename, "//", "/");
		struct FileSystem::FileInfo fileInfo;
		if (pNextInfo.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) { //目录
			fileInfo.FileType = FileType::Directory;
			fileInfo.FullName = filename;
			fileInfo.FileName = filename;
		}
		else if (FILE_ATTRIBUTE_ARCHIVE& pNextInfo.dwFileAttributes) {
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
	  size_t  Find(const std::string& directory, std::vector<FileSystem::FileInfo>&result, const char* pattern) {
		HANDLE hFile = INVALID_HANDLE_VALUE;
		WIN32_FIND_DATAA pNextInfo;
		hFile = FindFirstFileA((directory + "\\" + pattern).c_str(), &pNextInfo);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			return 0;
		}
		if (pNextInfo.cFileName[0] != '.') {
			ReadFileInfo(directory, pNextInfo, result);
		}
		while (FindNextFileA(hFile, &pNextInfo))
		{
			if (pNextInfo.cFileName[0] != '.') {
				ReadFileInfo(directory, pNextInfo, result);
			}
		}
		FindClose(hFile);//避免内存泄漏
		return result.size();
	};
};
namespace File {
	  bool Exists(const std::string&filename) {
		struct stat buf;
		int status = stat(filename.c_str(), &buf);
		if (status == 0 && (buf.st_mode & S_IFREG) == S_IFREG) {
			return true;
		}
		else {
			return false;
		}
	}
	  bool Create(const std::string &filename) {
		std::ofstream ofs(filename.c_str(), std::ios::app);
		ofs.flush();
		ofs.close();
		return true;
	}
	  std::string ReadFile(const std::string&filename) {
		std::stringstream ss;
		std::ifstream ifs(filename);
		ss << ifs.rdbuf();
		return ss.str();
	}
	  bool Delete(const std::string&filename) {
		::DeleteFileA(filename.c_str());
		return !File::Exists(filename);
	}
	  bool Move(const std::string &oldname, const std::string &newname) {
		return ::rename(oldname.c_str(), newname.c_str()) == 0 ? false : true;
	}
	  void ReadFile(const  std::string &filename, std::string&outData) {
		outData.clear();
		std::ifstream *ifs = new std::ifstream(filename.c_str(), std::ios::binary);
		std::stringstream ss;
		ss << ifs->rdbuf();
		ifs->close();
		outData = ss.str();
		delete ifs;
	}
	  void WriteFile(const std::stringstream & data, const std::string & filename)
	{
		std::string buf = data.str();
		File::Delete(filename);
		std::ofstream *ofs = new std::ofstream(filename, std::ios::binary);
		ofs->write(buf.c_str(), buf.size());
		ofs->flush();
		ofs->close();
		delete ofs;
	}
};
namespace Path {
	  bool Create(const std::string &path, bool MultiDir) {
		if (MultiDir &&path.find(":") != size_t(-1)) { //创建多级目录
			std::string dir = path + "/";
			Text::Replace(dir, "\\", "/");
			Text::Replace(dir, "//", "/");
			std::vector<std::string> arr = Text::Split(dir, "/");
			std::string root;
			if (arr.size() > 0) {
				root += arr[0] + "/";
				for (size_t i = 1; i < arr.size(); i++)
				{
					if (arr[i].empty()) {
						continue;
					}
					root += arr[i] + "/";
					if (!Path::Exists(root)) {
						int result = ::CreateDirectoryA(root.c_str(), 0);
					}
				}
			}
			return Path::Exists(path);
		}
		return	::CreateDirectoryA(path.c_str(), 0) == 0 ? false : true;
	}
	  bool Delete(const std::string& directoryName) {
		std::vector<FileSystem::FileInfo>result;
		FileSystem::Find(directoryName, result);
		for (auto&it : result) {
			if (it.FileType == FileSystem::FileType::File) {
				File::Delete(it.FullName);
			}
			if (it.FileType == FileSystem::FileType::Directory) {
				Path::Delete(it.FullName);
			}
		}
		return	::RemoveDirectoryA(directoryName.c_str()) == 0 ? false : true;
		/*system(std::string("rmdir /s /q " + directoryName).c_str());
		return true;*/
	}
	  std::vector<std::string> SearchFiles(const std::string &path, const char*pattern)
	{
		std::vector<std::string> files;
		HANDLE hFile = INVALID_HANDLE_VALUE;
		WIN32_FIND_DATAA pNextInfo;
		std::string dir;
		dir.append(path);
		dir.append("\\");
		dir.append(pattern);
		hFile = FindFirstFileA(dir.c_str(), &pNextInfo);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			return files;
		}
		WCHAR infPath[MAX_PATH] = { 0 };
		if (pNextInfo.cFileName[0] != '.')
		{
			std::string filename;
			filename.append(path);
			filename.append("\\");
			filename.append(pNextInfo.cFileName);
			Text::Replace(filename, "\\", "/");
			Text::Replace(filename, "//", "/");
			files.push_back(filename);
		}
		while (FindNextFileA(hFile, &pNextInfo))
		{
			if (pNextInfo.cFileName[0] == '.')
			{
				continue;
			}
			if (pNextInfo.dwFileAttributes == FILE_ATTRIBUTE_ARCHIVE) { //如果是文件才要
				std::string filename;
				filename.append(path);
				filename.append("\\");
				filename.append(pNextInfo.cFileName);
				Text::Replace(filename, "\\", "/");
				Text::Replace(filename, "//", "/");

				files.push_back(filename);

			}
		}
		FindClose(hFile);//避免内存泄漏
		return files;
	}
	  bool Exists(const std::string & path)
	{
		struct stat buf;
		int status = stat(path.c_str(), &buf);
		if (status == 0) {
			return true;
		}
		return false;
	}
	  std::string GetFileNameWithoutExtension(const std::string &_filename) {
		std::string newDir = Text::ReplaceAll(_filename, "\\", "/");
		int bPos = newDir.rfind("/");
		int ePos = newDir.rfind(".");
		newDir = newDir.substr(bPos + 1, ePos - bPos - 1);
		return newDir;
	}
	  std::string GetDirectoryName(const std::string &_filename) {
		int pos = Text::ReplaceAll(_filename, "\\", "/").rfind("/");
		return _filename.substr(0, pos);
	}
	  std::string GetExtension(const std::string &_filename) {
		size_t pos = _filename.rfind(".");
		return pos == size_t(-1) ? "" : _filename.substr(pos);
	}
	  std::string GetFileName(const std::string &filename) {
		return Path::GetFileNameWithoutExtension(filename) + Path::GetExtension(filename);
	}
	  std::string StartPath() {
		return Path::GetDirectoryName(GetModuleFileName());
	}
	  std::string GetModuleFileName() {
		CHAR exeFullPath[MAX_PATH];
		::GetModuleFileNameA(NULL, exeFullPath, MAX_PATH);
		std::string filename = exeFullPath;
		return filename;
	}
}