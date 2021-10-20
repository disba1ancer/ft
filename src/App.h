/*
 * App.h
 *
 *  Created on: 19 сент. 2020 г.
 *      Author: disba1ancer
 */

#ifndef APP_H_
#define APP_H_

#include "win32/win32.h"
#include "win32/handle.h"
#include "win32/window.h"
#include "win32/com.h"
#include <vector>

namespace ft {

class App {
public:
	static HINSTANCE hInstance;
public:
	App(int argc, char *argv[]);
	App(const App&) = delete;
	App& operator=(const App&) = delete;
	int run();
private:
	static ATOM MyRegisterClass();
	void initAudio();
	LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
public:
	static float func(int sec, float fractsec);
private:
	int argc;
	char **argv;
	Microsoft::WRL::ComPtr<IAudioClient> audioClient;
	//Microsoft::WRL::ComPtr<IAudioRenderClient> renderClient;
	Microsoft::WRL::ComPtr<IAudioCaptureClient> captureClient;
	UINT32 bufSize;
	win32::Event event;
	std::unique_ptr<WAVEFORMATEXTENSIBLE, win32::com_deleter> wfex;
	std::vector<float> func_v;
	std::vector<float> func_ftv;
	//std::vector<float> func_fftv;
	//std::vector<float> func_rv;
	std::vector<float> mic_samples;
	win32::Window wnd;
	Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> renderTarget;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush;
	int time, ftime;
};

} // namespace ft

#endif /* APP_H_ */
