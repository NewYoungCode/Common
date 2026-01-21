#include "DateTime.h"

DateTime DateTime::Now() {
	return DateTime(::time(NULL));
}

DateTime::DateTime(time_t second) {
	m_time = second;
	m_data_str = ToString("yyyy-MM-dd HH:mm:ss");
}

DateTime::DateTime(const std::string& timeStr, const std::string& format) {
	std::string formatStr = format;
	Text::Replace(&formatStr, "yyyy", "%Y");
	Text::Replace(&formatStr, "MM", "%m");
	Text::Replace(&formatStr, "dd", "%d");
	Text::Replace(&formatStr, "HH", "%H");
	Text::Replace(&formatStr, "mm", "%M");
	Text::Replace(&formatStr, "ss", "%S");

	std::tm tm{};
	std::istringstream ss(timeStr);
	ss >> std::get_time(&tm, formatStr.c_str());
	tm.tm_isdst = -1; // Enable daylight saving time

	std::time_t timeStamp = std::mktime(&tm);
	if (timeStamp == -1) {
		throw std::runtime_error("Error converting time string to timestamp.");
	}
	this->m_time = timeStamp;
	m_data_str = ToString("yyyy-MM-dd HH:mm:ss");
}

std::string DateTime::ToString(const std::string& format) const {
	std::string formatStr = format;
	Text::Replace(&formatStr, "yyyy", "%Y");
	Text::Replace(&formatStr, "MM", "%m");
	Text::Replace(&formatStr, "dd", "%d");
	Text::Replace(&formatStr, "HH", "%H");
	Text::Replace(&formatStr, "mm", "%M");
	Text::Replace(&formatStr, "ss", "%S");

	char timeStr[128]{ 0 };
	time_t t = m_time;
	struct tm* ttime = std::localtime(&t);
	std::strftime(timeStr, sizeof(timeStr), formatStr.c_str(), ttime);
	return timeStr;
}

time_t DateTime::operator - (const DateTime& right) {
	return this->m_time - right.m_time;
}

bool DateTime::operator == (const DateTime& right) {
	return  this->m_time == right.m_time;
}

bool DateTime::operator != (const DateTime& right) {
	return  this->m_time != right.m_time;
}

bool DateTime::operator > (const DateTime& right) {
	return  this->m_time > right.m_time;
}

bool DateTime::operator < (const DateTime& right) {
	return  this->m_time < right.m_time;
}

DateTime& DateTime::operator += (time_t right) {
	this->m_time += right;
	return  *this;
}

DateTime& DateTime::operator -= (time_t right) {
	this->m_time -= right;
	return  *this;
}

DateTime DateTime::operator + (time_t right) {
	return  DateTime(this->m_time + right);
}

DateTime DateTime::operator - (time_t right) {
	return  DateTime(this->m_time - right);
}
