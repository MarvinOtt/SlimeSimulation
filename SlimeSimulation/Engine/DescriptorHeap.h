#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>

class GraphicsDevice;
class Texture3D;
class Texture2D;
class Texture1D;
class Buffer;

class DescriptorHeap
{
public:
	ID3D12DescriptorHeap* descriptorHeap;
	D3D12_DESCRIPTOR_HEAP_TYPE descriptorType;
    D3D12_DESCRIPTOR_HEAP_FLAGS descriptorFlags;
	int descriptorSize;
	int descriptorNum;

private:
	HRESULT hr;
	GraphicsDevice* device;

public:
	DescriptorHeap();
	DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE, D3D12_DESCRIPTOR_HEAP_FLAGS);
	bool Create(GraphicsDevice*, int);
	bool SetTexture1D(Texture1D* tex, int index);
    bool SetTexture2D(Texture2D* tex, int index);
	bool SetTexture3D(Texture3D* tex, int index);
	bool SetBuffer(Buffer* buf, int index);
    bool SetSRV(D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc, int index);
};
