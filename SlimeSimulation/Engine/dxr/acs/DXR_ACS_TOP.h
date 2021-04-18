#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
#include <vector>
#include "../../../include/glm/glm.hpp"
//#include "DXR_ACS_OBJ.h"

using namespace std; 
using namespace glm;

class GraphicsDevice;
class Buffer;
class DXR_ACS_OBJ;

class DXR_ACS_TOP
{
public:
	vector<DXR_ACS_OBJ*> ACS_OBJ_buffers;
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	bool HasBeenBuilt;
	int numberOfRays;

	Buffer* pResult;
	Buffer* pInstanceDesc;

private:
	HRESULT hr;
	Buffer* pScratch;

public:
	DXR_ACS_TOP(int numberOfRays2);
	~DXR_ACS_TOP();
	bool Add_ACS_OBJ(DXR_ACS_OBJ* acs);
	bool Add_ACS_OBJ_multiple(DXR_ACS_OBJ** acs, int size);
	bool Build(GraphicsDevice*);
	bool Update(GraphicsDevice*);
};

