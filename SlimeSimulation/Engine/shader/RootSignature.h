#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include "../../include/d3dx12.h"
#include <vector>

using namespace std;

class GraphicsDevice;
class Sampler;

class RootSignature
{
public:
	ID3D12RootSignature* rootSignature;

private:
	HRESULT hr;
	int paramCount, descriptorTableCount;
	vector<D3D12_STATIC_SAMPLER_DESC> samplers;
	vector<D3D12_DESCRIPTOR_RANGE>* descriptorTables;
	int* descriptorTables_ID;

	D3D12_ROOT_PARAMETER* rootParameters;

public:
	RootSignature(int parameterCount);
	bool SetDescriptorTables(vector<int> rootIDs);
	bool AddSampler(Sampler* sampler);
	bool AddDescrTableRange(int rootID, D3D12_DESCRIPTOR_RANGE_TYPE type, UINT rangeLength, UINT shaderRegisterStart, UINT heapOffset);
	bool SetCBV(int rootID, UINT shaderRegister, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
	bool Set32BitConstant(int rootID, UINT shaderRegister, UINT num32BitValues, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);
	bool Create(GraphicsDevice*);
	bool Create(GraphicsDevice* device, D3D12_ROOT_SIGNATURE_DESC desc);
};