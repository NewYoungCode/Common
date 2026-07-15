#include "Socket.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <limits>
#include <utility>

namespace {
	class WinsockRuntime
	{
	public:
		WinsockRuntime()
		{
			code = ::WSAStartup(MAKEWORD(2, 2), &data);
			valid = code == 0 && LOBYTE(data.wVersion) == 2 && HIBYTE(data.wVersion) == 2;
			if (code == 0 && !valid) {
				::WSACleanup();
				code = WSAVERNOTSUPPORTED;
			}
		}

		~WinsockRuntime()
		{
			if (valid) {
				::WSACleanup();
			}
		}

		bool valid = false;
		int code = 0;
		WSADATA data{};
	};

	bool EnsureWinsock()
	{
		static WinsockRuntime runtime;
		if (!runtime.valid) {
			::WSASetLastError(runtime.code);
		}
		return runtime.valid;
	}

	bool SetSocketOption(SOCKET socket, int level, int option, int value)
	{
		return ::setsockopt(socket, level, option,
			reinterpret_cast<const char*>(&value), sizeof(value)) == 0;
	}

	bool SetSocketTimeout(SOCKET socket, int option, DWORD milliseconds)
	{
		return ::setsockopt(socket, SOL_SOCKET, option,
			reinterpret_cast<const char*>(&milliseconds), sizeof(milliseconds)) == 0;
	}

	bool GetSocketTimeout(SOCKET socket, int option, DWORD& milliseconds)
	{
		int length = sizeof(milliseconds);
		return ::getsockopt(socket, SOL_SOCKET, option,
			reinterpret_cast<char*>(&milliseconds), &length) == 0;
	}

	bool ApplySocketTimeouts(SOCKET socket, DWORD receiveMilliseconds,
		DWORD writeMilliseconds)
	{
		return SetSocketTimeout(socket, SO_RCVTIMEO, receiveMilliseconds) &&
			SetSocketTimeout(socket, SO_SNDTIMEO, writeMilliseconds);
	}

	bool ConvertPollTimeout(size_t milliseconds, int& timeout)
	{
		if (milliseconds > static_cast<size_t>((std::numeric_limits<int>::max)())) {
			::WSASetLastError(WSAEINVAL);
			return false;
		}
		timeout = milliseconds == 0 ? -1 : static_cast<int>(milliseconds);
		return true;
	}

	bool ConvertSocketTimeout(size_t milliseconds, DWORD& timeout)
	{
		if (milliseconds > static_cast<size_t>((std::numeric_limits<DWORD>::max)())) {
			::WSASetLastError(WSAEINVAL);
			return false;
		}
		timeout = static_cast<DWORD>(milliseconds);
		return true;
	}

	int ToPollTimeout(DWORD milliseconds)
	{
		if (milliseconds == 0) {
			return -1;
		}
		return static_cast<int>((std::min)(milliseconds,
			static_cast<DWORD>((std::numeric_limits<int>::max)())));
	}

	SOCKET CreateNativeSocket(const addrinfo& address)
	{
		SOCKET socket = ::WSASocketW(address.ai_family, address.ai_socktype,
			address.ai_protocol, NULL, 0,
			WSA_FLAG_NO_HANDLE_INHERIT | WSA_FLAG_OVERLAPPED);
		if (socket == INVALID_SOCKET) {
			socket = ::socket(address.ai_family, address.ai_socktype, address.ai_protocol);
		}
		return socket;
	}

	bool SetNonblocking(SOCKET socket, bool nonblocking)
	{
		u_long mode = nonblocking ? 1UL : 0UL;
		return ::ioctlsocket(socket, FIONBIO, &mode) == 0;
	}

	int PollSocket(SOCKET socket, short events, int timeoutMilliseconds)
	{
		WSAPOLLFD pollFd{};
		pollFd.fd = socket;
		pollFd.events = events;

		for (;;) {
			int result = ::WSAPoll(&pollFd, 1, timeoutMilliseconds);
			if (result != SOCKET_ERROR || ::WSAGetLastError() != WSAEINTR) {
				return result;
			}
		}
	}

