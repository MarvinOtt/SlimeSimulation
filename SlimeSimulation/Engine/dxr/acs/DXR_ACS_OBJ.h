#pragma once
#include <d3d12.h>
#include "../../../include/glm/glm.hpp"

using namespace glm;

class DXR_ACS_BOT;
class GraphicsDevice;

#include "DXR_ACS_BOT.h"
#include "../../buffer/VertexBuffer.h"
#include "../../buffer/IndexBuffer.h"
#include "../../buffer/Buffer.h"

class DXR_ACS_OBJ
{
public:
	DXR_ACS_BOT* ACS_BOT;
	Buffer* transBuf = nullptr;
	mat4 trans;
	int ACS_TOP_index = -1;

private:
	HRESULT hr;
	bool acs_bot_created = false;

public:
	template <typename T>
	DXR_ACS_OBJ(GraphicsDevice* graphicsDevice, VertexBuffer<T>* vertexBuffer, mat4 trans, bool createTransBuf = false);
	template <typename T, typename U>
	DXR_ACS_OBJ(GraphicsDevice* graphicsDevice, VertexBuffer<T>* vertexBuffer, IndexBuffer<U>* indexBuffer, mat4 trans, bool createTransBuf = false);
	DXR_ACS_OBJ(GraphicsDevice* graphicsDevice, DXR_ACS_BOT* acs_bot, mat4 trans, bool createTransBuf = false);
	~DXR_ACS_OBJ();
	void ChangeTrans(mat4 newtrans);
};

template <typename T>
DXR_ACS_OBJ::DXR_ACS_OBJ(GraphicsDevice* graphicsDevice, VertexBuffer<T>* vertexBuffer, mat4 trans, bool createTransBuf)
{
	acs_bot_created = true;
	ACS_BOT = new DXR_ACS_BOT(graphicsDevice, vertexBuffer);
	this->trans = trans;
	if (createTransBuf)
	{
		transBuf = new Buffer(graphicsDevice, sizeof(mat4), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);
		transBuf->SetData(&trans);
	}
}

template <typename T, typename U>
DXR_ACS_OBJ::DXR_ACS_OBJ(GraphicsDevice* graphicsDevice, VertexBuffer<T>* vertexBuffer, IndexBuffer<U>* indexBuffer, mat4 trans, bool createTransBuf)
{
	acs_bot_created = true;
	ACS_BOT = new DXR_ACS_BOT(graphicsDevice, vertexBuffer, indexBuffer);
	this->trans = trans;
	if (createTransBuf)
	{
		transBuf = new Buffer(graphicsDevice, sizeof(mat4), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);
		transBuf->SetData(&trans);
	}
}

