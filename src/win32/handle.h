/*
 * handle.h
 *
 *  Created on: 6 сент. 2020 г.
 *      Author: disba1ancer
 */

#ifndef WIN32_HANDLE_H_
#define WIN32_HANDLE_H_

#include <windows.h>
#include <string>
#include "error.h"
#include "move_only.h"
#include "strconv.h"
#include "enum_bitwise.h"

namespace win32 {

class Handle : public move_only<HANDLE> {
public:
	Handle(HANDLE handle) : move_only(handle) {}
};

template<typename T>
class CloseableHandle {
public:
	~CloseableHandle() { CloseHandle(*static_cast<T*>(this)); }
};

template<typename T>
class WaitableHandle {
public:
	void WaitFor(DWORD milliseconds = INFINITE) const { win32::error::throw_or_result(WaitForSingleObject(*static_cast<T*>(this), milliseconds)); }
};

enum class EventFlags {
	InitialSet = CREATE_EVENT_INITIAL_SET,
	ManualReset = CREATE_EVENT_MANUAL_RESET
};

template <>
struct enable_enum_bitwise<EventFlags> {
	static constexpr bool value = true;
};

class Event : public Handle, public CloseableHandle<Event>, public WaitableHandle<Event> {
public:
	Event(SECURITY_ATTRIBUTES* sattrs, bool manualReset, bool initialState)
		: Handle(win32::error::throw_or_result(CreateEvent(sattrs, manualReset, initialState, nullptr))) {}
	Event(SECURITY_ATTRIBUTES* sattrs, bool manualReset, bool initialState, std::u8string_view name)
		: Handle(win32::error::throw_or_result(CreateEvent(sattrs, manualReset, initialState, multibyte_to_wide_char(name).data()))) {}
	Event(SECURITY_ATTRIBUTES* sattrs, EventFlags flags, DWORD access)
		: Handle(win32::error::throw_or_result(CreateEventEx(sattrs, nullptr, static_cast<DWORD>(flags), access))) {}
	Event(SECURITY_ATTRIBUTES* sattrs, std::u8string_view name, EventFlags flags, DWORD access)
		: Handle(win32::error::throw_or_result(CreateEventEx(sattrs, multibyte_to_wide_char(name).data(), static_cast<DWORD>(flags), access))) {}
	void Set() { error::throw_or_result(SetEvent(*this)); }
	void Reset() { error::throw_or_result(ResetEvent(*this)); }
};

}

#endif /* WIN32_HANDLE_H_ */
