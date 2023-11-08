#include "HttpUtility.h"


namespace HttpUtility {

	unsigned char __base64__alphabet_map[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	unsigned char __base64__reverse_map[] = {
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 62, 255, 255, 255, 63,
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 255, 255, 255, 255, 255, 255,
		255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255, 255,
		255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 255, 255, 255, 255, 255
	};

	std::string base64_encode(const unsigned char* text, unsigned int text_len)
	{
		unsigned int i, j;
		std::string base64Str;
		for (i = 0, j = 0; i + 3 <= text_len; i += 3)
		{
			base64Str += __base64__alphabet_map[text[i] >> 2];                             //取出第一个字符的前6位并找出对应的结果字符
			base64Str += __base64__alphabet_map[((text[i] << 4) & 0x30) | (text[i + 1] >> 4)];     //将第一个字符的后2位与第二个字符的前4位进行组合并找到对应的结果字符
			base64Str += __base64__alphabet_map[((text[i + 1] << 2) & 0x3c) | (text[i + 2] >> 6)];   //将第二个字符的后4位与第三个字符的前2位组合并找出对应的结果字符
			base64Str += __base64__alphabet_map[text[i + 2] & 0x3f];                         //取出第三个字符的后6位并找出结果字符
		}
		if (i < text_len)
		{
			unsigned int tail = text_len - i;
			if (tail == 1)
			{
				base64Str += __base64__alphabet_map[text[i] >> 2];
				base64Str += __base64__alphabet_map[(text[i] << 4) & 0x30];
				base64Str += '=';
				base64Str += '=';
			}
			else //tail==2
			{
				base64Str += __base64__alphabet_map[text[i] >> 2];
				base64Str += __base64__alphabet_map[((text[i] << 4) & 0x30) | (text[i + 1] >> 4)];
				base64Str += __base64__alphabet_map[(text[i + 1] << 2) & 0x3c];
				base64Str += '=';
			}
		}
		return base64Str;
	}
	std::string  base64_decode(const unsigned char* code, unsigned int code_len)
	{
		//assert();  //如果它的条件返回错误，则终止程序执行。4的倍数。
		if ((code_len & 0x03) == 0) {
			return "";
		}
		std::string str;

		unsigned int i, j = 0;
		unsigned char quad[4];
		for (i = 0; i < code_len; i += 4)
		{
			for (unsigned int k = 0; k < 4; k++)
			{
				quad[k] = __base64__reverse_map[code[i + k]];//分组，每组四个分别依次转换为base64表内的十进制数
			}

			//assert(quad[0] < 64 && quad[1] < 64);
			if (quad[0] < 64 && quad[1] < 64) {
				return "";
			}

			str += (quad[0] << 2) | (quad[1] >> 4); //取出第一个字符对应base64表的十进制数的前6位与第二个字符对应base64表的十进制数的前2位进行组合

			if (quad[2] >= 64)
				break;
			else if (quad[3] >= 64)
			{
				str += (quad[1] << 4) | (quad[2] >> 2); //取出第二个字符对应base64表的十进制数的后4位与第三个字符对应base64表的十进制数的前4位进行组合
				break;
			}
			else
			{
				str += (quad[1] << 4) | (quad[2] >> 2);
				str += (quad[2] << 6) | quad[3];//取出第三个字符对应base64表的十进制数的后2位与第4个字符进行组合
			}
		}
		return str;
	}


	namespace detail {

		inline bool is_hex(char c, int& v) {
			if (0x20 <= c && isdigit(c)) {
				v = c - '0';
				return true;
			}
			else if ('A' <= c && c <= 'F') {
				v = c - 'A' + 10;
				return true;
			}
			else if ('a' <= c && c <= 'f') {
				v = c - 'a' + 10;
				return true;
			}
			return false;
		}

		inline bool from_hex_to_i(const std::string& s, size_t i, size_t cnt,
			int& val) {
			if (i >= s.size()) { return false; }

			val = 0;
			for (; cnt; i++, cnt--) {
				if (!s[i]) { return false; }
				int v = 0;
				if (is_hex(s[i], v)) {
					val = val * 16 + v;
				}
				else {
					return false;
				}
			}
			return true;
		}

		inline std::string from_i_to_hex(size_t n) {
			const char* charset = "0123456789abcdef";
			std::string ret;
			do {
				ret = charset[n & 15] + ret;
				n >>= 4;
			} while (n > 0);
			return ret;
		}

		inline size_t to_utf8(int code, char* buff) {
			if (code < 0x0080) {
				buff[0] = (code & 0x7F);
				return 1;
			}
			else if (code < 0x0800) {
				buff[0] = static_cast<char>(0xC0 | ((code >> 6) & 0x1F));
				buff[1] = static_cast<char>(0x80 | (code & 0x3F));
				return 2;
			}
			else if (code < 0xD800) {
				buff[0] = static_cast<char>(0xE0 | ((code >> 12) & 0xF));
				buff[1] = static_cast<char>(0x80 | ((code >> 6) & 0x3F));
				buff[2] = static_cast<char>(0x80 | (code & 0x3F));
				return 3;
			}
			else if (code < 0xE000) { // D800 - DFFF is invalid...
				return 0;
			}
			else if (code < 0x10000) {
				buff[0] = static_cast<char>(0xE0 | ((code >> 12) & 0xF));
				buff[1] = static_cast<char>(0x80 | ((code >> 6) & 0x3F));
				buff[2] = static_cast<char>(0x80 | (code & 0x3F));
				return 3;
			}
			else if (code < 0x110000) {
				buff[0] = static_cast<char>(0xF0 | ((code >> 18) & 0x7));
				buff[1] = static_cast<char>(0x80 | ((code >> 12) & 0x3F));
				buff[2] = static_cast<char>(0x80 | ((code >> 6) & 0x3F));
				buff[3] = static_cast<char>(0x80 | (code & 0x3F));
				return 4;
			}

			// NOTREACHED
			return 0;
		}

		// NOTE: This code came up with the following stackoverflow post:
		// https://stackoverflow.com/questions/180947/base64-decode-snippet-in-c
		inline std::string base64_encode(const std::string& in) {
			static const auto lookup =
				"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

			std::string out;
			out.reserve(in.size());

			int val = 0;
			int valb = -6;

			for (auto c : in) {
				val = (val << 8) + static_cast<uint8_t>(c);
				valb += 8;
				while (valb >= 0) {
					out.push_back(lookup[(val >> valb) & 0x3F]);
					valb -= 6;
				}
			}

			if (valb > -6) { out.push_back(lookup[((val << 8) >> (valb + 8)) & 0x3F]); }

			while (out.size() % 4) {
				out.push_back('=');
			}

			return out;
		}

		std::string encode_url(const std::string& s) {
			std::string result;
			result.reserve(s.size());

			for (size_t i = 0; s[i]; i++) {
				switch (s[i]) {
				case ' ': result += "%20"; break;
				case '+': result += "%2B"; break;
				case '\r': result += "%0D"; break;
				case '\n': result += "%0A"; break;
				case '\'': result += "%27"; break;
				case ',': result += "%2C"; break;
					// case ':': result += "%3A"; break; // ok? probably...
				case ';': result += "%3B"; break;
				default:
					auto c = static_cast<uint8_t>(s[i]);
					if (c >= 0x80) {
						result += '%';
						char hex[4];
						auto len = snprintf(hex, sizeof(hex) - 1, "%02X", c);
						assert(len == 2);
						result.append(hex, static_cast<size_t>(len));
					}
					else {
						result += s[i];
					}
					break;
				}
			}

			return result;
		}

		std::string decode_url(const std::string& s,
			bool convert_plus_to_space) {
			std::string result;

			for (size_t i = 0; i < s.size(); i++) {
				if (s[i] == '%' && i + 1 < s.size()) {
					if (s[i + 1] == 'u') {
						int val = 0;
						if (from_hex_to_i(s, i + 2, 4, val)) {
							// 4 digits Unicode codes
							char buff[4];
							size_t len = to_utf8(val, buff);
							if (len > 0) { result.append(buff, len); }
							i += 5; // 'u0000'
						}
						else {
							result += s[i];
						}
					}
					else {
						int val = 0;
						if (from_hex_to_i(s, i + 1, 2, val)) {
							// 2 digits hex codes
							result += static_cast<char>(val);
							i += 2; // '00'
						}
						else {
							result += s[i];
						}
					}
				}
				else if (convert_plus_to_space && s[i] == '+') {
					result += ' ';
				}
				else {
					result += s[i];
				}
			}

			return result;
		}
	};

	std::string UrlEncode(const std::string& str) {
		return detail::encode_url(str);
	}
	std::string UrlDecode(const std::string& str, bool convert_plus_to_space) {
		return detail::decode_url(str, convert_plus_to_space);
	}

};