	bool WaitUntilConnected(SOCKET socket, int timeoutMilliseconds)
	{
		int result = PollSocket(socket, POLLIN | POLLOUT, timeoutMilliseconds);
		if (result == 0) {
			::WSASetLastError(WSAETIMEDOUT);
			return false;
		}
		if (result == SOCKET_ERROR) {
			return false;
		}

		int error = 0;
		int errorLength = sizeof(error);
		if (::getsockopt(socket, SOL_SOCKET, SO_ERROR,
			reinterpret_cast<char*>(&error), &errorLength) == SOCKET_ERROR) {
			return false;
		}
		if (error != 0) {
			::WSASetLastError(error);
			return false;
		}
		return true;
	}

	bool ConnectNativeSocket(SOCKET socket, const addrinfo& address,
		int timeoutMilliseconds)
	{
		if (!SetNonblocking(socket, true)) {
			return false;
		}

		int result = ::connect(socket, address.ai_addr, static_cast<int>(address.ai_addrlen));
		if (result == SOCKET_ERROR) {
			int error = ::WSAGetLastError();
			if (error != WSAEWOULDBLOCK && error != WSAEINPROGRESS &&
				error != WSAEALREADY) {
				SetNonblocking(socket, false);
				::WSASetLastError(error);
				return false;
			}
			if (!WaitUntilConnected(socket, timeoutMilliseconds)) {
				error = ::WSAGetLastError();
				SetNonblocking(socket, false);
				::WSASetLastError(error);
				return false;
			}
		}

		return SetNonblocking(socket, false);
	}

	bool GetEndpoint(const sockaddr* address, int addressLength,
		std::string& ip, size_t& port)
	{
		char host[NI_MAXHOST]{};
		char service[NI_MAXSERV]{};
		if (::getnameinfo(address, addressLength, host, sizeof(host), service,
			sizeof(service), NI_NUMERICHOST | NI_NUMERICSERV) != 0) {
			return false;
		}

		ip = host;
		port = static_cast<size_t>(std::strtoul(service, NULL, 10));
		return true;
	}

	bool GetSocketEndpoint(SOCKET socket, bool peer, std::string& ip, size_t& port)
	{
		sockaddr_storage address{};
		int addressLength = sizeof(address);
		int result = peer
			? ::getpeername(socket, reinterpret_cast<sockaddr*>(&address), &addressLength)
			: ::getsockname(socket, reinterpret_cast<sockaddr*>(&address), &addressLength);
		return result == 0 && GetEndpoint(reinterpret_cast<sockaddr*>(&address),
			addressLength, ip, port);
	}

	int ReceiveNative(SOCKET socket, char* buffer, size_t size, int flags)
	{
		size = (std::min)(size, static_cast<size_t>((std::numeric_limits<int>::max)()));
		for (;;) {
			int result = ::recv(socket, buffer, static_cast<int>(size), flags);
			if (result != SOCKET_ERROR || ::WSAGetLastError() != WSAEINTR) {
				return result;
			}
		}
	}

	int SendNative(SOCKET socket, const char* buffer, int size)
	{
		for (;;) {
			int result = ::send(socket, buffer, size, 0);
			if (result != SOCKET_ERROR || ::WSAGetLastError() != WSAEINTR) {
				return result;
			}
		}
	}

	void CloseNativeSocket(SOCKET socket, bool shutdownFirst)
	{
		if (socket == INVALID_SOCKET) {
			return;
		}
		if (shutdownFirst) {
			::shutdown(socket, SD_BOTH);
		}
		::closesocket(socket);
	}

	int RemainingTimeout(int timeoutMilliseconds,
		const std::chrono::steady_clock::time_point& start)
	{
		if (timeoutMilliseconds < 0) {
			return -1;
		}
		const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - start).count();
		if (elapsed >= timeoutMilliseconds) {
			return 0;
		}
		return timeoutMilliseconds - static_cast<int>(elapsed);
	}
}

