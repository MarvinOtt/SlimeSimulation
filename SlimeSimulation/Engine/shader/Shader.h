#pragma once
#include <d3d12.h>
#include <dxgi1_4.h>

class Shader
{
public:
	ID3DBlob* shaderBlob;

private:
	HRESULT hr;
	ID3DBlob* errorBuff;

public:
	Shader();
	bool Compile(char* data, int length, LPCSTR entryPoint, LPCSTR shaderType);
	bool Load(LPCWSTR filename, LPCSTR entryPoint, LPCSTR shaderType);
	D3D12_SHADER_BYTECODE GetByteCode();
};

