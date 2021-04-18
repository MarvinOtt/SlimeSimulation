#include "CommandList.h"
#include "../GraphicsDevice.h"
#include "CommandAllocator.h"
#include "../shader/PipelineState.h"
#include "CommandQueue.h"
#include "../texture/Texture2D.h"
#include "../../include/d3dx12.h"


CommandList::CommandList(GraphicsDevice* device, CommandAllocator* commandAllocator, D3D12_COMMAND_LIST_TYPE type, HRESULT& HR)
{
	this->device = device;
	HR = device->device->CreateCommandList(0, type, commandAllocator->commandAllocator, NULL, IID_PPV_ARGS(&commandList));
}

bool CommandList::Reset(CommandAllocator* commandAllocator, PipelineState* pso)
{
	hr = commandList->Reset(commandAllocator->commandAllocator, pso->pipelineState);
	if (FAILED(hr))
		return false;

	return true;
}

bool CommandList::CloseAndExecute(CommandQueue* queue)
{
	hr = commandList->Close();
	if (FAILED(hr))
		return false;
	ID3D12CommandList* ppCommandLists[] = { commandList };
	queue->commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	return true;
}

bool CommandList::ResourceBarrierTransition(D3D12_RESOURCE_STATES oldstate, D3D12_RESOURCE_STATES newstate, std::vector<ID3D12Resource*> resources)
{
	for (int i = 0; i < resources.size(); ++i)
	{
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resources[i], oldstate, newstate));
	}

	return true;
}

bool CommandList::ResourceBarrierTransition(D3D12_RESOURCE_STATES oldstate, D3D12_RESOURCE_STATES newstate, ID3D12Resource* resource)
{
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource, oldstate, newstate));

	return true;
}

bool CommandList::SetViewPortScissorRect(D3D12_VIEWPORT viewport, D3D12_RECT scissorRect)
{
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	return true;
}

bool CommandList::SetDescriptorHeaps(std::vector<ID3D12DescriptorHeap*> heaps)
{
	commandList->SetDescriptorHeaps((UINT)heaps.size(), heaps.data());
	return true;
}


