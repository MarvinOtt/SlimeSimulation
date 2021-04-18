#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include "../../include/Events.h"

class GraphicsDevice;
class CommandAllocator;
class PipelineState;
class CommandQueue;
class Texture2D;

template<typename T> class VertexBuffer;

class CommandList
{
private:
	using VoidDelegate = std::function<void()>;
	GraphicsDevice* device;

public:
	ID3D12GraphicsCommandList4* commandList;
	Event<VoidDelegate>* CommandListFinished = new Event<VoidDelegate>();

private:
	HRESULT hr;

public:
	CommandList(GraphicsDevice* device, CommandAllocator* commandAllocator, D3D12_COMMAND_LIST_TYPE type, HRESULT& HR);
	bool Reset(CommandAllocator*, PipelineState*);
	bool CloseAndExecute(CommandQueue* queue);
	bool ResourceBarrierTransition(D3D12_RESOURCE_STATES oldstate, D3D12_RESOURCE_STATES newstate, std::vector<ID3D12Resource*> resources);
	bool ResourceBarrierTransition(D3D12_RESOURCE_STATES oldstate, D3D12_RESOURCE_STATES newstate, ID3D12Resource* resource);
	bool SetViewPortScissorRect(D3D12_VIEWPORT viewport, D3D12_RECT scissorRect);
	template<typename T>
	bool DrawInstanced(D3D_PRIMITIVE_TOPOLOGY topology, VertexBuffer<T>* vertexBuffer, UINT vertexCount);
	bool SetDescriptorHeaps(std::vector<ID3D12DescriptorHeap*> heaps);
};

#include "../buffer/VertexBuffer.h"

template<typename T>
bool CommandList::DrawInstanced(D3D_PRIMITIVE_TOPOLOGY topology, VertexBuffer<T>* vertexBuffer, UINT vertexCount)
{
	commandList->IASetPrimitiveTopology(topology);
	commandList->IASetVertexBuffers(0, 1, &(vertexBuffer->bufferView));
	commandList->DrawInstanced(vertexCount, 1, 0, 0);
	return true;
}

