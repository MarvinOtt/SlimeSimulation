#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>

class GraphicsDevice;
class DescriptorHeap;

class RenderTarget
{
public:
	DescriptorHeap* descriptorHeap;
	ID3D12Resource** renderTargets;
    ID3D12Resource* depthStencilBuffer = nullptr;
    int Width, Height;

private:
	HRESULT hr;

public:
    RenderTarget();
	RenderTarget(DescriptorHeap* descHeap);
	bool Create(GraphicsDevice*, int, int, DescriptorHeap*);
};


