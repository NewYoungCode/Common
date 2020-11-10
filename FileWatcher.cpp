#include "FileWatcher.h"
void FileWatcher::TaskFunc()
{
	std::vector< std::string>	files = Path::SearchFiles(path, math.c_str());
	for (; exit; )
	{
		//移除不存在的文件
		for (size_t i = 0; i < files.size(); i++)
		{
			if (!File::Exists(files[i]))
			{
				std::vector< std::string>::iterator it = std::find(files.begin(), files.end(), files[i]);
				if (it != files.end()) {
					files.erase(it);
				}
			}
		}

		//判断是否新增的文件s
		std::vector< std::string> tmp = Path::SearchFiles(path, math.c_str());
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
		Sleep(500);
	}
}

FileWatcher::FileWatcher(const std::string& path, const std::string& math, const  std::function<void(const std::string& filename)>& callback, size_t sleep)
{
	this->sleep = sleep;
	this->callback = callback;
	this->path = path;
	this->math = math;
	TaskFunc();
}


FileWatcher::~FileWatcher()
{
	gogogo = false;
}
