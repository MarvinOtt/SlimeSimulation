#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include <vector>

using namespace std;

class GraphicsDevice;

struct HitGroup
{
    LPCWSTR AHS;
    LPCWSTR CHS;
    LPCWSTR HitGroupName;

    HitGroup(LPCWSTR AHS2, LPCWSTR CHS2, LPCWSTR HitGroupName2)
    {
        AHS = AHS2;
        CHS = CHS2;
        HitGroupName = HitGroupName2;
    }
};

struct Shader_RS_Pair
{
    LPCWSTR shader;
    LPCWSTR hitGroup;
    D3D12_ROOT_SIGNATURE_DESC rootSignature;

    Shader_RS_Pair(LPCWSTR shader2, D3D12_ROOT_SIGNATURE_DESC RS_Desc, LPCWSTR hitGroup2 = nullptr)
    {
        shader = shader2;
        rootSignature = RS_Desc;
        hitGroup = hitGroup2;
    }
};

class DXR_PipelineState
{
public:
    ID3D12RootSignature* emptyRootSig;
    ID3D12StateObject* pipelineState = nullptr;
    int TraceRecursionDepth;
    LPCWSTR shaderFileName;
    vector<Shader_RS_Pair> shaderRS_Pairs;  

private:
    HRESULT hr;
    vector<HitGroup> hitGroups;
    vector<LPCWSTR> shaderNames;
    

public:
    DXR_PipelineState(int TraceRecursionDepth);
    ~DXR_PipelineState();
    bool SetShaders(LPCWSTR shaderFileName, vector<LPCWSTR> shaderNames);
    bool AddHitGroup(LPCWSTR AHS, LPCWSTR CHS, LPCWSTR HitGroupName);
    bool AddRootSignature2Shader(LPCWSTR shader, D3D12_ROOT_SIGNATURE_DESC RS_Desc, LPCWSTR hitGroup = nullptr);

    bool Build(GraphicsDevice* device, int maxPayLoadAttrSize, int maxPayLoadSize);


};
