#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include <vector>

class GraphicsDevice;
class DXR_RootSignature;
class Shader;
using namespace std;

class PipelineState
{
public:
	ID3D12PipelineState* pipelineState;
	Shader* PS = nullptr;
	Shader* VS = nullptr;
	Shader* CS = nullptr;
	vector<DXGI_FORMAT> rtv_formats;

private:
	HRESULT hr;

public:
	PipelineState();
	bool SetShaders(Shader* vs, Shader* ps);
	bool SetCS(Shader* cs);
	bool AddRTV(DXGI_FORMAT format);
	bool CreateCompute(GraphicsDevice* device, DXR_RootSignature* RS);
	bool Create(GraphicsDevice*, D3D12_GRAPHICS_PIPELINE_STATE_DESC desc);
	bool Create(GraphicsDevice* device, D3D12_INPUT_ELEMENT_DESC* IE_desc, int IE_desc_size, DXR_RootSignature* RS, D3D12_PRIMITIVE_TOPOLOGY_TYPE top_type);
};

