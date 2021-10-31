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
#include <cstring>
#include <cstddef>
#include "App.h"
#include <swal/error.h>
//#include <swal/gdi.h>
#include "ft.h"
#include "extmath.h"
#include "int24_t.h"
#include "shaders/gl31.h"

namespace ft {

HINSTANCE App::hInstance = GetModuleHandle(nullptr);

namespace {
struct VertexIn {
	float in0[2];
	float in1[2];
};
constexpr int maxX = 1 << 10;
constexpr DWORD ThisIdx = 0;
const TCHAR ftWndClassName[] = TEXT("ftwnd");
}

App::App(int argc, char *argv[]) :
	argc(argc),
	argv(argv),
	event(false, false),
	func_v(maxX),
	func_ftv(maxX * 2),
	maxXOff(false),
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

	SetupRender();

	wnd.Show(swal::ShowCmd::ShowDefault);
}

int App::run()
{
	constexpr auto mask = maxX * 2 - 1;
	MSG msg;

	UINT32 bufPad;
	float* data;
	DWORD flags;

	FFT fft(maxX);

	fft(func_v.data(), func_ftv.data());

	while (true) {
		switch (swal::winapi_call(MsgWaitForMultipleObjects(1, event.get_ptr(), FALSE, INFINITE, QS_ALLINPUT), swal::wait_func_error_check)) {
		case WAIT_OBJECT_0:
			while (true) try {
				swal::com_call(captureClient->GetNextPacketSize(&bufPad));
				while (bufPad) {
					{
						BYTE* t;
						swal::com_call(captureClient->GetBuffer(&t, &bufPad, &flags, nullptr, nullptr));
						data = reinterpret_cast<decltype(data)>(t);
					}
					for (unsigned i = 0; i < bufPad; ++i) {
						mic_samples[ftime] = float(data[i * wfex->Format.nChannels]);
						ftime = (ftime + 1) & mask;
						if ((ftime & mask) == 0) {
							time = (ftime - maxX) & mask;
							maxXOff = !maxXOff;
							fft(mic_samples.data(), func_ftv.data() + (maxX * maxXOff), time, mask, false);
							wnd.InvalidateRect();
						}
					}
					swal::com_call(captureClient->ReleaseBuffer(/*bufSize -*/ bufPad));
					swal::com_call(captureClient->GetNextPacketSize(&bufPad));
				}
				break;
			} catch (std::system_error& error) {
				if (error.code() == std::error_code(swal::com_errc(AUDCLNT_E_DEVICE_INVALIDATED))) {
					initAudio();
				}
				else throw;
			}
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
	swal::com_call(audioClient->Stop());

	return msg.wParam;
}

ATOM App::MyRegisterClass()
{
	auto regCls = []{
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(wcex);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = swal::ClsWndProc<App, &App::WndProc, ThisIdx>;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(App*);
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = ftWndClassName;
		wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
		return swal::winapi_call(RegisterClassEx(&wcex));
	};
	static ATOM cls = regCls();
	return cls;
}

void App::initAudio()
{
	while (true) try {
		Microsoft::WRL::ComPtr<IMMDeviceEnumerator> devEnum;
		Microsoft::WRL::ComPtr<IMMDevice> device;
		swal::com_call(CoCreateInstance(CLSID_MMDeviceEnumerator, nullptr, CLSCTX_ALL, IID_PPV_ARGS(devEnum.ReleaseAndGetAddressOf())));
		swal::com_call(devEnum->GetDefaultAudioEndpoint(EDataFlow::eRender, ERole::eConsole, device.ReleaseAndGetAddressOf()));
		swal::com_call(device->Activate(IID_IAudioClient, CLSCTX_ALL, nullptr, reinterpret_cast<void**>(audioClient.ReleaseAndGetAddressOf())));

		{
			WAVEFORMATEX* t;
			swal::com_call(audioClient->GetMixFormat(&t));
			wfex.reset(reinterpret_cast<WAVEFORMATEXTENSIBLE*>(t));
		}
		{
			REFERENCE_TIME def, min;
			swal::com_call(audioClient->GetDevicePeriod(&def, &min));
			swal::com_call(audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_LOOPBACK, def, 0, &(wfex->Format), nullptr));
		}
		swal::com_call(audioClient->GetBufferSize(&bufSize));
		swal::com_call(audioClient->GetService(IID_PPV_ARGS(captureClient.ReleaseAndGetAddressOf())));
		swal::com_call(audioClient->SetEventHandle(event));

		swal::com_call(audioClient->Start());
		break;
	} catch (std::system_error& error) {
		if (error.code() == std::error_code(swal::com_errc(AUDCLNT_E_DEVICE_INVALIDATED))) continue;
		else throw;
	}
}

LRESULT App::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	using Microsoft::WRL::ComPtr;
	switch (message) {
		case WM_ERASEBKGND: return true;
		case WM_PAINT: {
			//auto a = std::chrono::high_resolution_clock::now();{
			DrawFTAndFunc();
			wnd.ValidateRect();
			//}auto b = std::chrono::high_resolution_clock::now();
			//std::cout << (b - a).count() << std::endl;
			break;
		}
		case WM_SIZE:
			if (swapChain) {
//				swal::com_call(renderTarget->Flush());
				renderTarget.Reset();
				rtView.Reset();
				context->ClearState();
				swal::com_call(swapChain->ResizeBuffers(0, LOWORD(lParam), HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0));
				MakeRenderTarget();
				D3D11_VIEWPORT viewport = {
					.Width = float(LOWORD(lParam)),
					.Height = float(HIWORD(lParam)),
					.MinDepth = 0,
					.MaxDepth = 1
				};
				context->RSSetViewports(1, &viewport);
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void App::SetupRender()
{
	DetectAndCreateDevice();

	swal::com_call(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(&d2d1Factory)));
	MakeRenderTarget();
	auto bufDesc = CD3D11_BUFFER_DESC(
		maxX * sizeof(VertexIn),
		D3D11_BIND_VERTEX_BUFFER,
		D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_WRITE
	);
//	D3D11_SUBRESOURCE_DATA subResDesc = { .pSysMem = func_ftv.data() };
	swal::com_call(device->CreateBuffer(&bufDesc, nullptr, &ftvBuf));

	Microsoft::WRL::ComPtr<ID3DBlob> shaderCode;
	Microsoft::WRL::ComPtr<ID3DBlob> log;

	if (FAILED(D3DCompile(
		generated::shaders::shader_frag_hlsl,
		std::strlen(generated::shaders::shader_frag_hlsl),
		nullptr, nullptr, nullptr,
		"fragmentMain",
		"ps_5_0",
		0, 0,
		&shaderCode,
		&log
	))) {
		throw std::runtime_error(reinterpret_cast<char*>(log->GetBufferPointer()));
	}
	swal::com_call(device->CreatePixelShader(
		shaderCode->GetBufferPointer(),
		shaderCode->GetBufferSize(),
		nullptr,
		&fragShader
	));

	if (FAILED(D3DCompile(
		generated::shaders::shader_vert_hlsl,
		std::strlen(generated::shaders::shader_vert_hlsl),
		nullptr, nullptr, nullptr,
		"vertexMain",
		"vs_5_0",
		0, 0,
		&shaderCode,
		&log
	))) {
		throw std::runtime_error(reinterpret_cast<char*>(log->GetBufferPointer()));
	}
	swal::com_call(device->CreateVertexShader(
		shaderCode->GetBufferPointer(),
		shaderCode->GetBufferSize(),
		nullptr,
		&vertShader
	));

	D3D11_INPUT_ELEMENT_DESC layout[] = {{
		.SemanticName = "POSITION",
		.SemanticIndex = 0,
		.Format = DXGI_FORMAT_R32G32_FLOAT,
		.InputSlot = 0,
		.AlignedByteOffset = 0,
		.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
		.InstanceDataStepRate = 0
	}, {
		.SemanticName = "POSITION",
		.SemanticIndex = 1,
		.Format = DXGI_FORMAT_R32G32_FLOAT,
		.InputSlot = 1,
		.AlignedByteOffset = 0,
		.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
		.InstanceDataStepRate = 0
	}};
	swal::com_call(device->CreateInputLayout(
		layout,
		std::size(layout),
		shaderCode->GetBufferPointer(),
		shaderCode->GetBufferSize(),
		&inLayout
	));
}

void App::MakeRenderTarget()
{
	using Microsoft::WRL::ComPtr;
	ComPtr<IDXGISurface> surface;
	ComPtr<ID3D11Texture2D> renderTarget2;
	swal::com_call(swapChain->GetBuffer(0, IID_PPV_ARGS(&surface)));
	swal::com_call(swapChain->GetBuffer(0, IID_PPV_ARGS(&renderTarget2)));

	swal::com_call(d2d1Factory->CreateDxgiSurfaceRenderTarget(
		surface.Get(),
		D2D1::RenderTargetProperties(
		  D2D1_RENDER_TARGET_TYPE_DEFAULT,
		  D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED)
		),
		&renderTarget
	));

	renderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
	swal::com_call(renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &brush));
	device->CreateRenderTargetView(renderTarget2.Get(), nullptr, &rtView);
}

