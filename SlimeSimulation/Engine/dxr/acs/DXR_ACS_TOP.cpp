#include "DXR_ACS_TOP.h"
#include "../../GraphicsDevice.h"
#include "../../buffer/Buffer.h"
#include "../../command/CommandQueue.h"
#include "../../Fence.h"
#include "../../SwapChain.h"
#include "../../command/CommandAllocator.h"
#include "DXR_ACS_OBJ.h"

DXR_ACS_TOP::DXR_ACS_TOP(int numberOfRays2)
{
	numberOfRays = numberOfRays2;
	HasBeenBuilt = false;
}

DXR_ACS_TOP::~DXR_ACS_TOP()
{
	if (HasBeenBuilt)
	{
		delete pScratch;
		delete pResult;
		delete pInstanceDesc;
	}
}

bool DXR_ACS_TOP::Add_ACS_OBJ(DXR_ACS_OBJ* acs)
{
	acs->ACS_TOP_index = (int)ACS_OBJ_buffers.size();
	ACS_OBJ_buffers.push_back(acs);

	return true;
}

bool DXR_ACS_TOP::Add_ACS_OBJ_multiple(DXR_ACS_OBJ** acs, int size)
{
	for (int i = 0; i < size; ++i)
	{
		acs[i]->ACS_TOP_index = (int)ACS_OBJ_buffers.size() + i;
	}
	int oldsize = (int)ACS_OBJ_buffers.size();
	ACS_OBJ_buffers.resize(oldsize + size, 0);
	memcpy(&ACS_OBJ_buffers[oldsize], acs, size * sizeof(DXR_ACS_OBJ*));

	return true;
}

bool DXR_ACS_TOP::Build(GraphicsDevice* device)
{
	if (HasBeenBuilt)
	{
		delete pScratch;
		delete pResult;
		delete pInstanceDesc;
	}

	// First, get the size of the TLAS buffers and create them
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
	inputs.NumDescs = (int)ACS_OBJ_buffers.size();
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};

	device->device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

	// Create the buffers
	pScratch = new Buffer(device, info.ScratchDataSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_HEAP_TYPE_DEFAULT);
	pResult = new Buffer(device, info.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, D3D12_HEAP_TYPE_DEFAULT);

	// The instance desc should be inside a buffer, create and map the buffer
	pInstanceDesc = new Buffer(device, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * inputs.NumDescs, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RAYTRACING_INSTANCE_DESC* pInstanceDescRaw;
	pInstanceDesc->buffer->Map(0, nullptr, (void**)&pInstanceDescRaw);
	ZeroMemory(pInstanceDescRaw, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * inputs.NumDescs);
	int length = (int)ACS_OBJ_buffers.size();
	int curindex = 0;
	for (int i = 0; i < length; ++i)
	{
		pInstanceDescRaw[i].InstanceID = i; // This value will be exposed to the shader via InstanceID()
		pInstanceDescRaw[i].InstanceContributionToHitGroupIndex = curindex; // This is the offset inside the shader-table. We only have a single geometry, so the offset 0
		pInstanceDescRaw[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
		mat4 m = transpose(ACS_OBJ_buffers[i]->trans); // GLM is column major, the INSTANCE_DESC is row major
		memcpy(pInstanceDescRaw[i].Transform, &m, sizeof(pInstanceDescRaw[i].Transform));
		pInstanceDescRaw[i].AccelerationStructure = ACS_OBJ_buffers[i]->ACS_BOT->ACS_buffer->GetGPUVirtualAddress();
		pInstanceDescRaw[i].InstanceMask = 0xFF;
		curindex += numberOfRays * (int)ACS_OBJ_buffers[i]->ACS_BOT->geomDesc.size();
	}

	// Unmap
	pInstanceDesc->buffer->Unmap(0, nullptr);

	// Create the TLAS
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
	asDesc.Inputs = inputs;
	asDesc.Inputs.InstanceDescs = pInstanceDesc->buffer->GetGPUVirtualAddress();
	asDesc.DestAccelerationStructureData = pResult->buffer->GetGPUVirtualAddress();
	asDesc.ScratchAccelerationStructureData = pScratch->buffer->GetGPUVirtualAddress();

	device->commandList->commandList->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

	// We need to insert a UAV barrier before using the acceleration structures in a raytracing operation
	D3D12_RESOURCE_BARRIER uavBarrier = {};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = pResult->buffer;
	device->commandList->commandList->ResourceBarrier(1, &uavBarrier);

	srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.RaytracingAccelerationStructure.Location = pResult->buffer->GetGPUVirtualAddress();

	HasBeenBuilt = true;

	return true;
}

bool DXR_ACS_TOP::Update(GraphicsDevice* device)
{
	if (!HasBeenBuilt)
		return false;

	// First, get the size of the TLAS buffers and create them
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
	inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	inputs.NumDescs = (int)ACS_OBJ_buffers.size();
	inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};

	device->device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

	// If this a request for an update, then the TLAS was already used in a DispatchRay() call. We need a UAV barrier to make sure the read operation ends before updating the buffer
	D3D12_RESOURCE_BARRIER uavBarrier = {};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = pResult->buffer;
	device->commandList->commandList->ResourceBarrier(1, &uavBarrier);

	D3D12_RAYTRACING_INSTANCE_DESC* pInstanceDescRaw;
	pInstanceDesc->buffer->Map(0, nullptr, (void**)&pInstanceDescRaw);
	ZeroMemory(pInstanceDescRaw, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * inputs.NumDescs);
	int length = (int)ACS_OBJ_buffers.size();
	for (int i = 0; i < length; ++i)
	{
		pInstanceDescRaw[i].InstanceID = i; // This value will be exposed to the shader via InstanceID()
		pInstanceDescRaw[i].InstanceContributionToHitGroupIndex = i * numberOfRays; // This is the offset inside the shader-table. We only have a single geometry, so the offset 0
		pInstanceDescRaw[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
		mat4 m = transpose(ACS_OBJ_buffers[i]->trans); // GLM is column major, the INSTANCE_DESC is row major
		memcpy(pInstanceDescRaw[i].Transform, &m, sizeof(pInstanceDescRaw[i].Transform));
		pInstanceDescRaw[i].AccelerationStructure = ACS_OBJ_buffers[i]->ACS_BOT->ACS_buffer->GetGPUVirtualAddress();
		pInstanceDescRaw[i].InstanceMask = 0xFF;
	}

	// Unmap
	pInstanceDesc->buffer->Unmap(0, nullptr);

	// Create the TLAS
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
	asDesc.Inputs = inputs;
	asDesc.Inputs.InstanceDescs = pInstanceDesc->buffer->GetGPUVirtualAddress();
	asDesc.DestAccelerationStructureData = pResult->buffer->GetGPUVirtualAddress();
	asDesc.ScratchAccelerationStructureData = pScratch->buffer->GetGPUVirtualAddress();
	asDesc.Inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
	asDesc.SourceAccelerationStructureData = pResult->buffer->GetGPUVirtualAddress();

	device->commandList->commandList->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);

	// We need to insert a UAV barrier before using the acceleration structures in a raytracing operation
	uavBarrier = {};
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	uavBarrier.UAV.pResource = pResult->buffer;
	device->commandList->commandList->ResourceBarrier(1, &uavBarrier);

	return true;
}

