#include "Buffer.h"
#include "../GraphicsDevice.h"
#include "../../include/d3dx12.h"
#include "../../include/Framework.h"


Buffer::Buffer(GraphicsDevice* device, UINT64 buffersize, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initState, D3D12_HEAP_TYPE heap_type, DXGI_FORMAT _format)
{
	bufferSize = buffersize;
	format = _format;

	D3D12_RESOURCE_DESC bufDesc = {};
	bufDesc.Alignment = 0;
	bufDesc.DepthOrArraySize = 1;
	bufDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufDesc.Flags = flags;
	bufDesc.Format = format;
	bufDesc.Height = 1;
	bufDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufDesc.MipLevels = 1;
	bufDesc.SampleDesc.Count = 1;
	bufDesc.SampleDesc.Quality = 0;
	bufDesc.Width = buffersize;

	// create default heap
	// default heap is memory on the GPU. Only the GPU has access to this memory
	// To get data into this heap, we will have to upload the data using
	// an upload heap
	hr = device->device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(heap_type), // a default heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&bufDesc, // resource description for a buffer
		initState, // we will start this heap in the copy destination state since we will copy data
										// from the upload heap to this heap
		nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
		IID_PPV_ARGS(&buffer));

}

Buffer::~Buffer()
{
	if (buffer != nullptr)
		buffer->Release();
}

void Buffer::SetData(void* data)
{
	uint8_t* pData;
	d3d_call(buffer->Map(0, nullptr, (void**)&pData));
	memcpy(pData, data, bufferSize);
	buffer->Unmap(0, nullptr);
}