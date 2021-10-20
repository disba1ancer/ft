/*
 * App.cpp
 *
 *  Created on: 19 сент. 2020 г.
 *      Author: disba1ancer
 */

#include "win32/win32.h"
#include <iostream>
#include <chrono>
#include <cmath>
#include "App.h"
#include "win32/error.h"
//#include "win32/gdi.h"
#include "ft.h"
#include "extmath.h"
#include "int24_t.h"

namespace ft {

HINSTANCE App::hInstance = GetModuleHandle(nullptr);

namespace {
constexpr int logMaxX = 11;
constexpr int maxX = 1 << logMaxX;
constexpr DWORD ThisIdx = 0;
const TCHAR ftWndClassName[] = TEXT("ftwnd");
}

App::App(int argc, char *argv[]) :
argc(argc),
argv(argv),
event(nullptr, false, false),
func_v(maxX),
func_ftv(maxX),
//func_fftv(maxX),
//func_rv(maxX),
mic_samples(maxX * 2),
wnd(MyRegisterClass(), hInstance, this),
time(maxX),
ftime(0)
{
	initAudio();

	for (int i = 0; i < maxX; ++i) {
		func_v[i] = func(0, i / float(40000));
	}

	ft(func_v.data(), func_ftv.data(), maxX);

	/*for (int i = 0; i < maxX; ++i) {
		func_rv[i] = 0;
		for (int j = 0; j < maxX / 2; ++j) {
			func_rv[i] += std::cos(fract(j * i / float(maxX)) * 2 * Pi + func_ftv[maxX / 2 + j]) * func_ftv[j] * 2.f;
		}
	}*/

	{
		Microsoft::WRL::ComPtr<ID2D1Factory1> d2d1Factory;
		win32::com_error::throw_or_result(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(&d2d1Factory)));
		RECT rc = wnd.GetClientRect();
		win32::com_error::throw_or_result(
			d2d1Factory->CreateHwndRenderTarget(
				D2D1::RenderTargetProperties(
					D2D1_RENDER_TARGET_TYPE_DEFAULT,
					D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
				),
				D2D1::HwndRenderTargetProperties(wnd, D2D1::SizeU(rc.right, rc.bottom)),
				&renderTarget
			)
		);
	}
	win32::com_error::throw_or_result(renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &brush));
	renderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);

	wnd.Show(win32::ShowCmd::ShowDefault);
}

