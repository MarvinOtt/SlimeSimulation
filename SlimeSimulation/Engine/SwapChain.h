#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
class GraphicsDevice;
class CommandQueue;

class SwapChain
{
public:
	IDXGISwapChain3* swapChain;
    DXGI_SAMPLE_DESC sampleDesc;

private:
	HRESULT hr;

public:
	SwapChain();
	bool Create(GraphicsDevice*, CommandQueue*);
};

