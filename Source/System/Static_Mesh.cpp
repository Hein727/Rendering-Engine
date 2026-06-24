#include "Static_Mesh.h"
#include "Misc.h"
#include "Texture.h"
#include "Shader.h"
#include <vector>
#include <fstream>
#include <filesystem>

using namespace DirectX;

StaticMesh::StaticMesh(GameContext& gameContext, const wchar_t* obj_filename) : gameContext(gameContext)
{
	auto device = gameContext.graphics.GetDevice();

	// variables to hold vertex and index positions and normals
	std::vector<vertex> vertices;
	std::vector<uint32_t> indices;
	uint32_t current_index{ 0 };

	std::vector<XMFLOAT3> positions;
	std::vector<XMFLOAT3> normals;
	std::vector<XMFLOAT2> texcoords;
	std::vector<wstring> mtl_filenames;

	// load the obj file
	std::wifstream file(obj_filename);	
	_ASSERT_EXPR(file, L"Failed to load .obj file");
	wchar_t command[256];
	while (file)
	{
		file >> command;
		if (0 == wcscmp(command, L"v"))
		{
			float x, y, z;
			file >> x >> y >> z;
			positions.push_back({ x, y, z });
			file.ignore(1024, L'\n');
		}
		else if (0 == wcscmp(command, L"vn"))
		{
			float i, j, k;
			file >> i >> j >> k;
			normals.push_back({ i, j, k });
			file.ignore(1024, L'\n');
		}
		else if(0 == wcscmp(command, L"vt"))
		{
			float u, v;
			file >> u >> v;
			texcoords.push_back({ u, 1.0f - v });
			file.ignore(1024, L'\n');
		}
		else if (0 == wcscmp(command, L"f"))
		{
			for (int i = 0; i < 3; ++i)
			{
				vertex vertex;
				size_t v, vt, vn;

				file >> v;
				vertex.position = positions.at(v - 1);
				if (L'/' == file.peek())
				{
					file.ignore(1);
					if(L'/' != file.peek())
					{
						file >> vt;
						vertex.texcoord = texcoords.at(vt - 1);
					}
					if (L'/' == file.peek())
					{
						file.ignore(1);
						file >> vn;
						vertex.normal = normals.at(vn - 1);
					}
				}
				vertices.push_back(vertex);
				indices.push_back(current_index++);
			}
			file.ignore(1024, L'\n');
		}
		else if (0 == wcscmp(command, L"mtllib"))
		{
			wchar_t mtllib[256];
			file >> mtllib;
			mtl_filenames.push_back(mtllib);
		}
		else if (0 == wcscmp(command, L"usemtl"))
		{
			wchar_t usemtl[MAX_PATH]{ 0 };
			file >> usemtl;
			subsets.push_back({ usemtl, static_cast<uint32_t>(indices.size())});
		}
		else
		{
			file.ignore(1024, L'\n');
		}
	}
	file.close();


	// if there is no material info in the obj file, create a default one
	if (materials.size() == 0)
	{
		for (const subset& subset : subsets)
		{
			materials.push_back({ subset.usemtl });
		}
	}

	// calculate index counts for each subset
	std::vector<subset>::reverse_iterator iterator = subsets.rbegin();
	iterator->index_count = static_cast<uint32_t>(indices.size()) - iterator->index_start;
	for(iterator = subsets.rbegin() + 1; iterator != subsets.rend(); ++iterator)
	{
		iterator->index_count = (iterator - 1)->index_start - iterator->index_start;
	}

	createComBuffers(vertices.data(), vertices.size(), indices.data(), indices.size());

	// loading the texture from mtl file	
	std::filesystem::path mtl_filename(obj_filename);	
	mtl_filename.replace_filename(std::filesystem::path(mtl_filenames[0]).filename());

	file.open(mtl_filename);

	while (file)
	{
		file >> command;
		if (0 == wcscmp(command, L"map_Kd"))
		{
			file.ignore();
			wchar_t map_kd[256];	
			file >> map_kd;

			std::filesystem::path path(obj_filename);
			path.replace_filename(std::filesystem::path(map_kd).filename());
			materials.rbegin()->texture_filenames[0] = path;
			file.ignore(1024, L'\n');
		}
		else if(0 == wcscmp(command, L"map_bump") || 0 == wcscmp(command, L"bump"))
		{
			file.ignore();
			wchar_t map_bump[256];
			file >> map_bump;
			std::filesystem::path path(obj_filename);
			path.replace_filename(std::filesystem::path(map_bump).filename());
			materials.rbegin()->texture_filenames[1] = path;
			file.ignore(1024, L'\n');
		}
		else if (0 == wcscmp(command, L"newmtl"))
		{
			file.ignore();
			wchar_t newmtl[256];
			material material;
			file >> newmtl;
			material.name = newmtl;
			materials.push_back(material);
		}
		else if(0 == wcscmp(command, L"Kd"))
		{
			float r, g, b;
			file >> r >> g >> b;
			materials.rbegin()->Kd = { r, g, b, 1.0f };
			file.ignore(1024, L'\n');
		}
		else
		{
			file.ignore(1024, L'\n');
		}
	}
	file.close();

	D3D11_TEXTURE2D_DESC texture2d_desc{};

	// loading textures 
	for (auto& material : materials)
	{
		// checking if texture file name is empty or not
		// messy but works keep it 
		if (material.texture_filenames[0].empty())
		{
			makeDummyTexture(device, material.shader_resource_views[0].GetAddressOf(), 0xFFFFFFFF, 16);
		}
		else
		{
			loadTextureFromFile(device, material.texture_filenames[0].c_str(), material.shader_resource_views[0].GetAddressOf(), &texture2d_desc);
		}
		if (material.texture_filenames[1].empty())
		{
			makeDummyTexture(device, material.shader_resource_views[1].GetAddressOf(), 0xFFFF7F7F, 16);
		}
		else
		{
			loadTextureFromFile(device, material.texture_filenames[1].c_str(), material.shader_resource_views[1].GetAddressOf(), &texture2d_desc);
		}
	}

	// create shaders
	HRESULT hr{ S_OK };

	// create input element desc for vertex shader
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	hr = createVsFromCso(device, "Shader\\Static_Mesh_vs.cso",
		vertex_shader.GetAddressOf(), input_layout.GetAddressOf(), inputElementDesc, _countof(inputElementDesc));
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	hr = createPsFromCso(device, "Shader\\Static_Mesh_ps.cso",
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



	/////////////////////////////////////////////////

	////////// calculate bounding box //////////////
	for (auto& vertex : vertices)
	{
		bbox.min.x = bbox.min.x < vertex.position.x ? bbox.min.x : vertex.position.x;
		bbox.min.y = bbox.min.y < vertex.position.y ? bbox.min.y : vertex.position.y;
		bbox.min.z = bbox.min.z < vertex.position.z ? bbox.min.z : vertex.position.z;

		bbox.max.x = bbox.max.x > vertex.position.x ? bbox.max.x : vertex.position.x;
		bbox.max.y = bbox.max.y > vertex.position.y ? bbox.max.y : vertex.position.y;
		bbox.max.z = bbox.max.z > vertex.position.z ? bbox.max.z : vertex.position.z;
	}
}

// this is only for vertex and index buffer creation
// don't make other buffers here 
void StaticMesh::createComBuffers(vertex* vertices, size_t vertex_count, uint32_t* indices, size_t index_count)
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

void StaticMesh::render(const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& material_color)
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


	for (auto& material : materials)
	{
		context->PSSetShaderResources(0, 1, material.shader_resource_views[0].GetAddressOf());
		context->PSSetShaderResources(1, 1, material.shader_resource_views[1].GetAddressOf());

		constants data{ world, material_color };
		XMStoreFloat4(&data.material_color, XMLoadFloat4(&material_color) * XMLoadFloat4(&material.Kd));
		context->UpdateSubresource(constant_buffer.Get(), 0, nullptr, &data, 0, 0);
		context->VSSetConstantBuffers(1, 1, constant_buffer.GetAddressOf());

		for (auto& subset : subsets)
		{
			if (material.name == subset.usemtl)
			{
				context->DrawIndexed(subset.index_count, subset.index_start, 0);
			}
		}
	}
}