#include "Sampler.h"

//Sampler::Sampler()
//{
//
//}
bool Sampler::Create(D3D12_STATIC_SAMPLER_DESC desc)
{
	sampler = desc;
	return true;
}

bool Sampler::Create(D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE addressMode, UINT shaderRegister)
{
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = filter;
	sampler.AddressU = addressMode;
	sampler.AddressV = addressMode;
	sampler.AddressW = addressMode;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = shaderRegister;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	Create(sampler);
	return true;
}
