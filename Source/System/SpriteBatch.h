#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <vector>

class SpriteBatch
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

	const size_t maxVertices;
	std::vector<Vertex> vertices;

public:

	SpriteBatch(const wchar_t* filename, size_t max_sprites);
	~SpriteBatch() = default;

	void Begin();
	void End();

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

};