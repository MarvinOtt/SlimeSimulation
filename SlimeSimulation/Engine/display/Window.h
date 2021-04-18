#pragma once
#include <Windows.h>

class Window
{
public:
	HWND hwnd;
	HINSTANCE hInstance;
	bool IsCreated;

	int windowWidth;
	int windowHeight;

private:
	HRESULT hr;

public:
	Window(HINSTANCE hInstance);
	bool Create(bool fullscreen, LPCWSTR name, int sizex, int sizey, WNDPROC wndproc, int offsetx = 0, int offsety = 0);
	bool SetFullscreenState(bool state);
	bool Show(int state);
	bool Resize(UINT width, UINT height, bool AdjustRect = false);
};