std::vector<std::string> Socket::ResolveHostname(const std::string& hostname)
{
	std::vector<std::string> addresses;
	if (!EnsureWinsock() || hostname.empty()) {
		return addresses;
	}

	addrinfo hints{};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	addrinfo* result = NULL;
	int error = ::getaddrinfo(hostname.c_str(), NULL, &hints, &result);
	if (error != 0) {
		::WSASetLastError(error);
		return addresses;
	}

	for (addrinfo* current = result; current; current = current->ai_next) {
		std::string ip;
		size_t port = 0;
		if (GetEndpoint(current->ai_addr, static_cast<int>(current->ai_addrlen), ip, port) &&
			std::find(addresses.begin(), addresses.end(), ip) == addresses.end()) {
			addresses.push_back(ip);
		}
	}
	::freeaddrinfo(result);
	return addresses;
}

int Socket::Receive(char* outBuf, size_t recvLen, int flags) const
{
	if (socket_ == INVALID_SOCKET) {
		::WSASetLastError(WSAENOTSOCK);
		return SOCKET_ERROR;
	}
	if (!outBuf && recvLen > 0) {
		::WSASetLastError(WSAEFAULT);
		return SOCKET_ERROR;
	}
	if (recvLen == 0) {
		return 0;
	}

	for (;;) {
		int received = ReceiveNative(socket_, outBuf, recvLen, flags);
		if (received != SOCKET_ERROR || ::WSAGetLastError() != WSAEWOULDBLOCK) {
			return received;
		}

		int result = PollSocket(socket_, POLLIN, ToPollTimeout(receiveTimeoutMilliseconds_));
		if (result == 0) {
			::WSASetLastError(WSAETIMEDOUT);
			return SOCKET_ERROR;
		}
		if (result == SOCKET_ERROR) {
			return SOCKET_ERROR;
		}
	}
}

bool Socket::Connect(const std::string& ip, size_t port)
{
	Close();
	address_.clear();
	port_ = 0;
	if (!EnsureWinsock()) {
		return false;
	}
	if (ip.empty() || port > 65535) {
		::WSASetLastError(WSAEINVAL);
		return false;
	}

	addrinfo hints{};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	addrinfo* result = NULL;
	std::string service = std::to_string(port);
	int error = ::getaddrinfo(ip.c_str(), service.c_str(), &hints, &result);
	if (error != 0) {
		::WSASetLastError(error);
		return false;
	}

	int lastError = WSAECONNREFUSED;
	for (addrinfo* current = result; current; current = current->ai_next) {
		SOCKET candidate = CreateNativeSocket(*current);
		if (candidate == INVALID_SOCKET) {
			lastError = ::WSAGetLastError();
			continue;
		}

		if (ConnectNativeSocket(candidate, *current, connectionTimeoutMilliseconds_) &&
			ApplySocketTimeouts(candidate, receiveTimeoutMilliseconds_,
				writeTimeoutMilliseconds_)) {
			socket_ = candidate;
			if (!GetSocketEndpoint(candidate, true, address_, port_)) {
				address_ = ip;
				port_ = port;
			}
			::freeaddrinfo(result);
			return true;
		}

		lastError = ::WSAGetLastError();
		CloseNativeSocket(candidate, false);
	}

	::freeaddrinfo(result);
	::WSASetLastError(lastError);
	return false;
}

