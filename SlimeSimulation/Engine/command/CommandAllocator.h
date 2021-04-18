#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
class GraphicsDevice;

class CommandAllocator
{
public:
	ID3D12CommandAllocator* commandAllocator;

private:
	HRESULT hr;

public:
	CommandAllocator();
	bool Create(GraphicsDevice*, D3D12_COMMAND_LIST_TYPE);
};
