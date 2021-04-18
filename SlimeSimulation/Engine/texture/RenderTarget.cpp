#include "../GraphicsDevice.h"
#include "RenderTarget.h"
#include "../DescriptorHeap.h"
#include "../SwapChain.h"
#include "../../include/d3dx12.h"

RenderTarget::RenderTarget()
{}

RenderTarget::RenderTarget(DescriptorHeap* descHeap)
{
    descriptorHeap = descHeap;
}

bool RenderTarget::Create(GraphicsDevice* device, int width, int height, DescriptorHeap* dsHeap)
{
    Width = width;
    Height = height;

    if (depthStencilBuffer != nullptr)
    {
        depthStencilBuffer->Release();
        depthStencilBuffer = nullptr;
    }

    renderTargets = new ID3D12Resource * [device->frameBufferCount];

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(descriptorHeap->descriptorHeap->GetCPUDescriptorHandleForHeapStart());

    // Create a RTV for each buffer (double buffering is two buffers, tripple buffering is 3).
    for (int i = 0; i < device->frameBufferCount; i++)
    {
        // first we get the n'th buffer in the swap chain and store it in the n'th
        // position of our ID3D12Resource array
        hr = device->swapChain->swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
        if (FAILED(hr))
            return false;

        // the we "create" a render target view which binds the swap chain buffer (ID3D12Resource[n]) to the rtv handle
        device->device->CreateRenderTargetView(renderTargets[i], nullptr, rtvHandle);

        // we increment the rtv handle by the rtv descriptor size we got above
        rtvHandle.Offset(1, descriptorHeap->descriptorSize);
    }



    D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
    depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

    D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
    depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
    depthOptimizedClearValue.DepthStencil.Stencil = 0;

    hr = device->device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, Width, Height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthOptimizedClearValue,
        IID_PPV_ARGS(&depthStencilBuffer)
    );
    if (FAILED(hr))
        return false;
    //dsDescriptorHeap.descriptorHeap->SetName(L"Depth/Stencil Resource Heap");

    device->device->CreateDepthStencilView(depthStencilBuffer, &depthStencilDesc, dsHeap->descriptorHeap->GetCPUDescriptorHandleForHeapStart());
    
    
    return true;
}