bool Socket::Bind(const std::string& ip, size_t port)
{
	Close();
	address_.clear();
	port_ = 0;
	if (!EnsureWinsock()) {
		return false;
	}
	if (port > 65535) {
		::WSASetLastError(WSAEINVAL);
		return false;
	}

	addrinfo hints{};
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	addrinfo* result = NULL;
	std::string service = std::to_string(port);
	const char* host = ip.empty() ? NULL : ip.c_str();
	int error = ::getaddrinfo(host, service.c_str(), &hints, &result);
	if (error != 0) {
		::WSASetLastError(error);
		return false;
	}

	int lastError = WSAEADDRNOTAVAIL;
	for (addrinfo* current = result; current; current = current->ai_next) {
		SOCKET candidate = CreateNativeSocket(*current);
		if (candidate == INVALID_SOCKET) {
			lastError = ::WSAGetLastError();
			continue;
		}

		SetSocketOption(candidate, SOL_SOCKET, SO_REUSEADDR, 1);
		if (current->ai_family == AF_INET6) {
			SetSocketOption(candidate, IPPROTO_IPV6, IPV6_V6ONLY, 0);
		}

		if (::bind(candidate, current->ai_addr,
			static_cast<int>(current->ai_addrlen)) == 0) {
			socket_ = candidate;
			if (!GetSocketEndpoint(candidate, false, address_, port_)) {
				address_ = ip;
				port_ = port;
			}
			::freeaddrinfo(result);
			return true;
		}

		lastError = ::WSAGetLastError();
		CloseNativeSocket(candidate, false);
	}

	::freeaddrinfo(result);
	::WSASetLastError(lastError);
	return false;
}

bool Socket::Listen(int backlog)
{
	if (socket_ == INVALID_SOCKET) {
		::WSASetLastError(WSAENOTSOCK);
		return false;
	}
	if (!SetNonblocking(socket_, true)) {
		return false;
	}
	return ::listen(socket_, backlog) == 0;
}

Socket Socket::Accep() const
{
	if (socket_ == INVALID_SOCKET) {
		::WSASetLastError(WSAENOTSOCK);
		return Socket();
	}
	if (!SetNonblocking(socket_, true)) {
		return Socket();
	}

	const auto start = std::chrono::steady_clock::now();
	for (;;) {
		const int remaining = RemainingTimeout(acceptTimeoutMilliseconds_, start);
		if (remaining == 0) {
			::WSASetLastError(WSAETIMEDOUT);
			return Socket();
		}

		int result = PollSocket(socket_, POLLIN, remaining);
		if (result == 0) {
			::WSASetLastError(WSAETIMEDOUT);
			return Socket();
		}
		if (result == SOCKET_ERROR) {
			return Socket();
		}

		sockaddr_storage clientAddress{};
		int addressLength = sizeof(clientAddress);
		SOCKET client = ::WSAAccept(socket_,
			reinterpret_cast<sockaddr*>(&clientAddress), &addressLength, NULL, 0);
		if (client != INVALID_SOCKET) {
			if (!SetNonblocking(client, false) ||
				!ApplySocketTimeouts(client, receiveTimeoutMilliseconds_,
					writeTimeoutMilliseconds_)) {
				int error = ::WSAGetLastError();
				CloseNativeSocket(client, false);
				::WSASetLastError(error);
				return Socket();
			}

			Socket accepted(client);
			GetEndpoint(reinterpret_cast<sockaddr*>(&clientAddress), addressLength,
				accepted.address_, accepted.port_);
			return accepted;
		}

		int error = ::WSAGetLastError();
		if (error == WSAEINTR || error == WSAEWOULDBLOCK) {
			continue;
		}
		return Socket();
	}
}

int Socket::Write(const char* buff, int size) const
{
	if (size < 0 || (!buff && size > 0)) {
		::WSASetLastError(WSAEINVAL);
		return SOCKET_ERROR;
	}
	if (size == 0) {
		return 0;
	}
	if (socket_ == INVALID_SOCKET) {
		::WSASetLastError(WSAENOTSOCK);
		return SOCKET_ERROR;
	}

	int total = 0;
	while (total < size) {
		int sent = SendNative(socket_, buff + total, size - total);
		if (sent > 0) {
			total += sent;
			continue;
		}
		if (sent == 0) {
			break;
		}

		int error = ::WSAGetLastError();
		if (error != WSAEWOULDBLOCK) {
			return total > 0 ? total : SOCKET_ERROR;
		}

		int result = PollSocket(socket_, POLLOUT, ToPollTimeout(writeTimeoutMilliseconds_));
		if (result == 0) {
			::WSASetLastError(WSAETIMEDOUT);
			return total > 0 ? total : SOCKET_ERROR;
		}
		if (result == SOCKET_ERROR) {
			return total > 0 ? total : SOCKET_ERROR;
		}
	}
	return total;
}

