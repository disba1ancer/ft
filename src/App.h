/*
 * App.h
 *
 *  Created on: 19 сент. 2020 г.
 *      Author: disba1ancer
 */

#ifndef APP_H_
#define APP_H_

#include "win32/win32.h"
#include <swal/handle.h>
#include <swal/window.h>
#include <swal/com.h>
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
	void SetupRender();
	void MakeRenderTarget();
	void DetectAndCreateDevice();
	void DrawFTAndFunc();
public:
	static float func(int sec, float fractsec);
private:
	int argc;
	char **argv;
	Microsoft::WRL::ComPtr<IAudioClient> audioClient;
	//Microsoft::WRL::ComPtr<IAudioRenderClient> renderClient;
	Microsoft::WRL::ComPtr<IAudioCaptureClient> captureClient;
	UINT32 bufSize;
	swal::Event event;
	std::unique_ptr<WAVEFORMATEXTENSIBLE, swal::com_deleter> wfex;
	std::vector<float> func_v;
	std::vector<float> func_ftv;
	bool maxXOff;
	//std::vector<float> func_fftv;
	//std::vector<float> func_rv;
	std::vector<float> mic_samples;
	swal::Window wnd;
	Microsoft::WRL::ComPtr<ID2D1Factory1> d2d1Factory;
	Microsoft::WRL::ComPtr<ID2D1RenderTarget> renderTarget;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush;
	unsigned time, ftime;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtView;
	Microsoft::WRL::ComPtr<ID3D11Buffer> ftvBuf;
//	Microsoft::WRL::ComPtr<ID3DBlob> compiledVertShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertShader;
//	Microsoft::WRL::ComPtr<ID3DBlob> compiledFragShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> fragShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inLayout;
};

} // namespace ft

#endif /* APP_H_ */
