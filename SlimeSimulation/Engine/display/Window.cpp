#include "Window.h"

Window::Window(HINSTANCE hInstance2)
{
	hInstance = hInstance2;
	IsCreated = false;
}

LRESULT CALLBACK msgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		DestroyWindow(hwnd);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}

bool Window::Create(bool fullscreen, LPCWSTR name, int sizex, int sizey, WNDPROC wndproc, int offsetx, int offsety)
{
	//if (fullscreen)
	//{
	//	HMONITOR hmon = MonitorFromWindow(hwnd,
	//		MONITOR_DEFAULTTONEAREST);
	//	MONITORINFO mi = { sizeof(mi) };
	//	GetMonitorInfo(hmon, &mi);

	//}

	WNDCLASS wc = {};

	//wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = wndproc;
	//wc.cbClsExtra = NULL;
	//wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	//wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	//wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	//wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	//wc.lpszMenuName = NULL;
	wc.lpszClassName = name;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

	// Window size we have is for client area, calculate actual window size
	DWORD winStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	RECT r{ 0, 0, (LONG)sizex, (LONG)sizey };
	AdjustWindowRect(&r, winStyle, false);

	windowWidth = r.right - r.left;
	windowHeight = r.bottom - r.top;
	if (fullscreen)
	{
		windowWidth = sizex;
		windowHeight = sizey;
	}

	if (RegisterClass(&wc) == 0)
	{
		MessageBox(NULL, L"Error registering class",
			L"Error", MB_OK | MB_ICONERROR);
		return false;
	}
	hwnd = CreateWindowEx(NULL,
		name,
		name,
		winStyle,
		offsetx, offsety,
		windowWidth, windowHeight,
		nullptr,
		nullptr,
		hInstance,
		nullptr);
	if (hwnd == nullptr)
	{
		MessageBox(NULL, L"Error creating window",
			L"Error", MB_OK | MB_ICONERROR);
		return false;
	}

	if (fullscreen)
	{
		SetFullscreenState(true);
		//SetWindowLong(hwnd, GWL_STYLE, 0);
	}
	else
		SetFullscreenState(false);
	IsCreated = true;
	return true;
}

bool Window::SetFullscreenState(bool state)
{
	DWORD winStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE;
	if(state)
		SetWindowLong(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
	else
		SetWindowLong(hwnd, GWL_STYLE, winStyle);
	return true;
}

bool Window::Resize(UINT width, UINT height, bool AdjustRect)
{
	DWORD winStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
	RECT r{ 0, 0, (LONG)width, (LONG)height };
	AdjustWindowRect(&r, winStyle, false);
	windowWidth = r.right - r.left;
	windowHeight = r.bottom - r.top;
	return true;
}

bool Window::Show(int state)
{
	if (!IsCreated)
		return false;
	ShowWindow(hwnd, state);
	UpdateWindow(hwnd);
	SetActiveWindow(hwnd);
	return true;
}