void Socket::Close() const
{
	if (socket_ == INVALID_SOCKET) {
		return;
	}
	SOCKET socket = socket_;
	socket_ = INVALID_SOCKET;
	CloseNativeSocket(socket, true);
}

bool Socket::IsValid() const
{
	return socket_ != INVALID_SOCKET;
}

size_t Socket::GetPort() const
{
	return port_;
}

const std::string& Socket::GetAddress() const
{
	return address_;
}

bool Socket::SetConnectionTimeout(size_t milliseconds)
{
	int timeout = 0;
	if (!ConvertPollTimeout(milliseconds, timeout)) {
		return false;
	}
	connectionTimeoutMilliseconds_ = timeout;
	return true;
}

bool Socket::SetReceiveTimeout(size_t milliseconds)
{
	DWORD timeout = 0;
	if (!ConvertSocketTimeout(milliseconds, timeout)) {
		return false;
	}
	if (socket_ != INVALID_SOCKET && !SetSocketTimeout(socket_, SO_RCVTIMEO, timeout)) {
		return false;
	}
	receiveTimeoutMilliseconds_ = timeout;
	return true;
}

bool Socket::SetWriteTimeout(size_t milliseconds)
{
	DWORD timeout = 0;
	if (!ConvertSocketTimeout(milliseconds, timeout)) {
		return false;
	}
	if (socket_ != INVALID_SOCKET && !SetSocketTimeout(socket_, SO_SNDTIMEO, timeout)) {
		return false;
	}
	writeTimeoutMilliseconds_ = timeout;
	return true;
}

bool Socket::SetAcceptTimeout(size_t milliseconds)
{
	int timeout = 0;
	if (!ConvertPollTimeout(milliseconds, timeout)) {
		return false;
	}
	acceptTimeoutMilliseconds_ = timeout;
	return true;
}

Socket::Socket()
{
	EnsureWinsock();
}

Socket::Socket(SOCKET socket)
{
	if (!EnsureWinsock()) {
		int error = ::WSAGetLastError();
		CloseNativeSocket(socket, false);
		::WSASetLastError(error);
		return;
	}
	if (socket == INVALID_SOCKET) {
		return;
	}

	socket_ = socket;
	DWORD timeout = 0;
	if (GetSocketTimeout(socket, SO_RCVTIMEO, timeout)) {
		receiveTimeoutMilliseconds_ = timeout;
	}
	if (GetSocketTimeout(socket, SO_SNDTIMEO, timeout)) {
		writeTimeoutMilliseconds_ = timeout;
	}
	if (!GetSocketEndpoint(socket, true, address_, port_)) {
		GetSocketEndpoint(socket, false, address_, port_);
	}
}

Socket::Socket(Socket&& other) noexcept
	: socket_(other.socket_),
	connectionTimeoutMilliseconds_(other.connectionTimeoutMilliseconds_),
	receiveTimeoutMilliseconds_(other.receiveTimeoutMilliseconds_),
	writeTimeoutMilliseconds_(other.writeTimeoutMilliseconds_),
	acceptTimeoutMilliseconds_(other.acceptTimeoutMilliseconds_),
	port_(other.port_), address_(std::move(other.address_))
{
	other.socket_ = INVALID_SOCKET;
	other.port_ = 0;
	other.address_.clear();
}

Socket& Socket::operator=(Socket&& other) noexcept
{
	if (this != &other) {
		Close();
		socket_ = other.socket_;
		connectionTimeoutMilliseconds_ = other.connectionTimeoutMilliseconds_;
		receiveTimeoutMilliseconds_ = other.receiveTimeoutMilliseconds_;
		writeTimeoutMilliseconds_ = other.writeTimeoutMilliseconds_;
		acceptTimeoutMilliseconds_ = other.acceptTimeoutMilliseconds_;
		port_ = other.port_;
		address_ = std::move(other.address_);
		other.socket_ = INVALID_SOCKET;
		other.port_ = 0;
		other.address_.clear();
	}
	return *this;
}

Socket::~Socket()
{
	Close();
}
