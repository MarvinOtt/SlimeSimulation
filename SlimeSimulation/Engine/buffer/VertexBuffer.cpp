#include "VertexBuffer.h"
#include <DirectXMath.h>

bool VertexBuffer<DirectX::XMFLOAT4>::CreateFullScreen(GraphicsDevice* device)
{
	bufferSize = sizeof(DirectX::XMFLOAT4) * 6;
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

	hr = device->device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize), // resource description for a buffer
		D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
		nullptr,
		IID_PPV_ARGS(&uploadheap));
	if (FAILED(hr))
		return false;

	DirectX::XMFLOAT4 vList3[] = {
		{-10, -10, 0.5f, 1.0f},
		{-10, 10, 0.5f, 1.0f},
		{10, -10, 0.5f, 1.0f},

		{10, -10, 0.5f, 1.0f},
		{-10, 10, 0.5f, 1.0f},
		{10, 10, 0.5f, 1.0f}
	};

	// store vertex buffer in upload heap
	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<BYTE*>(vList3); // pointer to our vertex array
	vertexData.RowPitch = bufferSize; // size of all our triangle vertex data
	vertexData.SlicePitch = bufferSize; // also the size of our triangle vertex data

	// we are now creating a command with the command list to copy the data from
	// the upload heap to the default heap
	UpdateSubresources(device->commandList->commandList, buffer, uploadheap, 0, 0, 1, &vertexData);

	// transition the vertex buffer data from copy destination state to vertex buffer state
	device->commandList->commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	numVertices = 6;
	bufferView.BufferLocation = buffer->GetGPUVirtualAddress();
	bufferView.StrideInBytes = sizeof(DirectX::XMFLOAT4);
	bufferView.SizeInBytes = bufferSize;

	return true;
}