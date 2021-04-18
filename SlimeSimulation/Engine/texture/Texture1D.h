#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
class GraphicsDevice;
class DescriptorHeap;

#define NewTexture1D(Variable) (Variable = new Texture2D(L#Variable))
#define NewTexture1D_Create(Variable, device, format, Width) (Variable = new Texture1D(L#Variable, device, format, Width))
#define NewTexture1D_CreateS(Variable, device, format, Width, state) (Variable = new Texture1D(L#Variable, device, format, Width, state))

class Texture1D
{
public:
    ID3D12Resource* buffer;
    D3D12_RESOURCE_DESC desc;
    UINT64 bufferSize;
	LPCWSTR name;
    ID3D12Resource* uploadheap = nullptr;

private:
    HRESULT hr;

public:
    Texture1D(LPCWSTR name);
	Texture1D(LPCWSTR name, GraphicsDevice* device, DXGI_FORMAT format, int Width, D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COPY_DEST);
    bool Create(GraphicsDevice*, D3D12_RESOURCE_DESC, D3D12_RESOURCE_STATES state);
    bool Create(GraphicsDevice* device, DXGI_FORMAT format, int Width, D3D12_RESOURCE_STATES state);
    bool Create(GraphicsDevice* device, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flag, int Width, D3D12_RESOURCE_STATES state);

    void CreateView(GraphicsDevice* device, DescriptorHeap* descHeap, int heapindex);

    bool SetData(GraphicsDevice* device, BYTE* texdata, int imagesize);
};

