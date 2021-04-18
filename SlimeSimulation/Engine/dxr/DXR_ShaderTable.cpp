#include "DXR_ShaderTable.h"
#include "../GraphicsDevice.h"
#include "../buffer/Buffer.h"
#include "../dxr/DXR_PipelineState.h"
#include "../dxr/acs/DXR_ACS_TOP.h"
#include "../dxr/acs/DXR_ACS_OBJ.h"
#include "../../include/Framework.h"

DXR_ShaderTable::DXR_ShaderTable(DXR_ACS_TOP* acs_top2)
{
	acs_top = acs_top2;
	missShaders = new LPCWSTR[acs_top->numberOfRays];
	missShaders_data = new UINT64*[acs_top->numberOfRays];
	missShaders_data_count = new int[acs_top->numberOfRays];

	// Get geometry size
	int count = 0;
	for (int i = 0; i < acs_top->ACS_OBJ_buffers.size(); ++i)
	{
		count += (int)acs_top->ACS_OBJ_buffers[i]->ACS_BOT->geomDesc.size();
	}
	geomData = new UINT64*[count * acs_top->numberOfRays];
	geomHitGroup = new LPCWSTR[count * acs_top->numberOfRays];
}

DXR_ShaderTable::~DXR_ShaderTable()
{
	if (shaderTable != nullptr)
		delete shaderTable;

	delete[] missShaders;
	delete[] missShaders_data;
	delete[] missShaders_data_count;
	delete[] geomData;
	delete[] geomHitGroup;
}

bool DXR_ShaderTable::SetRayGenShader(LPCWSTR name, UINT64* data, int datasize)
{
	rayGenName = name;
	rayGenData = data;
	rayGenData_count = datasize;

	return true;
}

bool DXR_ShaderTable::SetMissShader(int index, LPCWSTR name, UINT64* data, int datasize)
{
	if (index < 0 || index >= acs_top->numberOfRays)
		return false;

	missShaders[index] = name;
	missShaders_data[index] = data;
	missShaders_data_count[index] = datasize;

	return true;
}

