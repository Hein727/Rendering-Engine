#include "Skinned_Mesh.h"
#include "Misc.h"
#include "Graphics.h"
#include "Shader.h"
#include <functional>
#include <sstream>

Skinned_Mesh::Skinned_Mesh(const char* fbx_filename, bool triangulate)
{
	// Calling in device from graphics
	auto device = graphics::getInstance().GetDevice();
	
	// Create FbxManager 
	FbxManager* fbx_manager{ FbxManager::Create()};

	// Create FbxScene
	FbxScene* fbx_scene{ FbxScene::Create(fbx_manager, "") };

	// Create Importer
	FbxImporter* fbx_importer{ FbxImporter::Create(fbx_manager, "") };
	bool import_status{ false };
	import_status = fbx_importer->Initialize(fbx_filename);
	_ASSERT_EXPR(import_status, fbx_importer->GetStatus().GetErrorString());

	// Import to scene
	import_status = fbx_importer->Import(fbx_scene);
	_ASSERT_EXPR(import_status, fbx_importer->GetStatus().GetErrorString());

	// Triangulate (if needed)
	FbxGeometryConverter fbx_converter(fbx_manager);
	if (triangulate)
	{
		fbx_converter.Triangulate(fbx_scene, true/*replace*/, false/*legacy*/);
		fbx_converter.RemoveBadPolygonsFromMeshes(fbx_scene);
	}

	// Recursive function to find node datas
	std::function<void(FbxNode*)> traverse{ [&](FbxNode* fbx_node) {
		scene::node& node{ scene_view.nodes.emplace_back()};
		node.attribute = fbx_node->GetNodeAttribute() ?
			fbx_node->GetNodeAttribute()->GetAttributeType() : FbxNodeAttribute::EType::eUnknown;
		node.name = fbx_node->GetName();
		node.unique_id = fbx_node->GetUniqueID();
		node.parent_index = scene_view.indexof(fbx_node->GetParent() ? fbx_node->GetParent()->GetUniqueID() : 0);
		for (int child_index = 0; child_index < fbx_node->GetChildCount(); child_index++)
		{
			traverse(fbx_node->GetChild(child_index));
		}
	} };

	// Getting the root node 
	traverse(fbx_scene->GetRootNode());

	Fetch_meshes(fbx_scene, meshes);

	// Destroy the FbxManager after use
	fbx_manager->Destroy();

	Create_com_object(fbx_filename);
}

void Skinned_Mesh::Fetch_meshes(FbxScene* fbx_scene, std::vector<Mesh>& meshes)
{
	for (const auto& node : scene_view.nodes)
	{
		if (node.attribute != FbxNodeAttribute::eMesh)
		{
			continue;
		}

		FbxNode* fbx_node{ fbx_scene->FindNodeByName(node.name.c_str()) };
		FbxMesh* fbx_mesh{ fbx_node->GetMesh() };

		Mesh& mesh{ meshes.emplace_back() };
		mesh.unique_id = fbx_node->GetUniqueID();
		mesh.name = fbx_node->GetName();
		mesh.node_index = scene_view.indexof(mesh.unique_id);

		const int polygon_count{ fbx_mesh->GetPolygonCount() };
		mesh.vertices.resize(polygon_count * 3LL);
		mesh.indices.resize(polygon_count * 3LL);
		
		FbxStringList uv_names;
		fbx_mesh->GetUVSetNames(uv_names);
		const FbxVector4* control_points{ fbx_mesh->GetControlPoints() };
		for (int polygon_index = 0; polygon_index < polygon_count; ++polygon_index)
		{
			for (int position_in_polygon = 0; position_in_polygon < 3; ++position_in_polygon)
			{
				const int vertex_index{ polygon_index * 3 + position_in_polygon };

				Vertex vertex;
				const int polygon_vertex{ fbx_mesh->GetPolygonVertex(polygon_index, position_in_polygon) };
				vertex.position.x = static_cast<float>(control_points[polygon_vertex][0]);
				vertex.position.y = static_cast<float>(control_points[polygon_vertex][1]);
				vertex.position.z = static_cast<float>(control_points[polygon_vertex][2]);

				if (fbx_mesh->GetElementNormalCount() > 0)
				{
					FbxVector4 normal;
					fbx_mesh->GetPolygonVertexNormal(polygon_index, position_in_polygon, normal);
					vertex.normal.x = static_cast<float>(normal[0]);
					vertex.normal.y = static_cast<float>(normal[1]);
					vertex.normal.z = static_cast<float>(normal[2]);
				}

				if (fbx_mesh->GetElementUVCount() > 0)
				{
					FbxVector2 uv;
					bool unmapped_uv;
					fbx_mesh->GetPolygonVertexUV(polygon_index, position_in_polygon, uv_names[0], uv, unmapped_uv);
					vertex.texcoord.x = static_cast<float>(uv[0]);
					vertex.texcoord.y = 1.0f - static_cast<float>(uv[1]);
				}

				mesh.vertices.at(vertex_index) = std::move(vertex);
				mesh.indices.at(vertex_index) = vertex_index;
			}
		}
	}
}

