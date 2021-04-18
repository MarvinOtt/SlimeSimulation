#include "DescriptorHeap.h"
#include "GraphicsDevice.h"
#include "texture/Texture1D.h"
#include "texture/Texture2D.h"
#include "texture/Texture3D.h"
#include "buffer/Buffer.h"

DescriptorHeap::DescriptorHeap()
{
	descriptorType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
}

DescriptorHeap::DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
{
	descriptorType = type;
	descriptorFlags = flags;
}

bool DescriptorHeap::Create(GraphicsDevice* device, int num)	
{
	this->device = device;
	descriptorNum = num;
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = descriptorNum; // number of descriptors for this heap.
	rtvHeapDesc.Type = descriptorType; // Type

													   // This heap will not be directly referenced by the shaders (not shader visible), as this will store the output from the pipeline
													   // otherwise we would set the heap's flag to D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
	rtvHeapDesc.Flags = descriptorFlags;    
	hr = device->device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	if (FAILED(hr))
		return false;

	// descriptor sizes may vary from device to device, which is why there is no set size and we must ask the 
	// device to give us the size. we will use this size to increment a descriptor handle offset
	descriptorSize = device->device->GetDescriptorHandleIncrementSize(descriptorType);

	return true;
}

bool DescriptorHeap::SetTexture2D(Texture2D* tex, int index)
{
	if (descriptorType != D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		return false;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

	D3D12_CPU_DESCRIPTOR_HANDLE Handle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	Handle.ptr += device->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * index;
	device->device->CreateUnorderedAccessView(tex->buffer, nullptr, &uavDesc, Handle);
	return true;
}

bool DescriptorHeap::SetTexture3D(Texture3D* tex, int index)
{
	if (descriptorType != D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		return false;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	uavDesc.Format = tex->desc.Format;
	uavDesc.Texture3D.WSize = tex->desc.DepthOrArraySize;

	D3D12_CPU_DESCRIPTOR_HANDLE Handle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	Handle.ptr += device->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * index;
	device->device->CreateUnorderedAccessView(tex->buffer, nullptr, &uavDesc, Handle);
	return true;
}

bool DescriptorHeap::SetBuffer(Buffer* buf, int index)
{
	if (descriptorType != D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		return false;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Format = buf->format;

	D3D12_CPU_DESCRIPTOR_HANDLE Handle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	Handle.ptr += device->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * index;
	device->device->CreateUnorderedAccessView(buf->buffer, nullptr, &uavDesc, Handle);
	return true;
}

bool DescriptorHeap::SetTexture1D(Texture1D* tex, int index)
{
	if (descriptorType != D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		return false;

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
	uavDesc.Format = tex->desc.Format;

	D3D12_CPU_DESCRIPTOR_HANDLE Handle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	Handle.ptr += device->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * index;
	device->device->CreateUnorderedAccessView(tex->buffer, nullptr, &uavDesc, Handle);
	return true;
}

bool DescriptorHeap::SetSRV(D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc, int index)
{
	if (descriptorType != D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		return false;

	D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	srvHandle.ptr += device->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * index;
	device->device->CreateShaderResourceView(nullptr, &srvDesc, srvHandle);
	return true;
}
