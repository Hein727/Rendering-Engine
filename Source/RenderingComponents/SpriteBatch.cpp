#include "SpriteBatch.h"
#include "../System/Misc.h"
#include "Texture.h"
#include "Shader.h"
#include <sstream>
#include <WICTextureLoader.h>

// Rotate around center
inline auto rotate(float& x, float& y, float cx, float cy, float angle)
{
	x -= cx;
	y -= cy;

	float cos{ cosf(DirectX::XMConvertToRadians(angle)) };
	float sin{ sinf(DirectX::XMConvertToRadians(angle)) };
	float tx{ x }, ty{ y };
	x = cos * tx + -sin * ty;
	y = sin * tx + cos * ty;

	x += cx;
	y += cy;
}

SpriteBatch::SpriteBatch(GameContext& gameContext, const wchar_t* filename, size_t max_sprites) : maxVertices(max_sprites * 6), gameContext(gameContext)
{
	auto device = gameContext.graphics.GetDevice();

	HRESULT hr{ S_OK };

	// Create vertex buffer
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(Vertex) * maxVertices;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	hr = device->CreateBuffer(&bufferDesc, NULL, vertexBuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	// Load and create vertex shader
	const char* cso_name{ "Shader\\Sprite_vs.cso" };

	D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	hr = createVsFromCso(device, cso_name, vertexShader.GetAddressOf(), inputLayout.GetAddressOf(), layoutDesc, _countof(layoutDesc));
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));	

	// Load and create pixel shader
	cso_name = { "Shader\\Sprite_ps.cso" };
	
	hr = createPsFromCso(device, cso_name, pixelShader.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	//Create Texture and Shader Resource View

	loadTextureFromFile(device, filename, shader_resource_view.GetAddressOf(), &texture2d_desc);
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	//////////////////////////////////////////////////////////////////////////////////////
}

//Call this before Render()

void SpriteBatch::Begin()
{
	vertices.clear();
	auto context = gameContext.graphics.GetDeviceContext();

	context->VSSetShader(vertexShader.Get(), nullptr, 0);	
	context->PSSetShader(pixelShader.Get(), nullptr, 0);
	context->PSSetShaderResources(0, 1, shader_resource_view.GetAddressOf());
}

/*
	dx : destination x (top-left)
	dy : destination y (top-left)
	dw : destination width
	dh : destination height
	r, g, b, a : color modulation (0.0 ~ 1.0)
	angle : rotation angle (degree)
	sx : source x (top-left)
	sy : source y (top-left)
	sw : source width
	sh : source height
	source means where to draw from the texture
*/

void SpriteBatch::Render(float dx, float dy, float dw, float dh, float r, float g, float b, float a, float angle, float sx, float sy, float sw, float sh)
{
	auto context = gameContext.graphics.GetDeviceContext();

	D3D11_VIEWPORT viewport{};
	UINT num_viewports{ 1 };
	context->RSGetViewports(&num_viewports, &viewport);

	//top left
	float x0{ dx };
	float y0{ dy };

	//top right
	float x1{ dx + dw };
	float y1{ dy };

	//bottom left
	float x2{ dx };
	float y2{ dy + dh };

	//bottom right
	float x3{ dx + dw };
	float y3{ dy + dh };

	float cx = dx + dw * 0.5f;
	float cy = dy + dh * 0.5f;
	rotate(x0, y0, cx, cy, angle);
	rotate(x1, y1, cx, cy, angle);
	rotate(x2, y2, cx, cy, angle);
	rotate(x3, y3, cx, cy, angle);

	// Convert to NDC

	x0 = 2.0f * (x0 / viewport.Width) - 1.0f;
	y0 = 1.0f - 2.0f * (y0 / viewport.Height);
	x1 = 2.0f * (x1 / viewport.Width) - 1.0f;
	y1 = 1.0f - 2.0f * (y1 / viewport.Height);
	x2 = 2.0f * (x2 / viewport.Width) - 1.0f;
	y2 = 1.0f - 2.0f * (y2 / viewport.Height);
	x3 = 2.0f * (x3 / viewport.Width) - 1.0f;
	y3 = 1.0f - 2.0f * (y3 / viewport.Height);

	// Texture coordinates
	float u0{ sx / static_cast<float>(texture2d_desc.Width) };
	float v0{ sy / static_cast<float>(texture2d_desc.Height) };
	float u1{ (sx + sw) / static_cast<float>(texture2d_desc.Width) };
	float v1{ (sy + sh) / static_cast<float>(texture2d_desc.Height) };

	// Update vertex buffer

	vertices.push_back({ { x0, y0, 0,}, { r, g, b, a }, { u0, v0} });
	vertices.push_back({ { x1, y1, 0,}, { r, g, b, a }, { u1, v0} });
	vertices.push_back({ { x2, y2, 0,}, { r, g, b, a }, { u0, v1} });
	vertices.push_back({ { x2, y2, 0,}, { r, g, b, a }, { u0, v1} });
	vertices.push_back({ { x1, y1, 0,}, { r, g, b, a }, { u1, v0} });
	vertices.push_back({ { x3, y3, 0,}, { r, g, b, a }, { u1, v1} });
}

//Call this after Begin() and Render()

void SpriteBatch::End()
{
	// Update vertex buffer
	auto context = gameContext.graphics.GetDeviceContext();
	HRESULT hr{ S_OK };
	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	hr = context->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	size_t vertexCount{ vertices.size() };
	_ASSERT_EXPR(maxVertices >= vertexCount, "exceeded max sprite count");

	// Check for empty vertex list
	Vertex* data{ reinterpret_cast<Vertex*>(mappedResource.pData) };
	if (data != nullptr)
	{
		const Vertex* p = vertices.data();
		memcpy_s(data, maxVertices * sizeof(Vertex), p, vertexCount * sizeof(Vertex));
	}
	context->Unmap(vertexBuffer.Get(), 0);

	// Set vertex buffer

	UINT stride{ sizeof(Vertex) };
	UINT offset{ 0 };

	context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	
	context->IASetInputLayout(inputLayout.Get());

	context->Draw(static_cast<UINT>(vertexCount), 0);
}