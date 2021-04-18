#pragma once
#include <Windows.h>
#include <shellscalingapi.h>
#include <iostream>

struct MonitorData
{
public:
	int ID;
	int sizeX, sizeY;
	int posX, posY;
	float scaling;

	MonitorData();
	MonitorData(int t_ID, int t_sizeX, int t_sizeY, int t_posX, int t_posY, float t_scaling);
};

class Monitor
{
public:
	static int monNumber;
	static int maxMonWidth, maxMonHeight;
	static int monRightEdge, monLeftEdge, monBottomEdge, monTopEdge;
	static int monSizeX, monSizeY;
	static bool IsMonitorHovered;
	static MonitorData* monitors;

private:
	static int curScreen;

public:

	static int GetMonitorData();
	static int GetMonPosX(int ID);
	static int GetMonPosY(int ID);
	static void ImGui_WndPreview(float width, int &selectedMonitor);

private:
	static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
	{

		MONITORINFOEX info;
		info.cbSize = sizeof(info);
		if (GetMonitorInfo(hMonitor, &info))
		{
			DEVICE_SCALE_FACTOR fac = {};
			GetScaleFactorForMonitor(hMonitor, &fac);


			DEVMODE devMode;
			devMode.dmSize = sizeof(devMode);
			devMode.dmDriverExtra = 0;
			EnumDisplaySettings(info.szDevice, ENUM_CURRENT_SETTINGS, &devMode);
			int sizeX = devMode.dmPelsWidth;
			int sizeY = devMode.dmPelsHeight;
			if (sizeX > maxMonWidth)
				maxMonWidth = sizeX;
			if (sizeY > maxMonHeight)
				maxMonHeight = sizeY;
			if (devMode.dmPosition.x + sizeX > monRightEdge)
				monRightEdge = devMode.dmPosition.x + sizeX;
			if (devMode.dmPosition.y + sizeY > monBottomEdge)
				monBottomEdge = devMode.dmPosition.y + sizeY;

			if (devMode.dmPosition.x < monLeftEdge)
				monLeftEdge = devMode.dmPosition.x;
			if (devMode.dmPosition.y < monTopEdge)
				monTopEdge = devMode.dmPosition.y;
			monitors[curScreen] = MonitorData(curScreen, sizeX, sizeY, devMode.dmPosition.x, devMode.dmPosition.y, fac * 0.01f);
			curScreen++;
		}
		return TRUE;
	}
};

