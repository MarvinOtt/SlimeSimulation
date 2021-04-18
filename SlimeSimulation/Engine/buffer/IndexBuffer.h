#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include "../../include/d3dx12.h"

class GraphicsDevice;

template<typename T>
class IndexBuffer
{
public:
    ID3D12Resource* buffer;
    D3D12_RESOURCE_DESC desc;
    D3D12_INDEX_BUFFER_VIEW bufferView;
    UINT bufferSize;
    int numIndices;

private:
    HRESULT hr;
    ID3D12Resource* uploadheap;

public:
    IndexBuffer();
    bool Create(GraphicsDevice*, UINT);

    bool SetData(GraphicsDevice* device, T* vertices, int numindices);

};

#include "../GraphicsDevice.h"
#include "../command/CommandList.h"

template<typename T>
IndexBuffer<T>::IndexBuffer()
{

}
template<typename T>
bool IndexBuffer<T>::Create(GraphicsDevice* device, UINT buffersize)
{
    bufferSize = buffersize;
    // create default heap
    // default heap is memory on the GPU. Only the GPU has access to this memory
    // To get data into this heap, we will have to upload the data using
    // an upload heap
    desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
    hr = device->device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &desc, // resource description for a buffer
        D3D12_RESOURCE_STATE_COPY_DEST, // we will start this heap in the copy destination state since we will copy data
                                        // from the upload heap to this heap
        nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
        IID_PPV_ARGS(&buffer));
    if (FAILED(hr))
        return false;

    return true;
}

template <typename T>
bool IndexBuffer<T>::SetData(GraphicsDevice* device, T* vertices, int numindices)
{
    numIndices = numindices;
    // create upload heap
    // upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
    // We will upload the vertex buffer using this heap to the default heap
    hr = device->device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &CD3DX12_RESOURCE_DESC::Buffer(bufferSize), // resource description for a buffer
        D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
        nullptr,
        IID_PPV_ARGS(&uploadheap));
    if (FAILED(hr))
        return false;


    // store vertex buffer in upload heap
    D3D12_SUBRESOURCE_DATA vertexData = {};
    vertexData.pData = reinterpret_cast<BYTE*>(vertices); // pointer to our vertex array
    vertexData.RowPitch = bufferSize; // size of all our triangle vertex data
    vertexData.SlicePitch = bufferSize; // also the size of our triangle vertex data

    // we are now creating a command with the command list to copy the data from
    // the upload heap to the default heap
    
    s(device->commandList->commandList, buffer, uploadheap, 0, 0, 1, &vertexData);

    // transition the vertex buffer data from copy destination state to vertex buffer state
    device->commandList->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

    bufferView.BufferLocation = buffer->GetGPUVirtualAddress();
    bufferView.Format = DXGI_FORMAT_R32_UINT;
    bufferView.SizeInBytes = bufferSize;

    return true;
}



