#include "LineRenderer.h"
#include "Shader.h"
#include <cmath>

LineRenderer::LineRenderer(GameContext& gameContext) : gameContext(gameContext)
{
	D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	createVsFromCso(gameContext.graphics.GetDevice(), "Shader\\LineRenderer_vs.cso", vertex_shader.ReleaseAndGetAddressOf(), input_layout.ReleaseAndGetAddressOf(), input_element_desc, ARRAYSIZE(input_element_desc));
	createPsFromCso(gameContext.graphics.GetDevice(), "Shader\\LineRenderer_ps.cso", pixel_shader.ReleaseAndGetAddressOf());

	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(Vertex) * VertexCapacity;
	buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;
	HRESULT hr = gameContext.graphics.GetDevice()->CreateBuffer(&buffer_desc, nullptr, vertext_buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

}

void LineRenderer::AddVertex(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT4& color)
{
	Vertex& v = vertices.emplace_back();
	v.position = position;
	v.color = color;
}

void LineRenderer::DrawGrid(int subdivisions, float scale)
{
	DirectX::XMFLOAT3 oldCamPos = gameContext.input.cameraControls.get_position();

	currentCellOrigin = DirectX::XMFLOAT3(static_cast<int>(std::floor(oldCamPos.x / scale) * scale), 0, static_cast<int>(std::floor(oldCamPos.z / scale) * scale));
	DirectX::XMVECTOR cellOriginV = DirectX::XMLoadFloat3(&currentCellOrigin);

	if(prevCellOrigin.x == currentCellOrigin.x && prevCellOrigin.z == currentCellOrigin.z)
	{
		return;
	}

	int numLines = (subdivisions + 1) * 2;
	int vertexCount = numLines * 2;

	float corner = 0.5f;
	float step = 1.0f / static_cast<float>(subdivisions);

	int index = 0;
	float s = -corner;

	const DirectX::XMFLOAT4 white = DirectX::XMFLOAT4(1, 1, 1, 1);

	float scaling = static_cast<float>(subdivisions) * scale;
	DirectX::XMMATRIX M = DirectX::XMMatrixScaling(scaling, scaling, scaling);
	DirectX::XMVECTOR V, P;
	DirectX::XMFLOAT3 position;
	for (int i = 0; i <= subdivisions; i++)
	{
		V = DirectX::XMVectorSet(s, 0, corner, 0);
		P = DirectX::XMVectorAdd(DirectX::XMVector3TransformCoord(V, M), cellOriginV);
		DirectX::XMStoreFloat3(&position, P);
		AddVertex(position, white);

		V = DirectX::XMVectorSet(s, 0, -corner, 0);
		P = DirectX::XMVectorAdd(DirectX::XMVector3TransformCoord(V, M), cellOriginV);
		DirectX::XMStoreFloat3(&position, P);
		AddVertex(position, white);

		s += step;
	}

	s = -corner;
	for (int i = 0; i <= subdivisions; i++)
	{
		V = DirectX::XMVectorSet(corner, 0, s, 0);
		P = DirectX::XMVectorAdd(DirectX::XMVector3TransformCoord(V, M), cellOriginV);
		DirectX::XMStoreFloat3(&position, P);
		AddVertex(position, white);

		V = DirectX::XMVectorSet(-corner, 0, s, 0);
		P = DirectX::XMVectorAdd(DirectX::XMVector3TransformCoord(V, M), cellOriginV);
		DirectX::XMStoreFloat3(&position, P);
		AddVertex(position, white);

		s += step;
	}

	gridCenterPoints.clear();
	for (int z = 0; z < subdivisions; ++z)
	{
		for (int x = 0; x < subdivisions; ++x)
		{
			float centerX = currentCellOrigin.x + x * scale + scale * 0.5f;
			float centerZ = currentCellOrigin.z + z * scale + scale * 0.5f;

			gridCenterPoints.emplace_back(centerX, 0.0f, centerZ);
		}
	}

}

void LineRenderer::Render()
{
	const auto& dc = gameContext.graphics.GetDeviceContext();

	dc->VSSetShader(vertex_shader.Get(), nullptr, 0);
	dc->PSSetShader(pixel_shader.Get(), nullptr, 0);
	dc->IASetInputLayout(input_layout.Get());

	UINT strite = sizeof(Vertex);	
	UINT offset = 0;
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	dc->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	dc->IASetVertexBuffers(0, 1, vertext_buffer.GetAddressOf(), &strite, &offset);

	UINT totalVertexCount = static_cast<UINT>(vertices.size());
	UINT start = 0;
	UINT count = (totalVertexCount > VertexCapacity) ? VertexCapacity : totalVertexCount;	

	while (start < totalVertexCount)
	{
		D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
		HRESULT hr = dc->Map(vertext_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
		_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

		memcpy(mappedSubresource.pData, &vertices[start], sizeof(Vertex) * count);

		dc->Unmap(vertext_buffer.Get(), 0);

		dc->Draw(count, 0);

		start += count;
		if ((start + count) > totalVertexCount)
		{
			count = totalVertexCount - start;
		}
	}

	vertices.clear();
}