bool DXR_ShaderTable::Create(GraphicsDevice* device2, DXR_PipelineState* pipelineState)
{
	if (shaderTable != nullptr)
		delete shaderTable;

	device = device2;
	// Get largest entrys size
	int largestSize = 0;
	for (int i = 0; i < pipelineState->shaderRS_Pairs.size(); ++i)
	{
		int cursize = 0;
		D3D12_ROOT_SIGNATURE_DESC cur_RS = pipelineState->shaderRS_Pairs[i].rootSignature;
		for (UINT j = 0; j < cur_RS.NumParameters; ++j)
		{
			if (cur_RS.pParameters[j].ParameterType == D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)
				cursize += 4;
			else
				cursize += 8;
		}
		if (cursize > largestSize)
			largestSize = cursize;
	}

	// Get number of entrys
	int totalgeometries = 0;
	for (int i = 0; i < acs_top->ACS_OBJ_buffers.size(); ++i)
	{
		totalgeometries += (int)acs_top->ACS_OBJ_buffers[i]->ACS_BOT->geomDesc.size();
	}
	entryNumber = 1 + acs_top->numberOfRays + totalgeometries * acs_top->numberOfRays;

	// Calculate the size and create the buffer
	shaderTableEntrySize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	shaderTableEntrySize += largestSize; // The ray-gen's descriptor table
	shaderTableEntrySize = align_to(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT, shaderTableEntrySize);
	uint32_t shaderTableSize = shaderTableEntrySize * entryNumber;

	// For simplicity, we create the shader-table on the upload heap. You can also create it on the default heap
	shaderTable = new Buffer(device, shaderTableSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_HEAP_TYPE_DEFAULT);
	tempShaderTable = new Buffer(device, shaderTableSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);
	// Map the buffer
	uint8_t* pData;
	hr = tempShaderTable->buffer->Map(0, nullptr, (void**)&pData);
	if (FAILED(hr))
		return false;

	MAKE_SMART_COM_PTR(ID3D12StateObjectProperties);
	ID3D12StateObjectPropertiesPtr pRtsoProps;
	pipelineState->pipelineState->QueryInterface(IID_PPV_ARGS(&pRtsoProps));

	uint8_t* pEntry, *pDesc;
	int curindex = 0;
	int cur_RS_index = 0;

	// Entry 0 - ray-gen program ID and descriptor data
	pEntry = pData + shaderTableEntrySize * (curindex++);
	memcpy(pEntry, pRtsoProps->GetShaderIdentifier(rayGenName), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
	pDesc = pEntry + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	for (int j = 0; j < rayGenData_count; ++j)
	{
		assert(((uint64_t)pDesc % 8) == 0);
		*(UINT64*)(pDesc) = rayGenData[j];
		D3D12_ROOT_SIGNATURE_DESC cur_RS = pipelineState->shaderRS_Pairs[cur_RS_index].rootSignature;
		if (cur_RS.pParameters[j].ParameterType == D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)
			pDesc += 4;
		else
			pDesc += 8;
	}
	if(rayGenData_count > 0)
		cur_RS_index++;

	// Miss Shaders
	missIndex = curindex;
	for (int i = 0; i < acs_top->numberOfRays; ++i)
	{
		pEntry = pData + shaderTableEntrySize * (curindex++);
		LPCWSTR aaa = missShaders[i];
		memcpy(pEntry, pRtsoProps->GetShaderIdentifier(missShaders[i]), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		pDesc = pEntry + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
		for (int j = 0; j < missShaders_data_count[i]; ++j)
		{
			assert(((uint64_t)pDesc % 8) == 0);
			*(UINT64*)(pDesc) = missShaders_data[i][j];
			D3D12_ROOT_SIGNATURE_DESC cur_RS = pipelineState->shaderRS_Pairs[cur_RS_index].rootSignature;
			if (cur_RS.pParameters[j].ParameterType == D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)
				pDesc += 4;
			else
				pDesc += 8;
		}
		if (missShaders_data_count[i] > 0)
			cur_RS_index++;
	}

	// Hits
	hitIndex = curindex;
	int geomcount = 0;
	for (int i = 0; i < acs_top->ACS_OBJ_buffers.size(); ++i)
	{
		for (int k = 0; k < acs_top->ACS_OBJ_buffers[i]->ACS_BOT->geomDesc.size(); ++k)
		{
			for (int l = 0; l < acs_top->numberOfRays; ++l)
			{
				pEntry = pData + shaderTableEntrySize * (curindex++);
				LPCWSTR curHitGroup = geomHitGroup[geomcount * acs_top->numberOfRays + l];
				memcpy(pEntry, pRtsoProps->GetShaderIdentifier(curHitGroup), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
				pDesc = pEntry + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
				int curdatacount = 0;
				int curshaderpairindex = 0;
				for (int j = 0; j < pipelineState->shaderRS_Pairs.size(); ++j)
				{
					if (pipelineState->shaderRS_Pairs[j].hitGroup != nullptr && std::wstring(curHitGroup) == std::wstring(pipelineState->shaderRS_Pairs[j].hitGroup))
					{
						curdatacount = pipelineState->shaderRS_Pairs[j].rootSignature.NumParameters;
						curshaderpairindex = j;
					}
				}
				for (int j = 0; j < curdatacount; ++j)
				{
					assert(((uint64_t)pDesc % 8) == 0);
					*(UINT64*)(pDesc) = geomData[geomcount * acs_top->numberOfRays + l][j];
					D3D12_ROOT_SIGNATURE_DESC cur_RS = pipelineState->shaderRS_Pairs[curshaderpairindex].rootSignature;
					if (cur_RS.pParameters[j].ParameterType == D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)
						pDesc += 4;
					else
						pDesc += 8;
				}
			}
			geomcount++;
		}  
	}
	// Unmap
	tempShaderTable->buffer->Unmap(0, nullptr);
	device->commandList->commandList->CopyResource(shaderTable->buffer, tempShaderTable->buffer);
	device->commandList->CommandListFinished->subscribe(L"UploadingFinished", [&]() {UploadingFinished(); });

	return true;
}

void DXR_ShaderTable::UploadingFinished()
{
	delete tempShaderTable;
	tempShaderTable = nullptr;
	device->commandList->CommandListFinished->unsubscribe(L"UploadingFinished");
}

bool DXR_ShaderTable::SetGeomData(DXR_ACS_OBJ* geom, int rayindex, UINT64* data, LPCWSTR hitGroup)
{
	geomData[geom->ACS_TOP_index * acs_top->numberOfRays + rayindex] = data;
	geomHitGroup[geom->ACS_TOP_index * acs_top->numberOfRays + rayindex] = hitGroup;
	return true;
}

D3D12_DISPATCH_RAYS_DESC DXR_ShaderTable::CreateRayDesc()
{
	return CreateRayDesc(device->bufferWidth, device->bufferHeight);
}

D3D12_DISPATCH_RAYS_DESC DXR_ShaderTable::CreateRayDesc(int Width, int Height)
{
	D3D12_DISPATCH_RAYS_DESC raytraceDesc = {};
	raytraceDesc.Width = Width;
	raytraceDesc.Height = Height;
	raytraceDesc.Depth = 1;

	// RayGen is the first entry in the shader-table
	raytraceDesc.RayGenerationShaderRecord.StartAddress = shaderTable->buffer->GetGPUVirtualAddress() + 0 * shaderTableEntrySize;
	raytraceDesc.RayGenerationShaderRecord.SizeInBytes = shaderTableEntrySize;

	// Miss is the second entry in the shader-table
	size_t missOffset = missIndex * shaderTableEntrySize;
	raytraceDesc.MissShaderTable.StartAddress = shaderTable->buffer->GetGPUVirtualAddress() + missOffset;
	raytraceDesc.MissShaderTable.StrideInBytes = shaderTableEntrySize;
	raytraceDesc.MissShaderTable.SizeInBytes = shaderTableEntrySize * (hitIndex - missIndex);

	// Hit is the third entry in the shader-table
	size_t hitOffset = (hitIndex) * shaderTableEntrySize;
	raytraceDesc.HitGroupTable.StartAddress = shaderTable->buffer->GetGPUVirtualAddress() + hitOffset;
	raytraceDesc.HitGroupTable.StrideInBytes = shaderTableEntrySize;
	raytraceDesc.HitGroupTable.SizeInBytes = shaderTableEntrySize * (entryNumber - hitIndex);
	return raytraceDesc;
}
