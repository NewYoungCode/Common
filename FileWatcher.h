#pragma once
#include <string>
#include <iostream>
#include <thread>
#include <Windows.h>
#include "Common.h"
#include <functional>
#include <vector>
class FileWatcher {
private:
	std::string math = "*.*";
	std::string path;
	std::function<void(const std::string& filename)> callback = NULL;
	size_t sleep = 500;
	bool gogogo = true;
	void TaskFunc();
public:
	FileWatcher(const std::string& path, const std::string& math, const  std::function<void(const std::string& filename)>& callback, size_t sleep=500);
	~FileWatcher();
};
