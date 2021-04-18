#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
class GraphicsDevice;

template <typename T>
class ConstantBuffer
{
public:
    ID3D12Resource** constantBufferUploadHeaps;
    int ConstantBufferPerObjectAlignedSize;
    T cb;
    UINT8** cbvGPUAddress;

private:
    HRESULT hr = 0;

public:
    ConstantBuffer();
    bool Create(GraphicsDevice*);

};

#include "../GraphicsDevice.h"

template <typename T>
ConstantBuffer<T>::ConstantBuffer()
{

}

template <typename T>
bool ConstantBuffer<T>::Create(GraphicsDevice* device)
{
    constantBufferUploadHeaps = new ID3D12Resource*[device->frameBufferCount];
    cbvGPUAddress = new UINT8*[device->frameBufferCount];
    ConstantBufferPerObjectAlignedSize = (sizeof(T) + 255) & ~255;

    for (int i = 0; i < device->frameBufferCount; ++i)
    {
        // create resource for cube 1
        hr = device->device->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
            D3D12_HEAP_FLAG_NONE, // no flags
            &CD3DX12_RESOURCE_DESC::Buffer(D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
            D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
            nullptr, // we do not have use an optimized clear value for constant buffers
            IID_PPV_ARGS(&constantBufferUploadHeaps[i]));
        if (FAILED(hr))
            return false;

        constantBufferUploadHeaps[i]->SetName(L"Constant Buffer Upload Resource Heap");

        ZeroMemory(&cb, sizeof(cb));

        CD3DX12_RANGE readRange(0, 0);    // We do not intend to read from this resource on the CPU. (so end is less than or equal to begin)

        // map the resource heap to get a gpu virtual address to the beginning of the heap
        hr = constantBufferUploadHeaps[i]->Map(0, &readRange, reinterpret_cast<void**>(&cbvGPUAddress[i]));

        // Because of the constant read alignment requirements, constant buffer views must be 256 bit aligned. Our buffers are smaller than 256 bits,
        // so we need to add spacing between the two buffers, so that the second buffer starts at 256 bits from the beginning of the resource heap.
        memcpy(cbvGPUAddress[i], &cb, sizeof(cb)); // cube1's constant buffer data
        memcpy(cbvGPUAddress[i] + ConstantBufferPerObjectAlignedSize, &cb, sizeof(cb)); // cube2's constant buffer data
    }


    return true;
}




