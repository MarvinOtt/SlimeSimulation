#include "GraphicsDevice.h"
#include "SwapChain.h"
#include "command/CommandQueue.h"
#include "../include/Framework.h"

SwapChain::SwapChain()
{

}

bool SwapChain::Create(GraphicsDevice* device, CommandQueue* commandQueue)
{
	DXGI_MODE_DESC backBufferDesc = {}; // this is to describe our display mode
	backBufferDesc.Width = device->bufferWidth; // buffer width
	backBufferDesc.Height = device->bufferHeight; // buffer height
	backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the buffer (rgba 32 bits, 8 bits for each chanel)

														// describe our multi-sampling. We are not multi-sampling, so we set the count to 1 (we need at least one sample of course)
	sampleDesc = {};
	sampleDesc.Count = 1; // multisample count (no multisampling, so we just put 1, since we still need 1 sample)

						  // Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = device->frameBufferCount; // number of buffers we have
	swapChainDesc.BufferDesc = backBufferDesc; // our back buffer description
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // this says the pipeline will render to this swap chain
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // dxgi will discard the buffer (data) after we call present
	swapChainDesc.OutputWindow = device->hwnd; // handle to our window
	swapChainDesc.SampleDesc = sampleDesc; // our multi-sampling description
	swapChainDesc.Windowed = true; // set to true, then if in fullscreen must call SetFullScreenState with true for full screen to get uncapped fps
	//swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	IDXGISwapChain* tempSwapChain;
	//hr = device->dxgiFactory->CreateSwapChainForHwnd(commandQueue->commandQueue, device->hwnd, &swapChainDesc, nullptr, nullptr, &tempSwapChain);
	hr = device->dxgiFactory->CreateSwapChain(
		commandQueue->commandQueue, // the queue will be flushed once the swap chain is created
		&swapChainDesc, // give it the swap chain description we created above
		&tempSwapChain // store the created swap chain in a temp IDXGISwapChain interface
	);
	if (FAILED(hr))
		return false;
	d3d_call(tempSwapChain->QueryInterface(IID_PPV_ARGS(&swapChain)));
	//swapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);
	return true;
}
