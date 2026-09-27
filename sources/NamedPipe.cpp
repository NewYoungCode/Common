#include "NamedPipe.h"

#include <algorithm>
#include <limits>
#include <utility>

namespace {
	std::wstring MakePipePath(const std::wstring& pipeName)
	{
		return L"\\\\.\\pipe\\" + pipeName;
	}

	DWORD ToDwordSize(size_t size)
	{
		return static_cast<DWORD>((std::min)(size,
			static_cast<size_t>((std::numeric_limits<DWORD>::max)())));
	}
}

bool NamedPipe::CreateServer(const std::wstring& pipeName,
	DWORD maxInstances, DWORD bufferSize)
{
	Close();
	if (pipeName.empty() || maxInstances == 0) {
		::SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	std::wstring path = MakePipePath(pipeName);
	pipe_ = ::CreateNamedPipeW(path.c_str(), PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
		maxInstances, bufferSize, bufferSize, 0, NULL);
	if (pipe_ == INVALID_HANDLE_VALUE) {
		return false;
	}
	server_ = true;
	return true;
}

bool NamedPipe::WaitForClient()
{
	if (pipe_ == INVALID_HANDLE_VALUE || !server_) {
		::SetLastError(ERROR_INVALID_HANDLE);
		return false;
	}
	if (connected_) {
		return true;
	}

	if (::ConnectNamedPipe(pipe_, NULL)) {
		connected_ = true;
		return true;
	}
	if (::GetLastError() == ERROR_PIPE_CONNECTED) {
		connected_ = true;
		return true;
	}
	return false;
}

bool NamedPipe::ConnectClient(const std::wstring& pipeName,
	DWORD timeoutMilliseconds)
{
	Close();
	if (pipeName.empty()) {
		::SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	std::wstring path = MakePipePath(pipeName);
	if (!::WaitNamedPipeW(path.c_str(), timeoutMilliseconds)) {
		return false;
	}

	pipe_ = ::CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, 0, NULL);
	if (pipe_ == INVALID_HANDLE_VALUE) {
		return false;
	}
	connected_ = true;
	return true;
}

bool NamedPipe::Read(void* buffer, size_t size, size_t& bytesRead) const
{
	bytesRead = 0;
	if (pipe_ == INVALID_HANDLE_VALUE) {
		::SetLastError(ERROR_INVALID_HANDLE);
		return false;
	}
	if (!buffer && size > 0) {
		::SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}
	if (size == 0) {
		return true;
	}

	DWORD read = 0;
	if (!::ReadFile(pipe_, buffer, ToDwordSize(size), &read, NULL)) {
		return false;
	}
	bytesRead = read;
	return true;
}

bool NamedPipe::ReadExact(void* buffer, size_t size) const
{
	if (!buffer && size > 0) {
		::SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	char* out = static_cast<char*>(buffer);
	size_t totalRead = 0;
	while (totalRead < size) {
		size_t bytesRead = 0;
		if (!Read(out + totalRead, size - totalRead, bytesRead) || bytesRead == 0) {
			return false;
		}
		totalRead += bytesRead;
	}
	return true;
}

bool NamedPipe::Write(const void* buffer, size_t size) const
{
	if (pipe_ == INVALID_HANDLE_VALUE) {
		::SetLastError(ERROR_INVALID_HANDLE);
		return false;
	}
	if (!buffer && size > 0) {
		::SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	const char* data = static_cast<const char*>(buffer);
	size_t totalWritten = 0;
	while (totalWritten < size) {
		DWORD written = 0;
		DWORD chunkSize = ToDwordSize(size - totalWritten);
		if (!::WriteFile(pipe_, data + totalWritten, chunkSize, &written, NULL) ||
			written == 0) {
			return false;
		}
		totalWritten += written;
	}
	return true;
}

void NamedPipe::Close()
{
	if (pipe_ == INVALID_HANDLE_VALUE) {
		return;
	}
	if (server_ && connected_) {
		::DisconnectNamedPipe(pipe_);
	}
	::CloseHandle(pipe_);
	pipe_ = INVALID_HANDLE_VALUE;
	server_ = false;
	connected_ = false;
}

bool NamedPipe::IsValid() const
{
	return pipe_ != INVALID_HANDLE_VALUE;
}

NamedPipe::NamedPipe(NamedPipe&& other) noexcept
	: pipe_(other.pipe_), server_(other.server_), connected_(other.connected_)
{
	other.pipe_ = INVALID_HANDLE_VALUE;
	other.server_ = false;
	other.connected_ = false;
}

NamedPipe& NamedPipe::operator=(NamedPipe&& other) noexcept
{
	if (this != &other) {
		Close();
		pipe_ = other.pipe_;
		server_ = other.server_;
		connected_ = other.connected_;
		other.pipe_ = INVALID_HANDLE_VALUE;
		other.server_ = false;
		other.connected_ = false;
	}
	return *this;
}

NamedPipe::~NamedPipe()
{
	Close();
}
