#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include <vector>

using namespace std;

class GraphicsDevice;
class Buffer;
class DXR_PipelineState;
class DXR_ACS_TOP;
class DXR_ACS_OBJ;

class DXR_ShaderTable
{
public:
    Buffer* shaderTable = nullptr;
    Buffer* tempShaderTable = nullptr;
    LPCWSTR* missShaders;
    UINT64** missShaders_data;
    int* missShaders_data_count;

    LPCWSTR rayGenName;
    UINT64* rayGenData;
    int rayGenData_count;

    UINT64** geomData;
	LPCWSTR* geomHitGroup;
    int shaderTableEntrySize;

private:
    HRESULT hr;
    DXR_ACS_TOP* acs_top;
    vector<D3D12_ROOT_PARAMETER> rootParams;
    int missIndex, hitIndex, entryNumber;
    GraphicsDevice* device;

public:
    DXR_ShaderTable(DXR_ACS_TOP* acs_top);
    ~DXR_ShaderTable();
    bool SetRayGenShader(LPCWSTR name, UINT64* data, int datasize);
    bool SetMissShader(int index, LPCWSTR name, UINT64* data, int datasize);
    bool Create(GraphicsDevice* device, DXR_PipelineState* pipelineState);
    bool SetGeomData(DXR_ACS_OBJ* geom, int rayindex, UINT64* data, LPCWSTR hitgroup);
    D3D12_DISPATCH_RAYS_DESC CreateRayDesc();
    D3D12_DISPATCH_RAYS_DESC CreateRayDesc(int Width, int Height);

private:
    void UploadingFinished();
};

