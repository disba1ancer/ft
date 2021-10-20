/*
 * strconv.h
 *
 *  Created on: 29 авг. 2020 г.
 *      Author: disba1ancer
 */

#ifndef WIN32_STRCONV_H_
#define WIN32_STRCONV_H_

#include <windows.h>
#include <string>

namespace win32 {

inline std::u8string wide_char_to_multibyte(std::wstring_view str) {
	std::u8string result;
	result.resize(WideCharToMultiByte(CP_UTF8, 0, str.data(), str.size(), nullptr, 0, nullptr, nullptr));
	WideCharToMultiByte(CP_UTF8, 0, str.data(), str.size(), reinterpret_cast<char*>(result.data()), result.size(), nullptr, nullptr);
	return result;
}

inline std::wstring multibyte_to_wide_char(std::u8string_view str) {
	std::wstring result;
	result.resize(MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(str.data()), str.size(), nullptr, 0));
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(str.data()), str.size(), result.data(), result.size());
	return result;
}

}

#endif /* WIN32_STRCONV_H_ */
