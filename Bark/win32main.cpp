#include <windows.h>
#include <windowsx.h>
#include <d3d11.h>
#include "main.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <crtdbg.h>

#define _CRTDBG_MAP_ALLOC
//#define FULLSCREEN_SMALL
#ifdef FULLSCREEN
#define SCREEN_X 1920
#define SCREEN_Y 1080
#elif defined FULLSCREEN_SMALL
#define SCREEN_X 1366
#define SCREEN_Y 768
#define FULLSCREEN
#else
#define SCREEN_X 800
#define SCREEN_Y 600
#endif


bool isRunning = false;

#define CHECK_WIN_ERROR(x,y) if(FAILED(x)) {OutputDebugStringA(y);}
#define RELEASE_DX_RESOURCE(x) x->Release(); x = NULL;

void DebugLiveObjects(ID3D11Device *device);
void SetDebugName(ID3D11DeviceChild* child, uint32 nameLength, char *name);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CLOSE:
		{
			isRunning = false;
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

static void ProcessMessages(HWND window, Input *input)
{
	MSG message;
	while (PeekMessageA(&message, window, 0, 0, PM_REMOVE))
	{
		switch (message.message)
		{
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			if (message.wParam == VK_ESCAPE)
			{ 
				isRunning = false;
			}
			if (message.wParam == 'R')
			{
			}
			switch (message.wParam)
			{
				case 'W':
					input->Wpressed = true;
					break;
				case 'A':
					break;
				case 'S':
					input->Spressed = true;
					break;
				case 'D':
					break;
				case 'B':
					input->Bpressed = true;
					break;
				case 'Q':
					break;
			}						
		} break;
		case WM_MOUSEMOVE:
		{
			input->touchPosition.x = (float)GET_X_LPARAM(message.lParam);
			input->touchPosition.y = (float)GET_Y_LPARAM(message.lParam);
		} break;
		case WM_LBUTTONDOWN:
		{
			input->leftButtonDown = true;
			input->leftButtonPressed = true;
		} break;
		case WM_RBUTTONDOWN:
		{
			input->rightButtonDown = true;
		} break;
		case WM_LBUTTONUP:
		{
			input->leftButtonDown = false;
		} break;
		case WM_MOUSEWHEEL:
		{
			input->mouseScroll = GET_WHEEL_DELTA_WPARAM(message.wParam);

		} break;
		default:
		{
			TranslateMessage(&message);
			DispatchMessageA(&message);
		} break;
		}
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASSEXA windowClass = {};
	windowClass.cbSize = sizeof(WNDCLASSEXA);
	windowClass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = WndProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(0, IDC_ARROW);
	windowClass.lpszClassName = "RTRT";

	DWORD winFlags = WS_VISIBLE;
#ifdef FULLSCREEN
	winFlags |= WS_POPUP;
#else
	winFlags |= WS_OVERLAPPED;
#endif

	if (RegisterClassExA(&windowClass))
	{
		HWND window = CreateWindowExA(0, windowClass.lpszClassName, "Real Time Ray Tracer", winFlags,
			CW_USEDEFAULT, CW_USEDEFAULT, SCREEN_X, SCREEN_Y,
			NULL, NULL, hInstance, NULL);
		if (window)
		{
			isRunning = true;

			LARGE_INTEGER clockFreq;
			QueryPerformanceFrequency(&clockFreq);
			LARGE_INTEGER startTime, endTime, elapsedTicks;
			QueryPerformanceCounter(&startTime);
			srand(startTime.LowPart);

			srand(startTime.LowPart);

			ID3D11Device *device;
			ID3D11DeviceContext *context;
			IDXGISwapChain *swapChain;

			UINT flags = 0;
#ifdef _DEBUG
			flags = D3D11_CREATE_DEVICE_DEBUG;
#endif
			DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
			swapChainDesc.BufferDesc.Width = (UINT)SCREEN_X;
			swapChainDesc.BufferDesc.Height = (UINT)SCREEN_Y;
			swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = 1;
			swapChainDesc.OutputWindow = window;
			swapChainDesc.Windowed = true;
			swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

			D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
			D3D_FEATURE_LEVEL supportedFeatureLevel;
			HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, &featureLevel, 1, D3D11_SDK_VERSION, 
													   &swapChainDesc, &swapChain, &device, &supportedFeatureLevel, &context);
			CHECK_WIN_ERROR(hr, "Error creating D3D11 device/context/swapChain \n");

			ID3D11Texture2D *backBuffer;
			hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void **)&backBuffer);
			CHECK_WIN_ERROR(hr, "Error getting backbuffer\n");

			ID3D11RenderTargetView *renderTargetView;
			hr = device->CreateRenderTargetView(backBuffer, NULL, &renderTargetView);
			CHECK_WIN_ERROR(hr, "Error creating Render Target\n");

			CD3D11_TEXTURE2D_DESC depthStencilDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, (UINT)SCREEN_X, (UINT)SCREEN_Y, 
										   1, 1, D3D11_BIND_DEPTH_STENCIL);
			ID3D11Texture2D *depthStencil;
			hr = device->CreateTexture2D(&depthStencilDesc, NULL, &depthStencil);
			CHECK_WIN_ERROR(hr, "Error creating depth stencil");

			ID3D11DepthStencilView *depthStencilView;
			CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
			hr = device->CreateDepthStencilView(depthStencil, &depthStencilViewDesc, &depthStencilView);
			CHECK_WIN_ERROR(hr, "Error creating depth stencil view\n");
			RELEASE_DX_RESOURCE(depthStencil);

			Context bContext;
			bContext.renderer.context = context;
			bContext.renderer.device = device;
			bContext.renderer.screenSize = Vector2(SCREEN_X, SCREEN_Y);
			bContext.screenTarget.renderTarget = renderTargetView;
			bContext.screenTarget.depthStencil = depthStencilView;
			bContext.screenTarget.resource = backBuffer;
			bContext.screenTarget.width = SCREEN_X;
			bContext.screenTarget.height = SCREEN_Y;

			D3D11_SAMPLER_DESC samplerDesc = {};
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			samplerDesc.MinLOD = -D3D11_FLOAT32_MAX;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
			hr = device->CreateSamplerState(&samplerDesc, &bContext.renderer.texSampler);

			D3D11_BLEND_DESC blendDesc = {};
			blendDesc.AlphaToCoverageEnable = FALSE;
			blendDesc.IndependentBlendEnable = FALSE;
			blendDesc.RenderTarget[0].BlendEnable = TRUE;
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			ID3D11BlendState *blendState;
			hr = device->CreateBlendState(&blendDesc, &blendState);
			CHECK_WIN_ERROR(hr, "Error creating blend state.\n");
			float factor[4] = { 1,1,1,1 };
			context->OMSetBlendState(blendState, factor, 0xffffffff);

			Init(&bContext);
			Input input = {};

			char buffer[100];
			srand(startTime.LowPart);
			while (isRunning)
			{
				QueryPerformanceCounter(&endTime);
				elapsedTicks.QuadPart = endTime.QuadPart - startTime.QuadPart;
				startTime.QuadPart = endTime.QuadPart;
				float dt = (float)elapsedTicks.QuadPart / clockFreq.QuadPart;
				sprintf_s(buffer, 100, "%f\n", dt);
				//OutputDebugStringA(buffer);
				ProcessMessages(window, &input);
				Update(&bContext, &input, dt);
				input.mouseScroll = 0;
				input.previousPosition = input.touchPosition;
				input.leftButtonPressed = false;
				input.rightButtonDown = false;
				input.Wpressed = false;
				input.Bpressed = false;
				input.Spressed = false;
				swapChain->Present(1, 0);
				QueryPerformanceCounter(&endTime);
			}
			
			Release(&bContext);
			RELEASE_DX_RESOURCE(renderTargetView);
			RELEASE_DX_RESOURCE(swapChain);
			RELEASE_DX_RESOURCE(context);
			RELEASE_DX_RESOURCE(depthStencilView);

			//DebugLiveObjects(device);
			RELEASE_DX_RESOURCE(device);
		}
	}
	_CrtDumpMemoryLeaks();
}

void DebugLiveObjects(ID3D11Device *device)
{
	ID3D11Debug *pDebug;
	HRESULT hr = device->QueryInterface(IID_PPV_ARGS(&pDebug));
	CHECK_WIN_ERROR(hr, "Error creating debug interface.\n");

	pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);

	RELEASE_DX_RESOURCE(pDebug);
}

void SetDebugName(ID3D11DeviceChild* child, uint32 nameLength, char *name)
{
	child->SetPrivateData(WKPDID_D3DDebugObjectName, nameLength, name);
}