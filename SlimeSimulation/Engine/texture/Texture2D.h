#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
class GraphicsDevice;
class DescriptorHeap;

#define NewTexture2D(Variable) (Variable = new Texture2D(L#Variable))
#define NewTexture2D_Create(Variable, device, format, Width, Height) (Variable = new Texture2D(L#Variable, device, format, Width, Height))

class Texture2D
{
public:
	ID3D12Resource* buffer = nullptr;
	D3D12_RESOURCE_DESC desc;
	UINT64 bufferSize;
	LPCWSTR name;

private:
	HRESULT hr;
	ID3D12Resource* uploadheap = nullptr;

public:
	Texture2D(LPCWSTR name);
	Texture2D(LPCWSTR name, GraphicsDevice* device, DXGI_FORMAT format, int Width, int Height);
	~Texture2D();
	bool Create(GraphicsDevice*, D3D12_RESOURCE_DESC);
	bool Create(GraphicsDevice* device, DXGI_FORMAT format, int Width, int Height);
	bool Create(GraphicsDevice* device, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flag, int Width, int Height);

	void CreateView(GraphicsDevice* device, DescriptorHeap* descHeap, int heapindex);

	bool SetData(GraphicsDevice* device, BYTE* texdata, int imagesize, DescriptorHeap* descHeap, int heapindex);
};

