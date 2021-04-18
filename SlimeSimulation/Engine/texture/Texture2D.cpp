#include "../GraphicsDevice.h"
#include "Texture2D.h"
#include "../DescriptorHeap.h"
#include "../command/CommandList.h"
#include "../../include/d3dx12.h"

Texture2D::Texture2D(LPCWSTR name)
{
	this->name = name;
}

Texture2D::Texture2D(LPCWSTR name, GraphicsDevice* device, DXGI_FORMAT format, int Width, int Height)
{
	this->name = name;
	Create(device, format, Width, Height);
}

bool Texture2D::Create(GraphicsDevice* device, D3D12_RESOURCE_DESC Desc)
{
	desc = Desc;
	// create default heap
	// default heap is memory on the GPU. Only the GPU has access to this memory
	// To get data into this heap, we will have to upload the data using
	// an upload heap
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

	device->device->GetCopyableFootprints(&desc, 0, 1, 0, nullptr, nullptr, nullptr, &bufferSize);

	
	return true;
}

bool Texture2D::Create(GraphicsDevice* device, DXGI_FORMAT format, int Width, int Height)
{
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.DepthOrArraySize = 1;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Format = format;
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	resDesc.Width = Width;
	resDesc.Height = Height;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	Create(device, resDesc);
	return true;
}
bool Texture2D::Create(GraphicsDevice* device, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flag, int Width, int Height)
{
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.DepthOrArraySize = 1;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Format = format;
	resDesc.Flags = flag;
	resDesc.Width = Width;
	resDesc.Height = Height;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.MipLevels = 1;
	resDesc.SampleDesc.Count = 1;
	Create(device, resDesc);
	return true;
}



bool Texture2D::SetData(GraphicsDevice* device, BYTE* rawtexdata, int imagesize, DescriptorHeap* descHeap, int heapindex)
{
	// create upload heap
	// upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
	// We will upload the vertex buffer using this heap to the default heap
	if (uploadheap == nullptr)
	{
		hr = device->device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize), // resource description for a buffer
			D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
			nullptr,
			IID_PPV_ARGS(&uploadheap));
		if (FAILED(hr))
			return false;
	}


	// store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA texData = {};
	texData.pData = rawtexdata; // pointer to our vertex array
	texData.RowPitch = imagesize / desc.Height; // size of all our triangle vertex data
	texData.SlicePitch = imagesize; // also the size of our triangle vertex data

	// we are now creating a command with the command list to copy the data from
	// the upload heap to the default heap
	UpdateSubresources(device->commandList->commandList, buffer, uploadheap, 0, 0, 1, &texData);

	CreateView(device, descHeap, heapindex);

	return true;
}

void Texture2D::CreateView(GraphicsDevice* device, DescriptorHeap* descHeap, int heapindex)
{
	// transition the vertex buffer data from copy destination state to vertex buffer state
	device->commandList->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	// now we create a shader resource view (descriptor that points to the texture and describes it)
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	CD3DX12_CPU_DESCRIPTOR_HANDLE newhandle(descHeap->descriptorHeap->GetCPUDescriptorHandleForHeapStart(), heapindex, descHeap->descriptorSize);
	device->device->CreateShaderResourceView(buffer, &srvDesc, newhandle);
	
}

Texture2D::~Texture2D()
{
	if (uploadheap != nullptr)
		uploadheap->Release();
	if (buffer != nullptr)
		buffer->Release();
}