int App::run() {
	MSG msg;

	UINT32 bufPad;
	float* data;
	DWORD flags;

	FFT fft(logMaxX);

	while (true) {
		switch (win32::error::throw_or_result(MsgWaitForMultipleObjects(1, event.get_ptr(), FALSE, INFINITE, QS_ALLINPUT), win32::wait_func_error_check)) {
		case WAIT_OBJECT_0:
			while (true) try {
				win32::com_error::throw_or_result(captureClient->GetNextPacketSize(&bufPad));
				while (bufPad) {
					{
						BYTE* t;
						//win32::com_error::throw_or_result(audioClient->GetCurrentPadding(&bufPad));
						win32::com_error::throw_or_result(captureClient->GetBuffer(&t, /*bufSize -*/ &bufPad, &flags, nullptr, nullptr));
						data = reinterpret_cast<decltype(data)>(t);
					}
					for (int i = 0; i < /*(bufSize / 4 * 3) -*/ bufPad; ++i) {
						mic_samples[(ftime + i) & (maxX * 2 - 1)] = float(data[i * wfex->Format.nChannels]) /*/ INT16_MAX*/;
						/*float a = func(time, fract((ftime) / float(wfex->Format.nSamplesPerSec)));
						std::int32_t b = a * ((1U << (fmt.Format.wBitsPerSample - 1)) - 1);
						for (int j = 0; j < wfex->Format.nChannels; ++j) {
							data[i * wfex->Format.nChannels + j] = b;
							//data[i * wfex->Format.nChannels + 1] = 0;
						}
						++ftime;
						if (ftime >= wfex->Format.nSamplesPerSec) {
							ftime -= wfex->Format.nSamplesPerSec;
							++time;
						}*/
					}
					ftime = (ftime + bufPad) & (maxX * 2 - 1);
					time = (time + bufPad) & (maxX * 2 - 1);
					win32::com_error::throw_or_result(captureClient->ReleaseBuffer(/*bufSize -*/ bufPad));
					win32::com_error::throw_or_result(captureClient->GetNextPacketSize(&bufPad));
				}
				break;
			} catch (win32::com_error& error) {
				if (error.get() == AUDCLNT_E_DEVICE_INVALIDATED) {
					initAudio();
				}
				else throw;
			}
			//ft(mic_samples.data() + maxX * fttime, func_ftv.data(), maxX/*, .0625f*/);
			fft(mic_samples.data(), func_ftv.data(), time, maxX * 2 - 1);
			wnd.InvalidateRect();
			break;
			[[fallthrough]];
		case WAIT_OBJECT_0 + 1:
		default:
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				if (msg.message == WM_QUIT) {
					goto exit;
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			break;
		}
	} exit:
	win32::com_error::throw_or_result(audioClient->Stop());

	return msg.wParam;
}

ATOM App::MyRegisterClass() {
	auto regCls = []{
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(wcex);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = win32::ClsWndProc<App, &App::WndProc, ThisIdx>;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(App*);
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = ftWndClassName;
		wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
		return win32::error::throw_or_result(RegisterClassEx(&wcex));
	};
	static ATOM cls = regCls();
	return cls;
}

void App::initAudio() {
	while (true) try {
		Microsoft::WRL::ComPtr<IMMDeviceEnumerator> devEnum;
		Microsoft::WRL::ComPtr<IMMDevice> device;
		win32::com_error::throw_or_result(CoCreateInstance(CLSID_MMDeviceEnumerator, nullptr, CLSCTX_ALL, IID_PPV_ARGS(devEnum.ReleaseAndGetAddressOf())));
		win32::com_error::throw_or_result(devEnum->GetDefaultAudioEndpoint(EDataFlow::eRender, ERole::eConsole, device.ReleaseAndGetAddressOf()));
		win32::com_error::throw_or_result(device->Activate(IID_IAudioClient, CLSCTX_ALL, nullptr, reinterpret_cast<void**>(audioClient.ReleaseAndGetAddressOf())));

		/*WAVEFORMATEXTENSIBLE fmt, *wfex = &fmt;
		{
			fmt.Format.wFormatTag = WAVE_FORMAT_PCM;
			fmt.Format.nChannels = 2;
			fmt.Format.nSamplesPerSec = 48000;
			fmt.Format.wBitsPerSample = 16;
			fmt.Format.nBlockAlign = fmt.Format.nChannels * fmt.Format.wBitsPerSample / 8;
			fmt.Format.nAvgBytesPerSec = fmt.Format.nSamplesPerSec * fmt.Format.nBlockAlign;
			fmt.Format.cbSize = 0;
			fmt.Samples.wValidBitsPerSample = 0;
			//fmt.Samples.wSamplesPerBlock = 2;
			//fmt.Samples.wReserved = 0;
			fmt.dwChannelMask = 3;
			fmt.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
			auto r = audioClient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, &fmt.Format, nullptr);
			std::cout << r << std::endl;
		}*/

		{
			WAVEFORMATEX* t;
			win32::com_error::throw_or_result(audioClient->GetMixFormat(&t));
			wfex.reset(reinterpret_cast<WAVEFORMATEXTENSIBLE*>(t));
		}
		{
			REFERENCE_TIME def, min;
			win32::com_error::throw_or_result(audioClient->GetDevicePeriod(&def, &min));
			win32::com_error::throw_or_result(audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_LOOPBACK, def, 0, &(wfex->Format), nullptr));
		}
		win32::com_error::throw_or_result(audioClient->GetBufferSize(&bufSize));
		win32::com_error::throw_or_result(audioClient->GetService(IID_PPV_ARGS(captureClient.ReleaseAndGetAddressOf())));
		win32::com_error::throw_or_result(audioClient->SetEventHandle(event));

		win32::com_error::throw_or_result(audioClient->Start());
		break;
	} catch (win32::com_error& error) {
		if (error.get() == AUDCLNT_E_DEVICE_INVALIDATED) continue;
		else throw;
	}
}

