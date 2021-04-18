#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
class GraphicsDevice;
class DescriptorHeap;

#define NewTexture3D(Variable) (Variable = new Texture3D(L#Variable))
#define NewTexture3D_Create(Variable, device, format, Width, Height, Depth) (Variable = new Texture3D(L#Variable, device, format, Width, Height, Depth))

class Texture3D
{
public:
    ID3D12Resource* buffer;
    D3D12_RESOURCE_DESC desc;
    UINT64 bufferSize;
	LPCWSTR name;

private:
    HRESULT hr;
    ID3D12Resource* uploadheap;

public:
    Texture3D(LPCWSTR name);
	Texture3D(LPCWSTR name, GraphicsDevice* device, DXGI_FORMAT format, int Width, int Height, int Depth);
    bool Create(GraphicsDevice*, D3D12_RESOURCE_DESC);
    bool Create(GraphicsDevice* device, DXGI_FORMAT format, int Width, int Height, int Depth);
    bool Create(GraphicsDevice* device, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flag, int Width, int Height, int Depth);

    void CreateView(GraphicsDevice* device, DescriptorHeap* descHeap, int heapindex);

    bool SetData(GraphicsDevice* device, BYTE* texdata, int imagesize, DescriptorHeap* descHeap = nullptr, int heapindex = 0);
};

