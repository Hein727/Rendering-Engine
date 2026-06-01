#pragma once
#include <Windows.h> // BYTE ‚Ì‚½‚ß’Ç‰Á
#include <WICTextureLoader.h>
using namespace DirectX;

#include <wrl.h>
using namespace Microsoft::WRL;

#include <string>
#include <map>
using namespace std;

static map<wstring, ComPtr<ID3D11ShaderResourceView>> textureCache;

HRESULT loadTextureFromFile(ID3D11Device* device, const wchar_t* filename,
	ID3D11ShaderResourceView** shader_resource_view, D3D11_TEXTURE2D_DESC* texture2d_desc);

HRESULT makeDummyTexture(ID3D11Device* device, ID3D11ShaderResourceView** shader_resource_view, DWORD value/*0xAABBGGRR*/, UINT dimension); 

HRESULT loadTextureFromMemory(ID3D11Device* device, const void* data, size_t size, ID3D11ShaderResourceView** srv);

void releaseTextureCache();