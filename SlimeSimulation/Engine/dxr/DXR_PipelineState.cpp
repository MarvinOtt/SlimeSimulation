#include "../GraphicsDevice.h"
#include "DXR_PipelineState.h"
#include <D3Dcompiler.h>
#include "../../include/dxcapi.use.h"
#include "../../include/Framework.h"
#include <sstream>
#include <fstream>

static dxc::DxcDllSupport gDxcDllHelper;

DXR_PipelineState::DXR_PipelineState(int TraceRecursionDepth2)
{
    TraceRecursionDepth = TraceRecursionDepth2;
}

DXR_PipelineState::~DXR_PipelineState()
{
    if (pipelineState != nullptr)
        pipelineState->Release();
}

bool DXR_PipelineState::SetShaders(LPCWSTR shaderFileName2, vector<LPCWSTR> shaderNames2)
{
    shaderFileName = shaderFileName2;
    shaderNames = shaderNames2;
    return true;
}

bool DXR_PipelineState::AddHitGroup(LPCWSTR AHS, LPCWSTR CHS, LPCWSTR HitGroupName)
{
    HitGroup newHitGroup(AHS, CHS, HitGroupName);
    hitGroups.push_back(newHitGroup);
    return true;
}

bool DXR_PipelineState::AddRootSignature2Shader(LPCWSTR shader2, D3D12_ROOT_SIGNATURE_DESC RS_Desc2, LPCWSTR hitGroup)
{
    Shader_RS_Pair newPair(shader2, RS_Desc2, hitGroup);
    shaderRS_Pairs.push_back(newPair);
    return true;
}




ID3DBlobPtr compileLibrary(const WCHAR* filename, const WCHAR* targetString)
{
    // Initialize the helper
    d3d_call(gDxcDllHelper.Initialize());
    IDxcCompilerPtr pCompiler;
    IDxcLibraryPtr pLibrary;
    d3d_call(gDxcDllHelper.CreateInstance(CLSID_DxcCompiler, &pCompiler));
    d3d_call(gDxcDllHelper.CreateInstance(CLSID_DxcLibrary, &pLibrary));

    // Open and read the file
    std::ifstream shaderFile(filename);
    if (shaderFile.good() == false)
    {
        msgBox("Can't open file " + wstring_2_string(std::wstring(filename)));
        return nullptr;
    }
    std::stringstream strStream;
    strStream << shaderFile.rdbuf();
    std::string shader = strStream.str();

    // Create blob from the string
    IDxcBlobEncodingPtr pTextBlob;
    d3d_call(pLibrary->CreateBlobWithEncodingFromPinned((LPBYTE)shader.c_str(), (uint32_t)shader.size(), 0, &pTextBlob));

    // Compile
    IDxcOperationResultPtr pResult;
    d3d_call(pCompiler->Compile(pTextBlob, filename, L"", targetString, nullptr, 0, nullptr, 0, nullptr, &pResult));

    // Verify the result
    HRESULT resultCode;
    d3d_call(pResult->GetStatus(&resultCode));
    if (FAILED(resultCode))
    {
        IDxcBlobEncodingPtr pError;
        d3d_call(pResult->GetErrorBuffer(&pError));
        std::string log = convertBlobToString(pError.GetInterfacePtr());
        msgBox("Compiler error:\n" + log);
        return nullptr;
    }

    MAKE_SMART_COM_PTR(IDxcBlob);
    IDxcBlobPtr pBlob;
    d3d_call(pResult->GetResult(&pBlob));
    return pBlob;
}

ID3D12RootSignaturePtr createRootSignature(ID3D12Device5Ptr pDevice, const D3D12_ROOT_SIGNATURE_DESC desc)
{
    ID3DBlobPtr pSigBlob;
    ID3DBlobPtr pErrorBlob;
    HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &pSigBlob, &pErrorBlob);
    if (FAILED(hr))
    {
        std::string msg = convertBlobToString(pErrorBlob.GetInterfacePtr());
        msgBox(msg);
        return nullptr;
    }
    ID3D12RootSignaturePtr pRootSig;
    d3d_call(pDevice->CreateRootSignature(0, pSigBlob->GetBufferPointer(), pSigBlob->GetBufferSize(), IID_PPV_ARGS(&pRootSig)));
    return pRootSig;
}

