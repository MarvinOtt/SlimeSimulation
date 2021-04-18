#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include <vector>

using namespace std;

struct DescriptorRange
{
	D3D12_DESCRIPTOR_RANGE_TYPE RangeType;
	int NumDescriptors;
	int Offset;
	int BaseShaderRegister;
	int RegisterSpace;
	DescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE type, int NumDescriptors2, int Offset2, int BaseShaderRegister2)
	{
		RangeType = type;
		NumDescriptors = NumDescriptors2;
		Offset = Offset2;
		BaseShaderRegister = BaseShaderRegister2;
		RegisterSpace = 0;
	}
};

class GraphicsDevice;

class DXR_RootSignature
{
public:
	D3D12_ROOT_SIGNATURE_DESC RS_desc;
	ID3D12RootSignature* RS;
	vector<D3D12_ROOT_PARAMETER> rootParams;
	vector<D3D12_STATIC_SAMPLER_DESC> samplers;

private:
	HRESULT hr;

public:
	bool AddRP_CBV(int ShaderRegister);
	bool AddRP_SRV(int ShaderRegister);
	bool AddRP_UAV(int ShaderRegister);
	bool AddRP_DescriptorTable(vector<DescriptorRange> descriptorTables);
	bool Add_Sampler(D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE uvw_mode, int reg);
	bool Create();
	bool CreateWithFlags(D3D12_ROOT_SIGNATURE_FLAGS flags);
	bool Build(GraphicsDevice* device);
};

