#include "Fence.h"
#include "GraphicsDevice.h"

HANDLE Fence::fenceEvent = 0;

Fence::Fence()
{

}

bool Fence::Create(GraphicsDevice* device)
{
	hr = device->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	if (FAILED(hr))
		return false;

	fenceValue = 0; // set the initial fence value to 0

	if (fenceEvent == nullptr)
	{
		fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (fenceEvent == nullptr)
			return false;
	}

	return true;
}