struct DxilLibrary
{
    DxilLibrary(ID3DBlobPtr pBlob, const WCHAR* entryPoint[], uint32_t entryPointCount) : pShaderBlob(pBlob)
    {
        stateSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
        stateSubobject.pDesc = &dxilLibDesc;

        dxilLibDesc = {};
        exportDesc.resize(entryPointCount);
        exportName.resize(entryPointCount);
        if (pBlob)
        {
            dxilLibDesc.DXILLibrary.pShaderBytecode = pBlob->GetBufferPointer();
            dxilLibDesc.DXILLibrary.BytecodeLength = pBlob->GetBufferSize();
            dxilLibDesc.NumExports = entryPointCount;
            dxilLibDesc.pExports = exportDesc.data();

            for (uint32_t i = 0; i < entryPointCount; i++)
            {
                exportName[i] = entryPoint[i];
                exportDesc[i].Name = exportName[i].c_str();
                exportDesc[i].Flags = D3D12_EXPORT_FLAG_NONE;
                exportDesc[i].ExportToRename = nullptr;
            }
        }
    };

    DxilLibrary() : DxilLibrary(nullptr, nullptr, 0) {}

    D3D12_DXIL_LIBRARY_DESC dxilLibDesc = {};
    D3D12_STATE_SUBOBJECT stateSubobject{};
    ID3DBlobPtr pShaderBlob;
    std::vector<D3D12_EXPORT_DESC> exportDesc;
    std::vector<std::wstring> exportName;
};

//DxilLibrary createDxilLibrary()
//{
//    // Compile the shader
//    ID3DBlobPtr pDxilLib = compileLibrary(L"DXR_Shader.hlsl", L"lib_6_3");
//    const WCHAR* entryPoints[] = { DXR_PipelineState::kRayGenShader, DXR_PipelineState::kMissShader, DXR_PipelineState::kTriangleChs, DXR_PipelineState::kPlaneChs, DXR_PipelineState::kShadowMiss, DXR_PipelineState::kShadowChs };
//    return DxilLibrary(pDxilLib, entryPoints, arraysize(entryPoints));
//}

struct HitProgram
{
    HitProgram(LPCWSTR ahsExport, LPCWSTR chsExport, const std::wstring& name) : exportName(name)
    {
        desc = {};
        desc.AnyHitShaderImport = ahsExport;
        desc.ClosestHitShaderImport = chsExport;
        desc.HitGroupExport = exportName.c_str();

        subObject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
        subObject.pDesc = &desc;
    }

    std::wstring exportName;
    D3D12_HIT_GROUP_DESC desc;
    D3D12_STATE_SUBOBJECT subObject;
};

struct ExportAssociation
{
    ExportAssociation(const WCHAR* exportNames[], uint32_t exportCount, const D3D12_STATE_SUBOBJECT* pSubobjectToAssociate)
    {
        association.NumExports = exportCount;
        association.pExports = exportNames;
        association.pSubobjectToAssociate = pSubobjectToAssociate;

        subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
        subobject.pDesc = &association;
    }

    D3D12_STATE_SUBOBJECT subobject = {};
    D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION association = {};
};

struct LocalRootSignature
{
    LocalRootSignature(ID3D12Device5Ptr pDevice, const D3D12_ROOT_SIGNATURE_DESC desc)
    {
        pRootSig = createRootSignature(pDevice, desc);
        pInterface = pRootSig.GetInterfacePtr();
        subobject.pDesc = &pInterface;
        subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
    }
    ID3D12RootSignaturePtr pRootSig;
    ID3D12RootSignature* pInterface = nullptr;
    D3D12_STATE_SUBOBJECT subobject = {};
};

struct GlobalRootSignature
{
    GlobalRootSignature(ID3D12Device5Ptr pDevice, const D3D12_ROOT_SIGNATURE_DESC& desc)
    {
        pRootSig = createRootSignature(pDevice, desc);
        pInterface = pRootSig.GetInterfacePtr();
        subobject.pDesc = &pInterface;
        subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
    }
    ID3D12RootSignaturePtr pRootSig;
    ID3D12RootSignature* pInterface = nullptr;
    D3D12_STATE_SUBOBJECT subobject = {};
};

struct RootSignature
{
    RootSignature(ID3D12Device5Ptr pDevice, const D3D12_ROOT_SIGNATURE_DESC& desc, bool IsLocal)
    {
        pRootSig = createRootSignature(pDevice, desc);
        pInterface = pRootSig.GetInterfacePtr();
        subobject.pDesc = &pInterface;
        if(IsLocal)
            subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
        else
            subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
    }
    ID3D12RootSignaturePtr pRootSig;
    ID3D12RootSignature* pInterface = nullptr;
    D3D12_STATE_SUBOBJECT subobject = {};
};

struct ShaderConfig
{
    ShaderConfig(uint32_t maxAttributeSizeInBytes, uint32_t maxPayloadSizeInBytes)
    {
        shaderConfig.MaxAttributeSizeInBytes = maxAttributeSizeInBytes;
        shaderConfig.MaxPayloadSizeInBytes = maxPayloadSizeInBytes;

        subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
        subobject.pDesc = &shaderConfig;
    }

