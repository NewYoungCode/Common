#pragma once
#include <thread>
class Thread {
	bool bStop = true;
	bool bBreak = false;
	std::thread* task = NULL;
public:
	template<class F, class... Args>
	Thread(F&& f, Args&& ...args) {
		task = new std::thread([=]() mutable {
			bStop = false;
			std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
			bStop = true;
			});
	}
	virtual ~Thread() {
		if (task) {
			delete task;
		}
	}
	std::thread* Get() {
		return task;
	}
	bool Joinable() {
		return task->joinable();
	}
	void Detach() {
		task->detach();
	}
	void Join() {
		task->join();
	}
	bool IsStop() {
		return bStop;
	}
};