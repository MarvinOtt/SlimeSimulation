#include "Monitor.h"
#include <Windows.h>
#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>
#include <string>

int Monitor::monNumber = 0;
int Monitor::maxMonWidth = 0;
int Monitor::maxMonHeight = 0;
int Monitor::monRightEdge = 0;
int Monitor::monLeftEdge = 0;
int Monitor::monBottomEdge = 0;
int Monitor::monTopEdge = 0;
int Monitor::monSizeX = 0;
int Monitor::monSizeY = 0;
int Monitor::curScreen = 0;
bool Monitor::IsMonitorHovered = false;
MonitorData* Monitor::monitors = nullptr;

MonitorData::MonitorData()
{
	MonitorData(0, 0, 0, 0, 0, 1.0f);
}

MonitorData::MonitorData(int t_ID, int t_sizeX, int t_sizeY, int t_posX, int t_posY, float t_scaling)
{
	ID = t_ID;
	sizeX = t_sizeX;
	sizeY = t_sizeY;
	posX = t_posX;
	posY = t_posY;
	scaling = t_scaling;
}

int Monitor::GetMonitorData()
{
	monNumber = GetSystemMetrics(SM_CMONITORS);
	monitors = new MonitorData[monNumber];
	curScreen = maxMonWidth = maxMonHeight = 0;
	monRightEdge = monBottomEdge = -9999;
	monLeftEdge = monTopEdge = 9999;

	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);
	monSizeX = monRightEdge - monLeftEdge;
	monSizeY = monBottomEdge - monTopEdge;
	return monNumber;
}

int Monitor::GetMonPosX(int ID)
{
	return monitors[ID].posX - monLeftEdge;
}

int Monitor::GetMonPosY(int ID)
{
	return monitors[ID].posY - monTopEdge;
}

void Monitor::ImGui_WndPreview(float width, int& selectedMonitor)
{
	float scale = width / monSizeX;
	int margin = 4;

	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	const ImVec2 p = ImGui::GetCursorScreenPos();
	IsMonitorHovered = false;
	for (int i = 0; i < monNumber; ++i)
	{
		float posX = p.x + (monitors[i].posX - monLeftEdge) * scale + margin + 2;
		float posY = p.y + (monitors[i].posY - monTopEdge) * scale + margin + 2;
		float sizeX = monitors[i].sizeX * scale - 4;
		float sizeY = monitors[i].sizeY * scale - 4;

		ImGui::ItemAdd(ImRect(posX, posY, posX + sizeX, posY + sizeY), 1000 + i);
		if (ImGui::IsItemClicked())
			selectedMonitor = i;
		if (ImGui::IsItemHovered())
			IsMonitorHovered = true;

		ImColor col(0.0f, 0.5f, 1.0f);
		if (i == selectedMonitor)
			col = ImColor(1.0f, 1.0f, 1.0f);
		draw_list->AddRect(ImVec2(posX, posY), ImVec2(posX + sizeX, posY + sizeY), col, 0.0f, ImDrawCornerFlags_All, 3);
		std::string IDstringS = std::to_string(i);
		const char* IDstring = IDstringS.c_str();
		ImVec2 texSize = ImGui::CalcTextSize(IDstring);
		draw_list->AddText(ImVec2(posX + sizeX / 2.0f - texSize.x / 2.0f, posY + sizeY / 2.0f - texSize.y / 2.0f), ImColor(255, 255, 255, 255), std::to_string(i).c_str());
	}
	ImGui::Dummy(ImVec2(350.0f, Monitor::monSizeY * scale));
}

