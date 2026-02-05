#include "Sprite.h"
#include "Misc.h"
#include "Graphics.h"
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

Sprite::Sprite(const wchar_t* filename)
{
	auto device = graphics::getInstance().GetDevice();

	HRESULT hr{ S_OK };

	Vertex vertices[]
	{
		{ { -0.5, +0.5, 0 }, { 1, 1, 1, 1 } },
		{ { +0.5, +0.5, 0 }, { 1, 0, 0, 1 } },
		{ { -0.5, -0.5, 0 }, { 0, 1, 0, 1 } },
		{ { +0.5, -0.5, 0 }, { 0, 0, 1, 1 } },
	};

	// Create vertex buffer
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = sizeof(vertices);
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;
	hr = device->CreateBuffer(&bufferDesc, &initData, vertexBuffer.GetAddressOf());
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

	/////////////////////////Create Sampler State////////////////////////////////////

	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = device->CreateSamplerState(&samplerDesc, samplerStates[SS_POINT].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	hr = device->CreateSamplerState(&samplerDesc, samplerStates[SS_LINEAR].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	hr = device->CreateSamplerState(&samplerDesc, samplerStates[SS_ANISOTROPIC].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	//////////////////////////////////////////////////////////////////////////////////////
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
*/

void Sprite::Render(float dx, float dy, float dw, float dh, float r, float g, float b, float a, float angle, float sx, float sy, float sw, float sh)
{
	auto context = graphics::getInstance().GetDeviceContext();

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

	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	HRESULT hr{ context->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource) };
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));
	
	Vertex* vertices{ reinterpret_cast<Vertex*>(mappedResource.pData) };
	if (vertices != nullptr)
	{
		vertices[0].position = { x0, y0, 0.0f };
		vertices[1].position = { x1, y1, 0.0f };
		vertices[2].position = { x2, y2, 0.0f };
		vertices[3].position = { x3, y3, 0.0f };
		vertices[0].color = vertices[1].color = vertices[2].color = vertices[3].color = { r, g, b, a };

		vertices[0].texcoord = { u0, v0 };
		vertices[1].texcoord = { u1, v0 };
		vertices[2].texcoord = { u0, v1 };
		vertices[3].texcoord = { u1, v1 };
        context->PSSetSamplers(0, 1, &samplerStates[SS_POINT]);
        context->PSSetSamplers(1, 1, &samplerStates[SS_LINEAR]);
        context->PSSetSamplers(2, 1, &samplerStates[SS_ANISOTROPIC]);
	}

	context->PSSetShaderResources(0, 1, shader_resource_view.GetAddressOf());

	context->Unmap(vertexBuffer.Get(), 0);

	UINT stride{ sizeof(Vertex) };
	UINT offset{ 0 };
	context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetInputLayout(inputLayout.Get());
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context->VSSetShader(vertexShader.Get(), nullptr, 0);
	context->PSSetShader(pixelShader.Get(), nullptr, 0);

	context->Draw(4, 0);
}