/*
 * win32_error.h
 *
 *  Created on: 28 авг. 2020 г.
 *      Author: disba1ancer
 */

#ifndef WIN32_ERROR_H_
#define WIN32_ERROR_H_

#include <windows.h>
#include <wchar.h>
#include <comdef.h>
#include <exception>
#include <string>
#include <memory>
#include "strconv.h"

namespace win32 {

class error : public std::exception {
public:

	template <typename T>
	static T throw_or_result(T result) {
		if (!result) throw error(GetLastError());
		return result;
	}

	template <typename T, typename F>
	static T throw_or_result(T result, const F& chk) {
		DWORD err = chk(result);
		if (err != ERROR_SUCCESS) throw error(err);
		return result;
	}

	static void throw_last_error() {
		auto err = GetLastError();
		if (err != ERROR_SUCCESS) throw error(err);
	}

	static std::u8string get_error_string(DWORD error) {
		constexpr std::size_t resultStringMaxSize = 512;
		wchar_t wStr[resultStringMaxSize];
		auto wSize = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK, nullptr, error, 0, wStr, resultStringMaxSize, nullptr);
		return wide_char_to_multibyte(std::wstring_view(wStr, wSize));
	}

	error(DWORD error) noexcept : m_error(error), errStr(std::make_shared<std::u8string>(get_error_string(error))) {}

	virtual const char* what() const noexcept override {
		return reinterpret_cast<const char*>(errStr->c_str());
	}

	DWORD get() {
		return m_error;
	}
private:
	DWORD m_error;
	std::shared_ptr<std::u8string> errStr;
};

class com_error : public std::exception {
public:

	static HRESULT throw_or_result(HRESULT result) {
		if (FAILED(result)) throw com_error(result);
		return result;
	}

	/*template <typename F>
	static HRESULT throw_or_result(HRESULT result, const F& chk) {
		if (!chk(result)) throw com_error(result);
		return result;
	}*/

	static std::u8string get_error_string(DWORD error) {
		constexpr std::size_t resultStringMaxSize = 512;
		wchar_t wStr[resultStringMaxSize];
		auto wSize = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK, nullptr, error, 0, wStr, resultStringMaxSize, nullptr);
		return wide_char_to_multibyte(std::wstring_view(wStr, wSize));
	}

	com_error(HRESULT error) noexcept : error(error), errStr(std::make_shared<std::u8string>(wide_char_to_multibyte(_com_error(error).ErrorMessage()))) {}

	virtual const char* what() const noexcept override {
		return reinterpret_cast<const char*>(errStr->c_str());
	}

	HRESULT get() {
		return error;
	}
private:
	HRESULT error;
	std::shared_ptr<std::u8string> errStr;
};

inline DWORD GetMessage_error_check(BOOL result) {
	return ((result != -1) ? 0 : GetLastError());
}

inline DWORD wait_func_error_check(DWORD result) {
	return ((result != WAIT_FAILED) ? 0 : GetLastError());
}

inline DWORD GetWindowLongPtr_error_check(LONG_PTR result) {
	return (result ? 0 : GetLastError());
}

inline DWORD invalid_color_error_check(COLORREF result) {
	return (result != CLR_INVALID ? 0 : GetLastError());
}

}

#endif /* WIN32_ERROR_H_ */
