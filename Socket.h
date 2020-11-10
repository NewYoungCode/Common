#pragma once
#include<iostream>
#include <string>
#include<winsock.h>
#pragma comment(lib,"ws2_32.lib")
static  bool init_socket_ = false;
class Socket
{
private:
	WSADATA wsadata;
	sockaddr_in sockaddr;
	SOCKET socket = NULL;
public:
	size_t Port = 0;
	std::string Address;
	size_t Receive(char* outBuf, size_t recvLen, int flags = 0) const;
	bool Connect(const std::string& ip, size_t port);
	bool Bind(const std::string & ip, size_t port);
	bool Listen(int backlog);
	Socket Accep() const;
	size_t Write(const char* buff, int size)const;
	void Close() const;
	Socket();
	Socket(SOCKET socket);
	~Socket();
};
