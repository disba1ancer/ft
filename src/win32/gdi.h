/*
 * gdi.h
 *
 *  Created on: 3 сент. 2020 г.
 *      Author: disba1ancer
 */

#ifndef WIN32_GDI_H_
#define WIN32_GDI_H_

#include <windows.h>
#include "error.h"
#include "move_only.h"

namespace win32 {

class GdiObj : public move_only<HGDIOBJ> {
public:
	GdiObj(HGDIOBJ obj) : move_only(obj) {}
	~GdiObj() {
		DeleteObject(*this);
	}
};

enum class PenStyle {
	Solid = PS_SOLID,
	Dash = PS_DASH,
	Dot = PS_DOT,
	DashDot = PS_DASHDOT,
	DashDotDot = PS_DASHDOTDOT,
	Null = PS_NULL,
	InsideFrame = PS_INSIDEFRAME
};

COLORREF Rgb(int r, int g, int b) { return RGB(r, g, b); }

class Pen : public GdiObj {
public:
	Pen(COLORREF color, PenStyle style = PenStyle::Solid, int width = 1) : GdiObj(error::throw_or_result(CreatePen(static_cast<int>(style), width, color))) {}
};

class DC {
public:
	DC(HDC hdc) : hdc(hdc) {}
	operator HDC() { return hdc; }
	HGDIOBJ SelectObject(HGDIOBJ obj) { return error::throw_or_result(::SelectObject(hdc, obj)); }
	void MoveTo(int x, int y) { error::throw_or_result(::MoveToEx(hdc, x, y, nullptr)); }
	POINT MoveToEx(int x, int y) {
		POINT pt;
		error::throw_or_result(::MoveToEx(hdc, x, y, &pt));
		return pt;
	}
	void LineTo(int x, int y) { error::throw_or_result(::LineTo(hdc, x, y)); }
	COLORREF SetPenColor(COLORREF color) { return error::throw_or_result(::SetDCPenColor(*this, color), invalid_color_error_check); }
	COLORREF SetPixel(int x, int y, COLORREF color) { return error::throw_or_result(::SetPixel(*this, x, y, color), invalid_color_error_check); }
private:
	HDC hdc;
};

class PaintDC : private PAINTSTRUCT, public DC {
public:
	~PaintDC() { EndPaint(hWnd, this); }
	const PAINTSTRUCT* operator ->() { return this; }
	static PaintDC BeginPaint(HWND hWnd) { return PaintDC(hWnd); }
private:
	PaintDC(HWND hWnd) : DC(error::throw_or_result(::BeginPaint(hWnd, this))), hWnd(hWnd) {}
	HWND hWnd;
};

}

#endif /* WIN32_GDI_H_ */