LRESULT App::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_ERASEBKGND: return true;
	case WM_PAINT: {
		//auto a = std::chrono::high_resolution_clock::now();{
		int width, height;
		{
			RECT rc = wnd.GetClientRect();
			width = rc.right;
			height = rc.bottom - 1;
		}
		D2D_POINT_2F point[2];
		renderTarget->BeginDraw();
		//brush->SetColor(D2D1::ColorF(0.f, 0.f, 0.f, 1.f));
		//renderTarget->FillRectangle(D2D1::RectF(0, 1, width, height + 1), brush.Get());
		renderTarget->Clear(D2D1::ColorF(0, 0, 0, 1.f));
		brush->SetColor(D2D1::ColorF(D2D1::ColorF::LightGray));
		point[0] = D2D1::Point2F(-1 + .5f, height / 2);
		for (int i = 0; i < std::min(maxX, width); ++i) {
			auto t = mic_samples[(time + i) & (maxX * 2 - 1)];
			t = (std::isnan(t) || std::isinf(t) ? 0.f : t);
			point[(i + 1) & 1] = D2D1::Point2F(i + .5f, (height - (t * height)) / 2);
			renderTarget->DrawLine(point[(i) & 1], point[(i + 1) & 1], brush.Get(), 1.f, nullptr);
		}
		brush->SetColor(D2D1::ColorF(D2D1::ColorF::Red));
		point[0] = D2D1::Point2F(-1 + .5f, height / 2);
		/*for (int i = 0; i < std::min(maxX, width); ++i) {
			auto t = func_rv[i];
			t = (std::isnan(t) || std::isinf(t) ? 0.f : t);
			point[(i + 1) & 1] = D2D1::Point2F(i + .5f, (height - (t * height)) / 2);
			renderTarget->DrawLine(point[(i) & 1], point[(i + 1) & 1], brush.Get(), 1.f, nullptr);
		}*/
//		brush->SetColor(D2D1::ColorF(0.f, 0.f, 1.f, 1.f));
//		point[0] = D2D1::Point2F(-1 + .5f, height / 2);
//		for (int i = 0; i < std::min(maxX / 2, width); ++i) {
//			auto t = func_ftv[i + maxX / 2];
//			t = (std::isnan(t) || std::isinf(t) ? 0.f : t);
//			point[(i + 1) & 1] = D2D1::Point2F(i + .5f, (height - (t * height / Pi)) / 2);
//			renderTarget->DrawLine(point[(i) & 1], point[(i + 1) & 1], brush.Get(), 1.f, nullptr);
//		}
//		brush->SetColor(D2D1::ColorF(0.f, 1.f, 1.f, 1.f));
//		point[0] = D2D1::Point2F(-1 + .5f, height / 2);
//		for (int i = 0; i < std::min(maxX / 2, width); ++i) {
//			auto t = func_ftv[i + maxX / 2];
//			t = (std::isnan(t) || std::isinf(t) ? 0.f : t);
//			point[(i + 1) & 1] = D2D1::Point2F(i + .5f, (height - (t * height / Pi)) / 2);
//			renderTarget->DrawLine(point[(i) & 1], point[(i + 1) & 1], brush.Get(), 1.f, nullptr);
//		}
//		brush->SetColor(D2D1::ColorF(0.f, 1.f, 0.f, 1.f));
//		point[0] = D2D1::Point2F(-1 + .5f, height);
//		for (int i = 0; i < std::min(maxX / 2, width); ++i) {
//			auto t = std::clamp(std::log10(func_ftv[i]), -10.f, 0.f);
//			point[(i + 1) & 1] = D2D1::Point2F(i + .5f, -(t * .1f * height) /*+ height*/ + .5f);
//			renderTarget->DrawLine(point[(i) & 1], point[(i + 1) & 1], brush.Get(), 1.f, nullptr);
//		}
		brush->SetColor(D2D1::ColorF(1.f, 1.f, 0.f, 1.f));
		point[0] = D2D1::Point2F(-1 + .5f, height);
		for (int i = 0; i < std::min(maxX / 2, width); ++i) {
			auto t = std::clamp(std::log10(func_ftv[i]), -5.f, 0.f);
			point[(i + 1) & 1] = D2D1::Point2F(i + .5f, -(t * .2f * height) /*+ height*/ + .5f);
			renderTarget->DrawLine(point[(i) & 1], point[(i + 1) & 1], brush.Get(), 1.f, nullptr);
		}
		win32::com_error::throw_or_result(renderTarget->EndDraw());
		wnd.ValidateRect();
		//}auto b = std::chrono::high_resolution_clock::now();
		//std::cout << (b - a).count() << std::endl;
		break;
	}
	case WM_SIZE:
		win32::com_error::throw_or_result(renderTarget->Resize(D2D1::SizeU(LOWORD(lParam), HIWORD(lParam))));
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

float App::func(int sec, float fractsec) {
	constexpr auto freq = 440.f;
	constexpr auto freqf = fract(freq);//fractsec
	if constexpr (freqf) {
		fractsec += (sec % int(std::floor(1.f / freqf)));
	}
	return std::cos(fractsec * freq * 2.f * Pi) /*+ std::sin(x / 1024.f * 20.f * 2.f * Pi)*/;
	//return std::sin(x / 1024.f * 128.f * 2.f * Pi) * std::sin(x / 1024.f * 8.f * 2.f * Pi);
	/*float a = 0;
	for (int i = 1; i < 10; i += 2) {
		a += std::sin(fract((x % 1024) / 1024.f * i * 16.f) * 2.f * Pi) / i;
	}
	return a;*/
	return (.5f - std::clamp(std::floor(fract(fractsec * 50.f) * 2.f), 0.f, 1.f)) * 2.f;
}

} // namespace ft
