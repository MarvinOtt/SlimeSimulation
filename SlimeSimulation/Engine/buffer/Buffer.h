#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>

class GraphicsDevice;

class Buffer
{
public:
	ID3D12Resource* buffer = nullptr;
	UINT64 bufferSize;
	DXGI_FORMAT format;

private:
	HRESULT hr;

public:
	Buffer(GraphicsDevice* device, UINT64 buffersize, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState, D3D12_HEAP_TYPE heap_type, DXGI_FORMAT _format = DXGI_FORMAT_UNKNOWN);
	~Buffer();
	void SetData(void* data);

};


