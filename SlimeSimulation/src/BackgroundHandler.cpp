#include "BackgroundHandler.h"
#include <display/Window.h>
#include <display/Monitor.h>
#include <GraphicsDevice.h>

bool BackgroundHandler::IsBackground = false;
HWND BackgroundHandler::desktopHWND = NULL;

void BackgroundHandler::SetBG(bool state, GraphicsDevice* device, Window* window, int width, int height, int posX, int posY)
{
	IsBackground = state;
	desktopHWND = NULL;

	int tryNum = 0;
tryNum:
	EnumWindows(enumWindowCallback, NULL);
	if (desktopHWND != NULL)
	{
		if (!state)
		{
			SetParent(window->hwnd, state ? desktopHWND : NULL);
			window->SetFullscreenState(state);
		}
		else
			window->SetFullscreenState(state);
		SetWindowPos(window->hwnd, NULL, posX, posY, width, height, SWP_SHOWWINDOW);
		window->Resize(width, height);
		device->ResizeBackBuffer(width, height);
		if (state)
			SetParent(window->hwnd, state ? desktopHWND : NULL);
		else
			window->SetFullscreenState(state);
	}
	else
	{
		HWND prog = FindWindow(L"Progman", NULL);
		SendMessageTimeout(prog, 0x052C, NULL, NULL, SMTO_NORMAL, 1000, nullptr);
		tryNum++;
		Sleep(500);
		if (tryNum < 5)
			goto tryNum;
		else
			IsBackground = false;
	}
}