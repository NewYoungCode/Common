#pragma once
#include <vector>
#include <iostream>
#include <string>
#include <memory>
#include <mutex>
#include "Socket.h"

std::mutex __socket_mtx;
WSADATA __wsadata__;
bool __socket_init = false;
//初始化套接字库
bool __func_socket_init() {
	std::unique_lock<std::mutex> autoLock(__socket_mtx);
	if (__socket_init == false) {
		int code = ::WSAStartup(MAKEWORD(2, 2), &__wsadata__);
		if (code == 0) {
			__socket_init = true;
		}
		else {
			printf("__func_socket_init failed: %d\n", code);
		}
	}
	return __socket_init;
}
//清理
void __func_socket_cleanup() {
	::WSACleanup();
	__wsadata__.wVersion = 0;
	__socket_init = false;
}

// 获取某个 SOCKET 的 sockaddr 信息（本地或远端，自动判断）
// 返回值：0 = 成功（优先返回远端地址），1 = 返回本地地址，-1 = 失败
int __func_get_sockaddr(SOCKET sock, sockaddr_in& outAddr) {
	int len = sizeof(sockaddr_in);
	memset(&outAddr, 0, len);
	// 优先获取远端地址
	if (getpeername(sock, (sockaddr*)&outAddr, &len) == 0) {
		return 0; // 是远端地址
	}
	// 如果是本地 socket（如 listen socket），则 getpeername 会失败
	if (getsockname(sock, (sockaddr*)&outAddr, &len) == 0) {
		return 1; // 是本地地址
	}
	return -1; // 都失败了（无效 socket）
}

std::vector<std::string> Socket::GetIpByName(const std::string& hostname) {
	__func_socket_init();
	std::vector<std::string> ipList;
	HOSTENT* pHostEnt;
	int             nAdapter = 0;
	struct hostent* host = ::gethostbyname(hostname.c_str());
	for (int i = 0; host && host->h_addr_list[i]; i++)
	{
		char* ip = inet_ntoa(*(struct in_addr*)host->h_addr_list[i]);
		ipList.emplace_back(ip);
	}
	return ipList;
}
int Socket::Receive(char* outBuf, size_t recvLen, int flags) const
{
	return ::recv(socket, outBuf, recvLen, 0);
}
bool Socket::Connect(const std::string& ip, size_t port)
{
	sockaddr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
	sockaddr.sin_port = htons(port);
	//创建套接字
	socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connect(socket, (SOCKADDR*)&sockaddr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		printf("%s\n", "Connection failed");
		return false;
	}
	else {
		return true;
	}
}
bool Socket::Bind(const std::string& ip, size_t port) {
	//TCP的

	socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket == INVALID_SOCKET) {
		wprintf(L"socket function failed with error: %u\n", WSAGetLastError());
		return false;
	}
	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port for the socket that is being bound.
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = inet_addr(ip.c_str());
	sockaddr.sin_port = htons(port);
	//----------------------
	// Bind the socket.
	if (bind(socket, (SOCKADDR*)&sockaddr, sizeof(sockaddr)) == SOCKET_ERROR) {
		wprintf(L"bind failed with error %u\n", WSAGetLastError());
		closesocket(socket);
		return false;
	}
	else
	{
		this->Port = port;
		this->Address = ip;
		return true;
	}
	return false;
}
bool Socket::Listen(int backlog) {
	return !::listen(socket, backlog);
}
Socket Socket::Accep() const {
	for (;;) {
		sockaddr_in  ClientAddr{ 0 };
		int  AddrLen = sizeof(ClientAddr);
		SOCKET clientSocket = accept(socket, (LPSOCKADDR)&ClientAddr, &AddrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			printf("INVALID_SOCKET\n");
			return Socket(NULL);
		}
		Socket skt(clientSocket);
		skt.Port = ClientAddr.sin_port;
		char* c_address = new char[15] { 0 };
		sprintf_s(c_address, 14, "%d.%d.%d.%d", ClientAddr.sin_addr.S_un.S_un_b.s_b1, ClientAddr.sin_addr.S_un.S_un_b.s_b2, ClientAddr.sin_addr.S_un.S_un_b.s_b3, ClientAddr.sin_addr.S_un.S_un_b.s_b4);
		skt.Address = c_address;
		delete[] c_address;
		return skt;
	}
	return NULL;
}
int Socket::Write(const char* buff, int size) const
{
	int totalCount = 0;
	const char* ptr = buff;
	for (; totalCount != size;) {
		int result = ::send(socket, buff, size - totalCount, 0);
		if (result == SOCKET_ERROR) {
			printf("send failed: %d\n", WSAGetLastError());
			break;
		}
		else {
			ptr += result;
			totalCount += result;
		}
	}
	return totalCount;
}
void Socket::Close() const {
	::closesocket(this->socket);
}
Socket::Socket()
{
	sockaddr.sin_family = AF_INET;
	__func_socket_init();
}
Socket::Socket(SOCKET socket)
{
	this->socket = socket;
	__func_get_sockaddr(socket, sockaddr);
}
Socket::~Socket()
{

}