void Skinned_Mesh::Create_com_object(const char* fbx_filename)
{
	auto device = graphics::getInstance().GetDevice();
	for (auto& mesh : meshes)
	{
		HRESULT hr{ S_OK };
		D3D11_BUFFER_DESC buffer_desc{};
		D3D11_SUBRESOURCE_DATA subresource_data{};
		buffer_desc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * mesh.vertices.size());
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffer_desc.CPUAccessFlags = 0;
		buffer_desc.MiscFlags = 0;
		buffer_desc.StructureByteStride = 0;
		subresource_data.pSysMem = mesh.vertices.data();
		subresource_data.SysMemPitch = 0;
		subresource_data.SysMemSlicePitch = 0;
		hr = device->CreateBuffer(&buffer_desc, &subresource_data, mesh.vertex_buffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

		buffer_desc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * mesh.indices.size());
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		subresource_data.pSysMem = mesh.indices.data();
		hr = device->CreateBuffer(&buffer_desc, &subresource_data, mesh.index_buffer.GetAddressOf());
#if 1
		mesh.vertices.clear();
		mesh.indices.clear();
#endif
	}
	
	HRESULT hr{ S_OK };
	D3D11_INPUT_ELEMENT_DESC input_element_desc[]
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT},
	};

	createVsFromCso(device, "Shader\\Skinned_Mesh_vs.cso", vertexShader.ReleaseAndGetAddressOf(), inputLayout.ReleaseAndGetAddressOf(), input_element_desc, ARRAYSIZE(input_element_desc));
	createPsFromCso(device, "Shader\\Skinned_Mesh_ps.cso", pixelShader.ReleaseAndGetAddressOf());

	D3D11_BUFFER_DESC buffer_desc{};

	buffer_desc.ByteWidth = sizeof(Constants);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hr = device->CreateBuffer(&buffer_desc, nullptr, constantBuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

}

void Skinned_Mesh::Render(const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& material_color)
{
	auto context = graphics::getInstance().GetDeviceContext();
	for (const auto& mesh : meshes)
	{
		uint32_t strides{ sizeof(Vertex) };
		uint32_t offset{ 0 };

		context->IASetVertexBuffers(0, 1, mesh.vertex_buffer.GetAddressOf(), &strides, &offset);
		context->IASetIndexBuffer(mesh.index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(inputLayout.Get());

		context->VSSetShader(vertexShader.Get(), nullptr, 0);
		context->PSSetShader(pixelShader.Get(), nullptr, 0);

		Constants data;
		data.world = world;
		data.material_color = material_color;
		context->UpdateSubresource(constantBuffer.Get(), 0, 0, &data, 0, 0);
		context->VSSetConstantBuffers(1, 1, constantBuffer.GetAddressOf());

		D3D11_BUFFER_DESC buffer_desc{};
		mesh.index_buffer->GetDesc(&buffer_desc);
		context->DrawIndexed(buffer_desc.ByteWidth / sizeof(uint32_t), 0, 0);
	}
}