#include "Time.hpp"
namespace Time {
	namespace Now {
		std::string ToString(const std::string& format) {
			SYSTEMTIME time;
			GetLocalTime(&time);
			std::string  year = std::to_string(time.wYear);//��
			std::string  Month = std::to_string(time.wMonth);//��
			std::string  Day = std::to_string(time.wDay);//��
			std::string Hour = std::to_string(time.wHour);//ʱ
			std::string Minute = std::to_string(time.wMinute);//��
			std::string Second = std::to_string(time.wSecond);//��
			std::string wMilliseconds = std::to_string(time.wMilliseconds);//����
			Month = Month.size() == 1U ? "0" + Month : Month;
			Day = Day.size() == 1U ? "0" + Day : Day;
			Hour = Hour.size() == 1U ? "0" + Hour : Hour;
			Minute = Minute.size() == 1U ? "0" + Minute : Minute;
			Second = Second.size() == 1U ? "0" + Second : Second;
			Text::Utf8String formatStr = format;
			formatStr = formatStr.Replace("yyyy", year);
			formatStr = formatStr.Replace("MM", Month);
			formatStr = formatStr.Replace("dd", Day);
			formatStr = formatStr.Replace("hh", Hour);
			formatStr = formatStr.Replace("mm", Minute);
			formatStr = formatStr.Replace("ss", Second);
			formatStr = formatStr.Replace("mmmm", wMilliseconds);
			return formatStr;
		}
	}
}