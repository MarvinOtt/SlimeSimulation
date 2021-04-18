#include "DXR_RootSignature.h"
#include "../GraphicsDevice.h"

bool DXR_RootSignature::AddRP_CBV(int ShaderRegister)
{
	D3D12_ROOT_PARAMETER newRP = {};
	newRP.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	newRP.Descriptor.RegisterSpace = 0;
	newRP.Descriptor.ShaderRegister = ShaderRegister;
	rootParams.push_back(newRP);
	return true;
}

bool DXR_RootSignature::AddRP_SRV(int ShaderRegister)
{
	D3D12_ROOT_PARAMETER newRP = {};
	newRP.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	newRP.Descriptor.RegisterSpace = 0;
	newRP.Descriptor.ShaderRegister = ShaderRegister;
	rootParams.push_back(newRP);
	return true;
}

bool DXR_RootSignature::AddRP_UAV(int ShaderRegister)
{
	D3D12_ROOT_PARAMETER newRP = {};
	newRP.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
	newRP.Descriptor.RegisterSpace = 0;
	newRP.Descriptor.ShaderRegister = ShaderRegister;
	rootParams.push_back(newRP);
	return true;
}

bool DXR_RootSignature::AddRP_DescriptorTable(vector<DescriptorRange> descriptorTables)
{
	D3D12_DESCRIPTOR_RANGE* newRanges = new D3D12_DESCRIPTOR_RANGE[descriptorTables.size()];
	for (int i = 0; i < descriptorTables.size(); ++i)
	{
		newRanges[i].RangeType = descriptorTables[i].RangeType;
		newRanges[i].NumDescriptors = descriptorTables[i].NumDescriptors;
		newRanges[i].OffsetInDescriptorsFromTableStart = descriptorTables[i].Offset;
		newRanges[i].BaseShaderRegister = descriptorTables[i].BaseShaderRegister;
		newRanges[i].RegisterSpace = descriptorTables[i].RegisterSpace;
	}
	D3D12_ROOT_PARAMETER newRP = {};
	newRP.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	newRP.DescriptorTable.NumDescriptorRanges = (UINT)descriptorTables.size();
	newRP.DescriptorTable.pDescriptorRanges = newRanges;
	rootParams.push_back(newRP);
	return true;
}

bool DXR_RootSignature::Add_Sampler(D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE uvw_mode, int reg)
{
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = filter;
	sampler.AddressU = uvw_mode;
	sampler.AddressV = uvw_mode;
	sampler.AddressW = uvw_mode;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = reg;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	samplers.push_back(sampler);
	return true;
}

bool DXR_RootSignature::Create()
{
	RS_desc = {};
	RS_desc.NumParameters = (UINT)rootParams.size();
	RS_desc.pParameters = rootParams.data();
	RS_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
	RS_desc.NumStaticSamplers = (UINT)samplers.size();
	RS_desc.pStaticSamplers = samplers.data();

	return true;
}

bool DXR_RootSignature::CreateWithFlags(D3D12_ROOT_SIGNATURE_FLAGS flags)
{
	RS_desc = {};
	RS_desc.NumParameters = (UINT)rootParams.size();
	RS_desc.pParameters = rootParams.data();
	RS_desc.Flags = flags;
	RS_desc.NumStaticSamplers = (UINT)samplers.size();
	RS_desc.pStaticSamplers = samplers.data();

	return true;
}

bool DXR_RootSignature::Build(GraphicsDevice* device)
{
	ID3DBlob* errorBuff; // a buffer holding the error data if any
	ID3DBlob* signature;
	hr = D3D12SerializeRootSignature(&RS_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &errorBuff);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBuff->GetBufferPointer());
		return false;
	}

	hr = device->device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&RS));
	if (FAILED(hr))
		return false;

	return true;
}