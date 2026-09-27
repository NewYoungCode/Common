#pragma once

#include <cstddef>
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

// 同步命名管道封装，用于传输字节流数据。
// 管道名称只需传入名称本身，不包含 \\.\pipe\ 前缀。
class NamedPipe
{
private:
	HANDLE pipe_ = INVALID_HANDLE_VALUE;
	bool server_ = false;
	bool connected_ = false;

public:
	// 创建服务端管道实例；可设置同名实例数量和缓冲区大小，创建后等待客户端连接。
	bool CreateServer(const std::wstring& pipeName,
		DWORD maxInstances = 1, DWORD bufferSize = 4096);
	// 阻塞等待客户端连接；仅服务端管道可调用。
	bool WaitForClient();

	// 连接服务端管道；服务端必须已创建管道，等待时间由参数指定。
	bool ConnectClient(const std::wstring& pipeName,
		DWORD timeoutMilliseconds = 5000);

	// 最多读取指定长度的数据；成功后返回实际读取长度，可能小于请求长度。
	bool Read(void* buffer, size_t size, size_t& bytesRead) const;
	// 阻塞读取直到收到指定长度的数据；调用方需预先知道数据长度。
	bool ReadExact(void* buffer, size_t size) const;
	// 写入指定长度的全部数据；底层分多次写入时会继续写完。
	bool Write(const void* buffer, size_t size) const;

	// 关闭管道并释放系统句柄。
	void Close();
	// 判断管道句柄是否有效。
	bool IsValid() const;

	// 创建未连接的管道对象。
	NamedPipe() = default;
	// 禁止复制，避免多个对象重复释放同一个系统句柄。
	NamedPipe(const NamedPipe&) = delete;
	NamedPipe& operator=(const NamedPipe&) = delete;
	// 移动管道所有权。
	NamedPipe(NamedPipe&& other) noexcept;
	NamedPipe& operator=(NamedPipe&& other) noexcept;
	// 析构时自动关闭管道。
	~NamedPipe();
};
