#include "CommandAllocator.h"
#include "../GraphicsDevice.h"

CommandAllocator::CommandAllocator()
{

}

bool CommandAllocator::Create(GraphicsDevice* device, D3D12_COMMAND_LIST_TYPE type)
{
	hr = device->device->CreateCommandAllocator(type, IID_PPV_ARGS(&commandAllocator));
	if (FAILED(hr))
		return false;

	return true;
}