void App::DetectAndCreateDevice()
{
	Microsoft::WRL::ComPtr<IDXGIFactory1> dxgiFactory;
	Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgiAdapter;

	CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

	static const D3D_FEATURE_LEVEL FeatureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

	auto rc = wnd.GetRect();

	DXGI_SWAP_CHAIN_DESC sd;
	memset(&sd, 0, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = rc.right - rc.left;
	sd.BufferDesc.Height = rc.bottom - rc.top;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = wnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	for (
		unsigned i = 0;
		(dxgiFactory->EnumAdapters1(i, &dxgiAdapter)) != DXGI_ERROR_NOT_FOUND;
		++i
	) {
		D3D_FEATURE_LEVEL FeatureLevel;

		if (SUCCEEDED(D3D11CreateDeviceAndSwapChain(
			dxgiAdapter.Get(),
			D3D_DRIVER_TYPE_UNKNOWN,
			nullptr,
			D3D11_CREATE_DEVICE_BGRA_SUPPORT,
			FeatureLevels,
			std::size(FeatureLevels),
			D3D11_SDK_VERSION,
			&sd,
			&swapChain,
			&device,
			&FeatureLevel,
			&context
		))) {
			DXGI_ADAPTER_DESC1 desc;
			std::fill(std::begin(desc.Description), std::end(desc.Description), 0);
			dxgiAdapter->GetDesc1(&desc);
			DWORD written;
			auto stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
			WriteConsole(stdoutHandle, desc.Description, wcslen(desc.Description), &written, nullptr);
			WriteConsole(stdoutHandle, L"\n", 1, &written, nullptr);
			return;
		}
	}
	throw std::runtime_error("No adapter with d3d11 support");
}

void App::DrawFTAndFunc()
{
	int width, height;
	{
		RECT rc = wnd.GetClientRect();
		width = rc.right;
		height = rc.bottom - 1;
	}
	D2D_POINT_2F point[2];
	renderTarget->BeginDraw();
//	brush->SetColor(D2D1::ColorF(0.f, 0.f, 0.f, .5f));
//	renderTarget->FillRectangle(D2D1::RectF(0, 0, width, height + 1), brush.Get());
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
//	brush->SetColor(D2D1::ColorF(0.f, 0.f, 1.f, 1.f));
//	point[0] = D2D1::Point2F(-1 + .5f, height / 2);
//	for (int i = 0; i < std::min(maxX / 2, width); ++i) {
//		auto t = func_ftv[i + maxX / 2];
//		t = (std::isnan(t) || std::isinf(t) ? 0.f : t);
//		point[(i + 1) & 1] = D2D1::Point2F(i + .5f, (height - (t * height / Pi)) / 2);
//		renderTarget->DrawLine(point[(i) & 1], point[(i + 1) & 1], brush.Get(), 1.f, nullptr);
//	}
//	brush->SetColor(D2D1::ColorF(0.f, 1.f, 1.f, 1.f));
//	point[0] = D2D1::Point2F(-1 + .5f, height / 2);
//	for (int i = 0; i < std::min(maxX / 2, width); ++i) {
//		auto t1 = func_ftv[i * 2 + 1 + (maxX * maxXOff)];
//		t1 = (std::isnan(t1) || std::isinf(t1) ? 0.f : t1);
//		auto t2 = func_ftv[i * 2 + 1 + (maxX * (!maxXOff))];
//		t2 = (std::isnan(t2) || std::isinf(t2) ? 0.f : t2);
//		auto t = fract((t1 - t2) * .5f / Pi + 1.f);
//		point[(i + 1) & 1] = D2D1::Point2F(i + .5f, height - (t * height) + .5f);
//		renderTarget->DrawLine(point[(i) & 1], point[(i + 1) & 1], brush.Get(), 1.f, nullptr);
//	}
//	brush->SetColor(D2D1::ColorF(0.f, 1.f, 0.f, 1.f));
//	point[0] = D2D1::Point2F(-1 + .5f, height);
//	for (int i = 0; i < std::min(maxX / 2, width); ++i) {
//		auto t = std::clamp(std::log10(func_ftv[i]), -10.f, 0.f);
//		point[(i + 1) & 1] = D2D1::Point2F(i + .5f, -(t * .1f * height) /*+ height*/ + .5f);
//		renderTarget->DrawLine(point[(i) & 1], point[(i + 1) & 1], brush.Get(), 1.f, nullptr);
//	}
//	brush->SetColor(D2D1::ColorF(1.f, 1.f, 0.f, 1.f));
//	point[0] = D2D1::Point2F(-1 + .5f, height * .5f);
//	for (int i = 0; i < std::min(maxX / 2, width); ++i) {
//		auto t = std::clamp(std::log10(func_ftv[i * 2 + (maxX * maxXOff)]), -5.f, 0.f);
//		point[(i + 1) & 1] = D2D1::Point2F(i + .5f, -(t * .2f * height) /*+ height * .5f*/ + .5f);
//		renderTarget->DrawLine(point[(i) & 1], point[(i + 1) & 1], brush.Get(), 1.f, nullptr);
//	}
	swal::com_call(renderTarget->EndDraw());

	D3D11_MAPPED_SUBRESOURCE subres;
	swal::com_call(context->Map(ftvBuf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subres));
	auto buf = static_cast<VertexIn *>(subres.pData);
	for (std::size_t i = 0; i < maxX; i += 2) {
		std::size_t off1 = (i + maxX * maxXOff);
		std::size_t off2 = (i + maxX * !maxXOff);
		buf[i].in0[0] = func_ftv[off1];
		buf[i].in0[1] = func_ftv[off1 + 1];
		buf[i].in1[0] = func_ftv[off2];
		buf[i].in1[1] = func_ftv[off2 + 1];
		buf[i + 1].in0[0] = 0.f;
		buf[i + 1].in0[1] = 0.f;
		buf[i + 1].in1[0] = 0.f;
		buf[i + 1].in1[1] = 0.f;
	}
	context->Unmap(ftvBuf.Get(), 0);
	UINT stride[2] = { sizeof(VertexIn), sizeof(VertexIn) };
	UINT offset[2] = { 0, offsetof(VertexIn, in1) };
	ID3D11Buffer *buffers[2] = { ftvBuf.Get(), ftvBuf.Get() };
	float color[4] = { .015625f, .015625f, .015625f, 1.f };
//	context->ClearRenderTargetView(rtView.Get(), color);
	context->OMSetRenderTargets(1, rtView.GetAddressOf(), nullptr);
	context->IASetVertexBuffers(0, std::size(buffers), buffers, stride, offset);
	context->IASetInputLayout(inLayout.Get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	context->VSSetShader(vertShader.Get(), nullptr, 0);
	context->PSSetShader(fragShader.Get(), nullptr, 0);
	context->Draw(maxX, 0);

	swal::com_call(swapChain->Present(1, 0));
}

float App::func(int sec, float fractsec)
{
	constexpr auto freqf = .0f;
	constexpr auto freq = 440.f + freqf;//fractsec
	if constexpr (freqf != 0.f) {
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
