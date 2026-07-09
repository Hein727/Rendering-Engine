#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl.h>
#include "../System/GameContext.h"	

class Geometric_Primitive
{
public:
	struct vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
	};

	struct constants
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4 material_color;
	};

private:

	Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffer;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;

	GameContext& gameContext;

public:

	Geometric_Primitive(GameContext& gameContext);
	~Geometric_Primitive() = default;

	void render(const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& material_color = {1, 1, 1, 1});

protected:

	void createComBuffers(vertex* vertices, size_t vertex_count, uint32_t* indices, size_t index_count);
};