#include "RootSignature.h"
#include "../GraphicsDevice.h"
#include "Sampler.h"

RootSignature::RootSignature(int parameterCount)
{
	paramCount = parameterCount;
	rootParameters = new D3D12_ROOT_PARAMETER[paramCount];
}

bool RootSignature::AddSampler(Sampler* sampler)
{
	samplers.push_back(sampler->sampler);
	return true;
}

bool RootSignature::SetDescriptorTables(vector<int> rootIDs)
{
	descriptorTableCount = (int)rootIDs.size();
	descriptorTables_ID = new int[rootIDs.size()];
	descriptorTables = new vector<D3D12_DESCRIPTOR_RANGE>[rootIDs.size()];
	for (int i = 0; i < rootIDs.size(); i++)
	{
		descriptorTables_ID[i] = rootIDs[i];
	}
	return true;
}

bool RootSignature::AddDescrTableRange(int rootID, D3D12_DESCRIPTOR_RANGE_TYPE type, UINT rangeLength, UINT shaderRegisterStart, UINT heapOffset)
{
	int vectorID = -1;
	for (int i = 0; i < descriptorTableCount; i++)
	{
		if (descriptorTables_ID[i] == rootID)
			vectorID = i;
	}
	if (vectorID < 0)
		return false;
	else 
	{
		D3D12_DESCRIPTOR_RANGE range = {};
		range.RangeType = type;
		range.NumDescriptors = rangeLength;
		range.BaseShaderRegister = shaderRegisterStart;
		range.RegisterSpace = 0;
		range.OffsetInDescriptorsFromTableStart = heapOffset;
		descriptorTables[vectorID].push_back(range);
	}
	return true;
}

bool RootSignature::SetCBV(int rootID, UINT shaderRegister, D3D12_SHADER_VISIBILITY visibility)
{
	D3D12_ROOT_DESCRIPTOR CBVDescriptor;
	CBVDescriptor.RegisterSpace = 0;
	CBVDescriptor.ShaderRegister = shaderRegister;

	rootParameters[rootID].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[rootID].Descriptor = CBVDescriptor;
	rootParameters[rootID].ShaderVisibility = visibility;
	return true;
}

bool RootSignature::Set32BitConstant(int rootID, UINT shaderRegister, UINT num32BitValues, D3D12_SHADER_VISIBILITY visibility)
{
	rootParameters[rootID].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameters[rootID].Constants.ShaderRegister = shaderRegister;
	rootParameters[rootID].Constants.RegisterSpace = 0;
	rootParameters[rootID].Constants.Num32BitValues = num32BitValues;
	rootParameters[rootID].ShaderVisibility = visibility;
	return true;
}

bool RootSignature::Create(GraphicsDevice* device)
{
	for (int i = 0; i < descriptorTableCount; i++)
	{
		int ID = descriptorTables_ID[i];

		D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
		descriptorTable.NumDescriptorRanges = (UINT)descriptorTables[i].size(); // we only have one range
		descriptorTable.pDescriptorRanges = descriptorTables[i].data(); // the pointer to the beginning of our ranges array

		rootParameters[ID].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // this is a descriptor table
		rootParameters[ID].DescriptorTable = descriptorTable; // this is our descriptor table for this root parameter
		rootParameters[ID].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // our pixel shader will be the only shader accessing this parameter for now
	}

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(paramCount,
		rootParameters, // a pointer to the beginning of our root parameters array
		(UINT)samplers.size(),
		samplers.size() == 0 ? nullptr : samplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

	ID3DBlob* errorBuff; // a buffer holding the error data if any
	ID3DBlob* signature;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &errorBuff);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBuff->GetBufferPointer());
		return false;
	}

	hr = device->device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	if (FAILED(hr))
		return false;

	return true;
}

bool RootSignature::Create(GraphicsDevice* device, D3D12_ROOT_SIGNATURE_DESC desc)
{
	ID3DBlob* errorBuff; // a buffer holding the error data if any
	ID3DBlob* signature;
	hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &errorBuff);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBuff->GetBufferPointer());
		return false;
	}

	hr = device->device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	if (FAILED(hr))
		return false;

	return true;
}
