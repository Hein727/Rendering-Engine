#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl.h>
#include "GameContext.h"
#include <vector>

class LineRenderer
{
public:
	LineRenderer(GameContext& gameContext);
	~LineRenderer() = default;

	void AddVertex(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT4& color);

	void DrawGrid(int subdivisions, float scale);

	void Render();

private:

	static const UINT VertexCapacity = 3 * 1024;

	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};
	std::vector<Vertex> vertices;

	Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertext_buffer;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;

	GameContext& gameContext;

	// Useful for snapping things into place on the grid
	std::vector<DirectX::XMFLOAT3> gridCenterPoints;

	DirectX::XMFLOAT3 prevCellOrigin{ 0, 0, 0 };
	DirectX::XMFLOAT3 currentCellOrigin;
};