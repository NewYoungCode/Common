#include "Socket.h"
  size_t Socket::Receive(char* outBuf, size_t recvLen, int flags) const
{
	recvLen = ::recv(socket, outBuf, recvLen - 1, 0);
	return recvLen;
}
  bool Socket::Connect(const std::string& ip, size_t port)
{
	sockaddr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
	sockaddr.sin_port = htons(port);
	//创建套接字
	socket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (connect(socket, (SOCKADDR *)&sockaddr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		printf("%s\n", "Connection failed");
		::WSACleanup();
		return false;
	}
	else {
		return true;
	}
}
  bool Socket::Bind(const std::string& ip, size_t port) {
	int iResult = 0;
	// The socket address to be passed to bind
	//----------------------
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (iResult != NO_ERROR) {
		wprintf(L"Error at WSAStartup()\n");
		return false;
	}
	//----------------------
	// Create a SOCKET for listening for 
	// incoming connection requests
	socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket == INVALID_SOCKET) {
		wprintf(L"socket function failed with error: %u\n", WSAGetLastError());
		WSACleanup();
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
	iResult = bind(socket, (SOCKADDR *)&sockaddr, sizeof(sockaddr));
	if (iResult == SOCKET_ERROR) {
		wprintf(L"bind failed with error %u\n", WSAGetLastError());
		closesocket(socket);
		WSACleanup();
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
	return ::listen(socket, backlog);
}
  Socket Socket::Accep() const {
	for (;;) {
		sockaddr_in  ClientAddr;
		int  AddrLen = sizeof(ClientAddr);
		SOCKET clientSocket = accept(socket, (LPSOCKADDR)&ClientAddr, &AddrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			continue;
		}
		Socket s(clientSocket);
		s.Port = ClientAddr.sin_port;
		s.Address.clear();
		s.Address.append(std::to_string((size_t)ClientAddr.sin_addr.S_un.S_un_b.s_b1));
		s.Address.push_back('.');
		s.Address.append(std::to_string((size_t)ClientAddr.sin_addr.S_un.S_un_b.s_b2));
		s.Address.push_back('.');
		s.Address.append(std::to_string((size_t)ClientAddr.sin_addr.S_un.S_un_b.s_b3));
		s.Address.push_back('.');
		s.Address.append(std::to_string((size_t)ClientAddr.sin_addr.S_un.S_un_b.s_b4));
		return s;
	}
	return NULL;
}
  size_t Socket::Write(const char * buff, int size) const
{
	int sl = ::send(socket, buff, size, 0);
	return sl;
}
  void Socket::Close() const {
	::closesocket(this->socket);
}
  Socket::Socket()
{
	if (!init_socket_) {
		//初始化套接字库
		sockaddr.sin_family = AF_INET;
		int err;
		WORD w_req = MAKEWORD(2, 2);//版本号
		err = WSAStartup(w_req, &wsadata);
		if (err != 0) {
			printf("%s\n", "Initialization socket faild");
			::WSACleanup();
			return;
		}
		//检测版本号
		if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
			printf("%s\n", "Socket version does not match");
			::WSACleanup();
			return;
		}
		init_socket_ = true;
	}
}
  Socket::Socket(SOCKET socket)
{
	this->socket = socket;
}
  Socket::~Socket()
{

}
;