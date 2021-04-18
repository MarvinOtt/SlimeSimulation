#include "CommandQueue.h"
#include "../GraphicsDevice.h"

CommandQueue::CommandQueue()
{

}

bool CommandQueue::Create(GraphicsDevice* device, D3D12_COMMAND_LIST_TYPE type)
{
	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = type; // direct means the gpu can directly execute this command queue

	hr = device->device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&commandQueue)); // create the command queue
	if (FAILED(hr))
		return false;
	return true;
}

