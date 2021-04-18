#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>

class GraphicsDevice;

class Fence
{
public:
	ID3D12Fence* fence;
	int fenceValue;
	static HANDLE fenceEvent;

private:
	HRESULT hr;

public:
	Fence();
	bool Create(GraphicsDevice*);
};