    D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
    D3D12_STATE_SUBOBJECT subobject = {};
};

struct PipelineConfig
{
    PipelineConfig(uint32_t maxTraceRecursionDepth)
    {
        config.MaxTraceRecursionDepth = maxTraceRecursionDepth;

        subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
        subobject.pDesc = &config;
    }

    D3D12_RAYTRACING_PIPELINE_CONFIG config = {};
    D3D12_STATE_SUBOBJECT subobject = {};
};

bool DXR_PipelineState::Build(GraphicsDevice* device, int maxPayLoadAttrSize, int maxPayLoadSize)
{
    int subobject_count = 1 + (int)hitGroups.size() + (int)shaderRS_Pairs.size() * 2;
    if (shaderNames.size() > shaderRS_Pairs.size())
        subobject_count += 2;
    subobject_count += 4;

    D3D12_STATE_SUBOBJECT* subobjects = new D3D12_STATE_SUBOBJECT[subobject_count]();
    uint32_t index = 0;

    // Create the DXIL library
    ID3DBlobPtr pDxilLib = compileLibrary(shaderFileName, L"lib_6_3");
    DxilLibrary dxilLib(pDxilLib, shaderNames.data(), (uint32_t)shaderNames.size());
    subobjects[index++] = dxilLib.stateSubobject;

    // Hit Groups
    for (int i = 0; i < hitGroups.size(); ++i)
    {
        HitProgram* hitProgram = new HitProgram(hitGroups[i].AHS, hitGroups[i].CHS, hitGroups[i].HitGroupName);
        subobjects[index++] = hitProgram->subObject;
    }

    // Shader - RootSignature Pairs
    for (int i = 0; i < shaderRS_Pairs.size(); ++i)
    {
        LocalRootSignature* rootSignature = new LocalRootSignature(device->device, shaderRS_Pairs[i].rootSignature);
        subobjects[index] = rootSignature->subobject; // 3 Ray Gen Root Sig

        uint32_t rootIndex = index++; // 3
        ExportAssociation* rootAssociation = new ExportAssociation(&shaderRS_Pairs[i].shader, 1, &(subobjects[rootIndex]));
        subobjects[index++] = rootAssociation->subobject;
    }

    // Empty RootSignature Shaders
    vector<LPCWSTR> remainingShaders;
    if (shaderNames.size() > shaderRS_Pairs.size())
    {
        for (int i = 0; i < shaderNames.size(); ++i)
        {
            int j;
            for (j = 0; j < shaderRS_Pairs.size(); ++j)
            {
                if (std::wstring(shaderNames[i]) == std::wstring(shaderRS_Pairs[j].shader))
                    break;
            }
            if (j == shaderRS_Pairs.size())
            {
                remainingShaders.push_back(shaderNames[i]);
            }
        }
        D3D12_ROOT_SIGNATURE_DESC emptyDesc = {};
        emptyDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
        LocalRootSignature* rootSignature = new LocalRootSignature(device->device, emptyDesc);
        subobjects[index] = rootSignature->subobject;

        uint32_t rootIndex = index++;
        ExportAssociation* rootAssociation = new ExportAssociation(remainingShaders.data(), (uint32_t)remainingShaders.size(), &(subobjects[rootIndex]));
        subobjects[index++] = rootAssociation->subobject;
    }

    // Bind the payload size to the programs
    ShaderConfig shaderConfig(maxPayLoadAttrSize, maxPayLoadSize);
    subobjects[index] = shaderConfig.subobject; // 9 Shader Config

    uint32_t shaderConfigIndex = index++; // 9
    ExportAssociation configAssociation(shaderNames.data(), (uint32_t)shaderNames.size(), &(subobjects[shaderConfigIndex]));
    subobjects[index++] = configAssociation.subobject; // 10 Associate Shader Config to all shaders and hit groups

    // Create the pipeline config
    PipelineConfig config(TraceRecursionDepth);
    subobjects[index++] = config.subobject; // 11

    // Create the global root signature and store the empty signature
    GlobalRootSignature root(device->device, {});
    emptyRootSig = root.pRootSig;
    subobjects[index++] = root.subobject; // 12

    

    // Create the state
    D3D12_STATE_OBJECT_DESC desc;
    desc.NumSubobjects = index;
    desc.pSubobjects = subobjects;
    desc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;

    hr = device->device->CreateStateObject(&desc, IID_PPV_ARGS(&pipelineState));
    if (FAILED(hr))
        return false;
    return true;
}