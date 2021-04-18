#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>
class GraphicsDevice;

class DXR_ACS
{
public:


private:
    HRESULT hr;

public:
    DXR_ACS();
    bool Create_BAS(GraphicsDevice*);
    bool Create_TAS(GraphicsDevice*);
};

