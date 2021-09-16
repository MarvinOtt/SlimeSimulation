#pragma once

#include <dinput.h>
#include <algorithm>
#include "../D3D12_Engine.h"
#include <windows.h>
#include <wincodec.h>
#include <initguid.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include "../include/d3dx12.h"
#include <string>
#include "../include/Framework.h"
#include <math.h>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <filesystem>
#include <iostream>
#include <random>

#include "Setting.h"
#include "BackgroundHandler.h"

#include "../include/ImGui/imgui.h"
#include "../include/ImGui/imgui_impl_win32.h"
#include "../include/ImGui/imgui_impl_dx12.h"
#include "../include/ImGui/imgui_internal.h"

#ifdef _DEBUG 
#include <csignal>
#define BOOST_STACKTRACE_USE_WINDBG
#include <boost/stacktrace.hpp>	
#endif

using namespace DirectX;
using namespace std::chrono;

D3D12_INPUT_ELEMENT_DESC inputLayout[] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
};

// Input Stuff
char* keyboardstate;
char* oldkeyboardstate;

// width and height of the window
int Width;
int Height;


const int particleMul = 128;

bool IsInitDone = false;
bool IsAutoRotate = true;
bool ShowAdvanced;

float progress = 0;
float resFac = 1.0f;

MSG msg;

// Monitors
int selectedMonitor = 0;
bool IsMonitorHovered = false;

Window* mainWindow;

GraphicsDevice* graphicsDevice = nullptr;
DescriptorHeap* mainHeap;

BYTE* particleBufferCPU;
ConstantBuffer<particleCB_DEF>* particleCB;
ConstantBuffer<diffuseDecayCB_DEF>* diffuseDecayCB;
DXR_RootSignature* particleRS;
DXR_RootSignature* diffuseDecayRS;

VertexBuffer<DirectX::XMFLOAT4>* fullscreenBuffer;

Texture3D* particletex;
Texture2D* trailtex1;
Texture2D* trailtex2;

Shader* diffuseDecayVS;
Shader* shaderDD_1TrailMaps;
Shader* shaderDD_2TrailMaps;

PipelineState* pipelineStateDD1;
PipelineState* pipelineStateDD2;

Setting* settings;

steady_clock::time_point lastSettingSwitch;

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void ChangeTrailTextures(int particleCount);
bool LoadSettings();
bool InitInput();
bool InitD3D();
bool Update();
void Render();

void PrepareSettingsWnd();
void PrepareAdvancedWnd();
