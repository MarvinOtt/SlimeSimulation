#pragma once
#include <Windows.h>

class Window;
class GraphicsDevice;

class BackgroundHandler
{
public:
	static bool IsBackground;

private:
	static HWND desktopHWND;

public:

	static void SetBG(bool state, GraphicsDevice* device, Window* window, int width, int height, int posX = 0, int posY = 0);

	static BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lparam)
	{
		HWND p = FindWindowEx(hWnd, NULL, L"SHELLDLL_DefView", L"");
		if (p != NULL)
			desktopHWND = FindWindowEx(NULL, hWnd, L"WorkerW", NULL);

		return true;
	}
};

