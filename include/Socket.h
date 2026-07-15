#pragma once
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif

class Socket
{
private:
	mutable SOCKET socket_ = INVALID_SOCKET;
	int connectionTimeoutMilliseconds_ = 30000;
	DWORD receiveTimeoutMilliseconds_ = 300000;
	DWORD writeTimeoutMilliseconds_ = 30000;
	int acceptTimeoutMilliseconds_ = -1;
	size_t port_ = 0;
	std::string address_;
public:
	// 解析主机名,返回其对应的所有IP地址(自动去重)
	// hostname: 待解析的主机名或域名
	// 返回值: 成功返回IP地址列表(可能为多个),解析失败或参数为空则返回空vector
	static std::vector<std::string> ResolveHostname(const std::string& hostname);
public:
	int Receive(char* outBuf, size_t recvLen, int flags = 0) const;
	bool Connect(const std::string& ip, size_t port);
	bool Bind(const std::string& ip, size_t port);
	bool Listen(int backlog = 5);
	Socket Accep() const;
	int Write(const char* buff, int size) const;
	void Close() const;
	bool IsValid() const;
	size_t GetPort() const;
	const std::string& GetAddress() const;
	// A zero timeout waits indefinitely.
	bool SetConnectionTimeout(size_t milliseconds);
	bool SetReceiveTimeout(size_t milliseconds);
	bool SetWriteTimeout(size_t milliseconds);
	bool SetAcceptTimeout(size_t milliseconds);
	Socket();
	// Takes ownership of the native socket.
	Socket(SOCKET socket);
	Socket(const Socket&) = delete;
	Socket& operator=(const Socket&) = delete;
	Socket(Socket&& other) noexcept;
	Socket& operator=(Socket&& other) noexcept;
	virtual ~Socket();
};
