#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>

class GraphicsDevice;

class CommandQueue
{
public:
	ID3D12CommandQueue* commandQueue;

private:
	HRESULT hr;

public:
	CommandQueue();
	bool Create(GraphicsDevice* device, D3D12_COMMAND_LIST_TYPE type);
};

