#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
class GraphicsDevice;

class RootParameter
{
public:
	D3D12_ROOT_PARAMETER rootParameter;

private:
	HRESULT hr;

public:
	RootParameter(D3D12_ROOT_PARAMETER);
};