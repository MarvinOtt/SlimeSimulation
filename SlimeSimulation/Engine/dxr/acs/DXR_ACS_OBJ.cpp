#include "DXR_ACS_OBJ.h"

DXR_ACS_OBJ::DXR_ACS_OBJ(GraphicsDevice* graphicsDevice, DXR_ACS_BOT* acs_bot, mat4 trans, bool createTransBuf)
{
	ACS_BOT = acs_bot;
	this->trans = trans;
	if (createTransBuf)
	{
		transBuf = new Buffer(graphicsDevice, sizeof(mat4), D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD);
		transBuf->SetData(&trans);
	}
}

void DXR_ACS_OBJ::ChangeTrans(mat4 newtrans)
{
	trans = newtrans;
	transBuf->SetData(&trans);
}

DXR_ACS_OBJ::~DXR_ACS_OBJ()
{
	if (transBuf != nullptr)
		delete transBuf;
	if (acs_bot_created)
		delete ACS_BOT;
}
