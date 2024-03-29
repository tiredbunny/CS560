#include "pch.h"
#include "Timer.h"
#include "DemoScene.h"
#include "imgui.h"
#include "StepTimer.h"
#include <windowsx.h>

LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool InitializeWindow(HINSTANCE hInstance, HWND& outWindowHandle);

constexpr auto g_ClientWidth = 1280;
constexpr auto g_ClientHeight = 720;

DWORD g_WindowStyle = WS_OVERLAPPEDWINDOW ^ (WS_THICKFRAME | WS_MAXIMIZEBOX);

std::unique_ptr<DemoScene> g_Scene;
std::unique_ptr<Path> g_Path;

DX::StepTimer g_Timer;

void Update(DX::StepTimer const& timer)
{
	float deltaTime = float(timer.GetElapsedSeconds());
	float totalTime = float(timer.GetTotalSeconds());

	g_Scene->UpdateScene(timer);
}

void Tick()
{
	g_Timer.Tick([&]()
		{
			Update(g_Timer);
		});
	g_Scene->DrawScene();
}

INT WINAPI wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nShowCmd)
{

#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"CoInitializeEx failed", 0, MB_OK | MB_ICONERROR);
		return -1;
	}

	HWND windowHandle;
	if (!InitializeWindow(hInstance, windowHandle))
	{
		MessageBox(NULL, L"Failed to create a window", 0, MB_OK | MB_ICONERROR);
		return -1;
	}
	
	if (!DirectX::XMVerifyCPUSupport())
	{
		MessageBox(NULL, L"DirectXMath not supported", 0, MB_OK | MB_ICONERROR);
		return -1;
	}

	

	g_Scene = std::make_unique<DemoScene>(windowHandle);
	g_Path = std::make_unique<Path>(g_Timer.GetTotalSeconds());

	if (!g_Scene->Initialize())
	{
		MessageBox(NULL, L"Failed to initialize scene", 0, MB_OK | MB_ICONERROR);
		return -1;
	}

	ShowWindow(windowHandle, nShowCmd);
	UpdateWindow(windowHandle);

	MSG msg = {};

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Tick();
			
		}
	}

	CoUninitialize();

	return static_cast<int>(msg.wParam);
}

bool InitializeWindow(HINSTANCE hInstance, HWND& windowHandle)
{
	WNDCLASS wc = {};

	const std::wstring windowClass = L"WindowClass1";

	wc.hInstance = hInstance;
	wc.lpszClassName = windowClass.c_str();
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW); 
	wc.lpfnWndProc = WindowProc;

	if (!RegisterClass(&wc))
		return false;

	RECT wr = { 0, 0, g_ClientWidth, g_ClientHeight };
	AdjustWindowRect(&wr, g_WindowStyle, FALSE);

	windowHandle = CreateWindow(windowClass.c_str(), L"Demo",
		g_WindowStyle,
		CW_USEDEFAULT, CW_USEDEFAULT,
		wr.right - wr.left, wr.bottom - wr.top,
		0, 0, hInstance, 0);

	if (!windowHandle)
		return false;
	
	
	return true;
}


// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		g_Scene->OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		g_Scene->OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		g_Scene->OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}