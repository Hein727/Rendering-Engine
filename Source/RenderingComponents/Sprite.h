#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <string>
#include "../System/GameContext.h"	

class Sprite
{
private:

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;	
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view;
	D3D11_TEXTURE2D_DESC texture2d_desc;

	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT2 texcoord;
	};

	GameContext& gameContext;

public :
	
	Sprite(GameContext& gameContext, const wchar_t* filename);
	~Sprite() = default;

	void Render(float dx, float dy, float dw, float dh, float r, float g, float b, float a, float angle,
		float sx, float sy, float sw, float sh);
	void Render(float dx, float dy, float dw, float dh, float r = 1, float g = 1, float b = 1, float a = 1, float angle = 0.0f)
	{
		float sx = 0.0f;
		float sy = 0.0f;
		float sw = static_cast<float>(texture2d_desc.Width);
		float sh = static_cast<float>(texture2d_desc.Height);
		Render(dx, dy, dw, dh, r, g, b, a, angle, sx, sy, sw, sh);
	}
	void textout(std::string s, float x, float y, float w, float h, float r, float g, float b, float a)
	{
		float sw = static_cast<float>(texture2d_desc.Width / 16);
		float sh = static_cast<float>(texture2d_desc.Height / 16);
		float carriage = 0;
		for (const char c : s)
		{
			Render(x + carriage, y, w, h, r, g, b, a, 0,
				sw * (c & 0x0F), sh * (c >> 4), sw, sh);
			carriage += w;
		}
	}
};