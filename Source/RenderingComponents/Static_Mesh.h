#pragma once
#include <DirectXMath.h>
#include <wrl.h>
#include <d3d11.h>
#include <string>
#include <vector>
#include <memory>
#include "../System/GameContext.h"	

class StaticMesh
{
public:
	struct vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 texcoord;
	};

	struct constants
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4 material_color;
	};

	struct subset
	{
		std::wstring usemtl;
		uint32_t index_start{ 0 };
		uint32_t index_count{ 0 };
	};
	std::vector<subset> subsets;

	struct boundingBox
	{
		DirectX::XMFLOAT3 min{ -FLT_EPSILON, -FLT_EPSILON, -FLT_EPSILON };
		DirectX::XMFLOAT3 max{ FLT_EPSILON, FLT_EPSILON, FLT_EPSILON };
	};
	boundingBox bbox;

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffer;
	
	struct material
	{
		std::wstring name;
		DirectX::XMFLOAT4 Ka{ 0.2f, 0.2f, 0.2f, 1.0f };
		DirectX::XMFLOAT4 Kd{ 0.8f, 0.8f, 0.8f, 1.0f };
		DirectX::XMFLOAT4 Ks{ 1.0f, 1.0f, 1.0f, 1.0f };

		std::wstring texture_filenames[2];
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_views[2];
	};
	std::vector<material> materials;

public :
	StaticMesh(GameContext& gameContext, const wchar_t* obj_filename);
	virtual ~StaticMesh() = default;

	void render(const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& material_color);

protected :
	void createComBuffers(vertex* vertices, size_t vertex_count, uint32_t* indices, size_t index_count);

	GameContext& gameContext;
};
