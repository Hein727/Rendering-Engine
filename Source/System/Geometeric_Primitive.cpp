#include "Geometric_Primitive.h"
#include "Misc.h"
#include "Texture.h"
#include "Shader.h"

Geometric_Primitive::Geometric_Primitive(GameContext& gameContext) : gameContext(gameContext)
{
	auto device = gameContext.graphics.GetDevice();

	vertex vertices[24]{};

	uint32_t indices[36]{};

	uint32_t face = 0;
	vertices[face * 4 + 0].position = { -0.5f, +0.5f, +0.5f };
	vertices[face * 4 + 1].position = { +0.5f, +0.5f, +0.5f };
	vertices[face * 4 + 2].position = { -0.5f, +0.5f, -0.5f };
	vertices[face * 4 + 3].position = { +0.5f, +0.5f, -0.5f };
	vertices[face * 4 + 0].normal = { +0.0f, +1.0f, +0.0f };
	vertices[face * 4 + 1].normal = { +0.0f, +1.0f, +0.0f };
	vertices[face * 4 + 2].normal = { +0.0f, +1.0f, +0.0f };
	vertices[face * 4 + 3].normal = { +0.0f, +1.0f, +0.0f };
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 1;
	indices[face * 6 + 2] = face * 4 + 2;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 3;
	indices[face * 6 + 5] = face * 4 + 2;

	face += 1;
	vertices[face * 4 + 0].position = { -0.5f, -0.5f, +0.5f };
	vertices[face * 4 + 1].position = { +0.5f, -0.5f, +0.5f };
	vertices[face * 4 + 2].position = { -0.5f, -0.5f, -0.5f };
	vertices[face * 4 + 3].position = { +0.5f, -0.5f, -0.5f };
	vertices[face * 4 + 0].normal = { +0.0f, -1.0f, +0.0f };
	vertices[face * 4 + 1].normal = { +0.0f, -1.0f, +0.0f };
	vertices[face * 4 + 2].normal = { +0.0f, -1.0f, +0.0f };
	vertices[face * 4 + 3].normal = { +0.0f, -1.0f, +0.0f };
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 2;
	indices[face * 6 + 2] = face * 4 + 1;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 2;
	indices[face * 6 + 5] = face * 4 + 3;

	face += 1;
	vertices[face * 4 + 0].position = { -0.5f, +0.5f, -0.5f };
	vertices[face * 4 + 1].position = { +0.5f, +0.5f, -0.5f };
	vertices[face * 4 + 2].position = { -0.5f, -0.5f, -0.5f };
	vertices[face * 4 + 3].position = { +0.5f, -0.5f, -0.5f };
	vertices[face * 4 + 0].normal = { +0.0f, +0.0f, -1.0f };
	vertices[face * 4 + 1].normal = { +0.0f, +0.0f, -1.0f };
	vertices[face * 4 + 2].normal = { +0.0f, +0.0f, -1.0f };
	vertices[face * 4 + 3].normal = { +0.0f, +0.0f, -1.0f };
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 1;
	indices[face * 6 + 2] = face * 4 + 2;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 3;
	indices[face * 6 + 5] = face * 4 + 2;

	face += 1;
	vertices[face * 4 + 0].position = { -0.5f, +0.5f, +0.5f };
	vertices[face * 4 + 1].position = { +0.5f, +0.5f, +0.5f };
	vertices[face * 4 + 2].position = { -0.5f, -0.5f, +0.5f };
	vertices[face * 4 + 3].position = { +0.5f, -0.5f, +0.5f };
	vertices[face * 4 + 0].normal = { +0.0f, +0.0f, +1.0f };
	vertices[face * 4 + 1].normal = { +0.0f, +0.0f, +1.0f };
	vertices[face * 4 + 2].normal = { +0.0f, +0.0f, +1.0f };
	vertices[face * 4 + 3].normal = { +0.0f, +0.0f, +1.0f };
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 2;
	indices[face * 6 + 2] = face * 4 + 1;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 2;
	indices[face * 6 + 5] = face * 4 + 3;

	face += 1;
	vertices[face * 4 + 0].position = { +0.5f, +0.5f, -0.5f };
	vertices[face * 4 + 1].position = { +0.5f, +0.5f, +0.5f };
	vertices[face * 4 + 2].position = { +0.5f, -0.5f, -0.5f };
	vertices[face * 4 + 3].position = { +0.5f, -0.5f, +0.5f };
	vertices[face * 4 + 0].normal = { +1.0f, +0.0f, +0.0f };
	vertices[face * 4 + 1].normal = { +1.0f, +0.0f, +0.0f };
	vertices[face * 4 + 2].normal = { +1.0f, +0.0f, +0.0f };
	vertices[face * 4 + 3].normal = { +1.0f, +0.0f, +0.0f };
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 1;
	indices[face * 6 + 2] = face * 4 + 2;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 3;
	indices[face * 6 + 5] = face * 4 + 2;

	// 左面
	face += 1;
	vertices[face * 4 + 0].position = { -0.5f, +0.5f, -0.5f };
	vertices[face * 4 + 1].position = { -0.5f, +0.5f, +0.5f };
	vertices[face * 4 + 2].position = { -0.5f, -0.5f, -0.5f };
	vertices[face * 4 + 3].position = { -0.5f, -0.5f, +0.5f };
	vertices[face * 4 + 0].normal = { -1.0f, +0.0f, +0.0f };
	vertices[face * 4 + 1].normal = { -1.0f, +0.0f, +0.0f };
	vertices[face * 4 + 2].normal = { -1.0f, +0.0f, +0.0f };
	vertices[face * 4 + 3].normal = { -1.0f, +0.0f, +0.0f };
	indices[face * 6 + 0] = face * 4 + 0;
	indices[face * 6 + 1] = face * 4 + 2;
	indices[face * 6 + 2] = face * 4 + 1;
	indices[face * 6 + 3] = face * 4 + 1;
	indices[face * 6 + 4] = face * 4 + 2;
	indices[face * 6 + 5] = face * 4 + 3;

	// バッファを作成
	createComBuffers(vertices, 24, indices, 36);

	// create shaders
	HRESULT hr{ S_OK };

	// create input element desc for vertex shader
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	hr = createVsFromCso(device, "Shader\\Geometric_Primitive_vs.cso",
		vertex_shader.GetAddressOf(), input_layout.GetAddressOf(), inputElementDesc, _countof(inputElementDesc));
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	hr = createPsFromCso(device, "Shader\\Geometric_Primitive_ps.cso",
		pixel_shader.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	// create constant buffer
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(constants);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hr = device->CreateBuffer(&buffer_desc, nullptr, constant_buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	////////////// create buffers here //////////////
	

}

// this is only for vertex and index buffer creation
// don't make other buffers here 
void Geometric_Primitive::createComBuffers(vertex* vertices, size_t vertex_count, uint32_t* indices, size_t index_count)
{
	auto device = gameContext.graphics.GetDevice();

	HRESULT hr{ S_OK };

	// create vertex buffer	
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = static_cast<UINT>(sizeof(vertex) * vertex_count);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA subresource_data{};
	subresource_data.pSysMem = vertices;
	subresource_data.SysMemPitch = 0;
	subresource_data.SysMemSlicePitch = 0;
	hr = device->CreateBuffer(&buffer_desc, &subresource_data, vertex_buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	// create index buffer
	buffer_desc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * index_count);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	subresource_data.pSysMem = indices;
	hr = device->CreateBuffer(&buffer_desc, &subresource_data, index_buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));
}

void Geometric_Primitive::render(const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& material_color)
{
	auto context = gameContext.graphics.GetDeviceContext();

	// set vertex and index buffer
	// set the primitive topology
	// set the input layout
	uint32_t stride{ sizeof(vertex) };
	uint32_t offset{ 0 };
	context->IASetVertexBuffers(0, 1, vertex_buffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(input_layout.Get());

	// set the shaders up
	context->VSSetShader(vertex_shader.Get(), nullptr, 0);
	context->PSSetShader(pixel_shader.Get(), nullptr, 0);

	// update constant buffer	
	constants constants_data{};
	constants_data.world = world;	
	constants_data.material_color = material_color;
	context->UpdateSubresource(constant_buffer.Get(), 0, nullptr, &constants_data, 0, 0);
	context->VSSetConstantBuffers(0, 1, constant_buffer.GetAddressOf());

	// set index buffer
	D3D11_BUFFER_DESC buffer_desc{};
	index_buffer->GetDesc(&buffer_desc);

	// draw call
	context->DrawIndexed(buffer_desc.ByteWidth / sizeof(uint32_t), 0, 0);
}