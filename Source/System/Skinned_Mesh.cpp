#include "Skinned_Mesh.h"
#include "Misc.h"
#include "Graphics.h"
#include "Shader.h"
#include "Texture.h"
#include <filesystem>
#include <functional>
#include <sstream>

inline DirectX::XMFLOAT4X4 to_xmfloat4x4(const FbxAMatrix matrix)
{
	DirectX::XMFLOAT4X4 result;
	for(int i = 0; i < 4; ++i)
	{
		for(int j = 0; j < 4; ++j)
		{
			result.m[i][j] = static_cast<float>(matrix.Get(i, j));
		}
	}
	return result;
}

inline DirectX::XMFLOAT3 to_xmfloat3(const FbxVector4 vector)
{
	DirectX::XMFLOAT3 result;
	result.x = static_cast<float>(vector[0]);
	result.y = static_cast<float>(vector[1]);
	result.z = static_cast<float>(vector[2]);
	return result;
}

inline DirectX::XMFLOAT4 to_xmfloat4(const FbxVector4 vector)
{
	DirectX::XMFLOAT4 result;
	result.x = static_cast<float>(vector[0]);
	result.y = static_cast<float>(vector[1]);
	result.z = static_cast<float>(vector[2]);
	result.w = static_cast<float>(vector[3]);
	return result;
}

void Fetch_Bone_Influences(const FbxMesh* fbx_mesh, std::vector<Bone_Influences_Per_Control_Point>& bone_influences)
{
	const int control_points_count{ fbx_mesh->GetControlPointsCount() };
	bone_influences.resize(control_points_count);

	const int skin_count{ fbx_mesh->GetDeformerCount(FbxDeformer::eSkin) };
	for (int skin_index = 0; skin_index < skin_count; ++skin_index)
	{
		const FbxSkin* fbx_skin{ static_cast<FbxSkin*>(fbx_mesh->GetDeformer(skin_index, FbxDeformer::eSkin)) };

		const int cluster_count{ fbx_skin->GetClusterCount() };
		for (int cluster_index = 0; cluster_index < cluster_count; ++cluster_index)
		{
			const FbxCluster* fbx_cluster{ fbx_skin->GetCluster(cluster_index) };

			const int control_point_indices_count{ fbx_cluster->GetControlPointIndicesCount() };
			for (int control_point_indices_index = 0; control_point_indices_index < control_point_indices_count; ++control_point_indices_index)
			{
				int control_point_index{ fbx_cluster->GetControlPointIndices()[control_point_indices_index] };
				double control_point_weight{ fbx_cluster->GetControlPointWeights()[control_point_indices_index] };

				Bone_Influence& bone_influence{ bone_influences.at(control_point_index).emplace_back() };
				bone_influence.bone_index = static_cast<uint32_t>(cluster_index);
				bone_influence.bone_weight = static_cast<float>(control_point_weight);
			}
		}
	}
}

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

	// Get meshes
	Fetch_meshes(fbx_scene, meshes);

	// Get materials
	Fetch_materials(fbx_scene, materials);

	// Get animations
	Fetch_Animation(fbx_scene, animations);

	// Destroy the FbxManager after use
	fbx_manager->Destroy();

	// Create COM objects
	Create_com_object(fbx_filename);
}

void Skinned_Mesh::Fetch_meshes(FbxScene* fbx_scene, std::vector<Mesh>& meshes)
{

	// Iterate through nodes in scene_view to find meshes
	for (const auto& node : scene_view.nodes)
	{
		if (node.attribute != FbxNodeAttribute::eMesh)
		{
			continue;
		}

		// Get FbxNode and FbxMesh
		FbxNode* fbx_node{ fbx_scene->FindNodeByName(node.name.c_str()) };
		FbxMesh* fbx_mesh{ fbx_node->GetMesh() };

		// Create Mesh
		Mesh& mesh{ meshes.emplace_back() };
		mesh.unique_id = fbx_node->GetUniqueID();
		mesh.name = fbx_node->GetName();
		mesh.node_index = scene_view.indexof(mesh.unique_id);
		mesh.default_gobal_transform = to_xmfloat4x4(fbx_node->EvaluateGlobalTransform());

		// Get bone and skeleton 
		std::vector<Bone_Influences_Per_Control_Point>  bone_influences;
		Fetch_Bone_Influences(fbx_mesh, bone_influences);
		Fetch_skeleton(fbx_mesh, mesh.bind_pose);

		std::vector<Mesh::Subsets>& subsets{ mesh.subsets };
		const int material_count{ fbx_mesh->GetNode()->GetMaterialCount() };
		subsets.resize(material_count > 0 ? material_count : 1);
		for (int material_index = 0; material_index < material_count; ++material_index)
		{
			const FbxSurfaceMaterial* fbx_material{ fbx_mesh->GetNode()->GetMaterial(material_index) };
			subsets.at(material_index).material_name = fbx_material->GetName();
			subsets.at(material_index).material_unique_id = fbx_material->GetUniqueID();
		}

		if (material_count > 0)
		{
			const int polygon_count{ fbx_mesh->GetPolygonCount() };
			for (int polygon_index = 0; polygon_index < polygon_count; ++polygon_index)
			{
				const int material_index
				{
					fbx_mesh->GetElementMaterial()->GetIndexArray().GetAt(polygon_index)
				};

				subsets.at(material_index).index_count += 3; // Each polygon is a triangle (3 indices)
			}
			uint32_t offset{ 0 };
			for (auto& subset : subsets)
			{
				subset.index_start = offset;
				offset += subset.index_count;
				// This will be used in Render function to set correct material
				subset.index_count = 0;
			}
		}
			

		// Fetch vertices and indices
		const int polygon_count{ fbx_mesh->GetPolygonCount() };
		mesh.vertices.resize(polygon_count * 3LL);
		mesh.indices.resize(polygon_count * 3LL);
		
		// Get UV set names
		FbxStringList uv_names;
		fbx_mesh->GetUVSetNames(uv_names);
		const FbxVector4* control_points{ fbx_mesh->GetControlPoints() };
		for (int polygon_index = 0; polygon_index < polygon_count; ++polygon_index)
		{
			const int material_index{ material_count > 0 ? fbx_mesh->GetElementMaterial()->GetIndexArray().GetAt(polygon_index) : 0 };
			Mesh::Subsets& subset{ subsets.at(material_index) };
			const uint32_t offset{ subset.index_start + subset.index_count };

			for (int position_in_polygon = 0; position_in_polygon < 3; ++position_in_polygon)
			{
				// Calculate vertex index
				// Each polygon is a triangle (3 vertices)
				const int vertex_index{ polygon_index * 3 + position_in_polygon };

				// Fill Vertex Data
				// Position
				Vertex vertex;
				const int polygon_vertex{ fbx_mesh->GetPolygonVertex(polygon_index, position_in_polygon) };
				vertex.position.x = static_cast<float>(control_points[polygon_vertex][0]);
				vertex.position.y = static_cast<float>(control_points[polygon_vertex][1]);
				vertex.position.z = static_cast<float>(control_points[polygon_vertex][2]);

				const Bone_Influences_Per_Control_Point& influences_per_contorl_point{ bone_influences.at(polygon_vertex) };
				for (size_t influence_index = 0; influence_index < influences_per_contorl_point.size(); ++influence_index)
				{
					if (influence_index < MAX_BONE_INFLUENCE)
					{
						vertex.bone_weights[influence_index] = influences_per_contorl_point.at(influence_index).bone_weight;
						vertex.bone_indices[influence_index] = influences_per_contorl_point.at(influence_index).bone_index;
					}
				}

				if (fbx_mesh->GetElementNormalCount() > 0)
				{
					// Normal
					FbxVector4 normal;
					fbx_mesh->GetPolygonVertexNormal(polygon_index, position_in_polygon, normal);
					vertex.normal.x = static_cast<float>(normal[0]);
					vertex.normal.y = static_cast<float>(normal[1]);
					vertex.normal.z = static_cast<float>(normal[2]);
				}

				if (fbx_mesh->GetElementUVCount() > 0)
				{
					// Texcoord
					FbxVector2 uv;
					bool unmapped_uv;
					fbx_mesh->GetPolygonVertexUV(polygon_index, position_in_polygon, uv_names[0], uv, unmapped_uv);
					vertex.texcoord.x = static_cast<float>(uv[0]);
					vertex.texcoord.y = 1.0f - static_cast<float>(uv[1]);
				}

				// Assign vertex to mesh
				mesh.vertices.at(vertex_index) = std::move(vertex);

				// Fill index data
				mesh.indices.at(static_cast<size_t>(offset) + position_in_polygon) = vertex_index;
				subset.index_count++;	
			}
		}
	}
}

void Skinned_Mesh::Create_com_object(const char* fbx_filename)
{
	auto device = graphics::getInstance().GetDevice();

	// Create vertex buffer and index buffer for each mesh
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
	
	// Create shaders, input layout, constant buffer
	HRESULT hr{ S_OK };
	D3D11_INPUT_ELEMENT_DESC input_element_desc[]
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT},
		{"WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT},
		{"BONES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT},
	};

	createVsFromCso(device, "Shader\\Skinned_Mesh_vs.cso", vertexShader.ReleaseAndGetAddressOf(), inputLayout.ReleaseAndGetAddressOf(), input_element_desc, ARRAYSIZE(input_element_desc));
	createPsFromCso(device, "Shader\\Skinned_Mesh_ps.cso", pixelShader.ReleaseAndGetAddressOf());

	D3D11_BUFFER_DESC buffer_desc{};

	buffer_desc.ByteWidth = sizeof(Constants);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hr = device->CreateBuffer(&buffer_desc, nullptr, constantBuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	// Create shader resource views for materials
	for (std::unordered_map<uint64_t, Material>::iterator iterator = materials.begin(); iterator != materials.end(); ++iterator)
	{
		if (iterator->second.texture_filenames[0].size() > 0)
		{
			std::filesystem::path path(fbx_filename);
			path.replace_filename(iterator->second.texture_filenames[0]);
			D3D11_TEXTURE2D_DESC texture2d_desc{};
			loadTextureFromFile(device, path.c_str(), iterator->second.srvs[0].GetAddressOf(), &texture2d_desc);
		}
		else
		{
			makeDummyTexture(device, iterator->second.srvs[0].GetAddressOf(), 0xFFFFFFFF, 16);
		}
	}

}

void Skinned_Mesh::Render(const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& material_color, const Animation::Keyframe* keyframe)
{
	auto context = graphics::getInstance().GetDeviceContext();
	for (const auto& mesh : meshes)
	{
		// Set IA, VS, PS stages
		uint32_t strides{ sizeof(Vertex) };
		uint32_t offset{ 0 };

		context->IASetVertexBuffers(0, 1, mesh.vertex_buffer.GetAddressOf(), &strides, &offset);
		context->IASetIndexBuffer(mesh.index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(inputLayout.Get());

		context->VSSetShader(vertexShader.Get(), nullptr, 0);
		context->PSSetShader(pixelShader.Get(), nullptr, 0);

		// Update constant buffer
		Constants data;

		// Get the keyframe node data the global transform for animation that have displacement in them 
		const Animation::Keyframe::Node& mesh_node{ keyframe->nodes.at(mesh.node_index) };
		XMStoreFloat4x4(&data.world, XMLoadFloat4x4(&mesh_node.global_transform) * XMLoadFloat4x4(&world));

		const size_t bone_count{ mesh.bind_pose.bones.size() };
		for (int bone_index = 0; bone_index < bone_count; ++bone_index)
		{
			const Skeleton::Bone& bone{ mesh.bind_pose.bones.at(bone_index) };
			const Animation::Keyframe::Node& bone_node{ keyframe->nodes.at(bone.node_index) };
			XMStoreFloat4x4(&data.bone_transforms[bone_index],
				XMLoadFloat4x4(&bone.offset_transform) *
				XMLoadFloat4x4(&bone_node.global_transform) *
				XMMatrixInverse(nullptr, XMLoadFloat4x4(&mesh_node.global_transform)) //use the mesh_node's so that you can animate the displaced nodes
			);
		}

		for (const auto& subset : mesh.subsets)
		{
			const Material& material{ materials.at(subset.material_unique_id) };

			DirectX::XMStoreFloat4(&data.material_color, DirectX::XMLoadFloat4(&material_color) * DirectX::XMLoadFloat4(&material.kd));
			context->UpdateSubresource(constantBuffer.Get(), 0, 0, &data, 0, 0);
			context->VSSetConstantBuffers(1, 1, constantBuffer.GetAddressOf());

			context->PSSetShaderResources(0, 1, material.srvs[0].GetAddressOf());

			context->DrawIndexed(subset.index_count, subset.index_start, 0);
		}
	}
}

void Skinned_Mesh::Fetch_materials(FbxScene* fbx_scene, std::unordered_map<uint64_t, Material>& materials)
{
	// Iterate through nodes in scene_view to find materials
	const size_t node_count{ scene_view.nodes.size() };

	for (size_t node_index = 0; node_index < node_count; ++node_index)
	{
		// Get FbxNode
		const scene::node& node{ scene_view.nodes.at(node_index) };
		const FbxNode* fbx_node{ fbx_scene->FindNodeByName(node.name.c_str()) };	

		// Get materials from the node	
		const int material_count{ fbx_node->GetMaterialCount() };
		for (int material_index = 0; material_index < material_count; ++material_index)
		{
			const FbxSurfaceMaterial* fbx_material{ fbx_node->GetMaterial(material_index) };

			Material material{};
			material.name = fbx_material->GetName();
			material.unique_id = fbx_material->GetUniqueID();

			FbxProperty fbx_property{};
			fbx_property = fbx_material->FindProperty(FbxSurfaceMaterial::sDiffuse);
			if(fbx_property.IsValid())
			{
				FbxDouble3 fbx_color{ fbx_property.Get<FbxDouble3>() };
				material.kd.x = static_cast<float>(fbx_color[0]);
				material.kd.y = static_cast<float>(fbx_color[1]);
				material.kd.z = static_cast<float>(fbx_color[2]);

				const FbxFileTexture* fbx_texture{ fbx_property.GetSrcObject<FbxFileTexture>() };	
				material.texture_filenames[0] = fbx_texture ? fbx_texture->GetRelativeFileName() : "";
			}

			fbx_property = fbx_material->FindProperty(FbxSurfaceMaterial::sAmbient);
			if (fbx_property.IsValid())
			{
				FbxDouble3 fbx_color{ fbx_property.Get<FbxDouble3>() };
				material.ka.x = static_cast<float>(fbx_color[0]);
				material.ka.y = static_cast<float>(fbx_color[1]);
				material.ka.z = static_cast<float>(fbx_color[2]);

				const FbxFileTexture* fbx_texture{ fbx_property.GetSrcObject<FbxFileTexture>() };
				material.texture_filenames[1] = fbx_texture ? fbx_texture->GetRelativeFileName() : "";
			}

			fbx_property = fbx_material->FindProperty(FbxSurfaceMaterial::sSpecular);
			if(fbx_property.IsValid())
			{
				FbxDouble3 fbx_color{ fbx_property.Get<FbxDouble3>() };
				material.ks.x = static_cast<float>(fbx_color[0]);
				material.ks.y = static_cast<float>(fbx_color[1]);
				material.ks.z = static_cast<float>(fbx_color[2]);
				const FbxFileTexture* fbx_texture{ fbx_property.GetSrcObject<FbxFileTexture>() };
				material.texture_filenames[2] = fbx_texture ? fbx_texture->GetRelativeFileName() : "";
			}

			materials.emplace(material.unique_id, std::move(material));
		}
	}

	// If no material found, create a default one
	// This is to avoid accessing invalid iterator in Render function
	materials.emplace(0, Material{});
}

void Skinned_Mesh::Fetch_skeleton(FbxMesh* fbx_mesh, Skeleton& bind_pose)
{
	const int deformer_count = fbx_mesh->GetDeformerCount(FbxDeformer::eSkin);
	for (int deformer_index = 0; deformer_index < deformer_count; ++deformer_index)
	{
		FbxSkin* fbx_skin = static_cast<FbxSkin*>(fbx_mesh->GetDeformer(deformer_index, FbxDeformer::eSkin));
		const int cluster_count = fbx_skin->GetClusterCount();
		bind_pose.bones.resize(cluster_count);

		for (int cluster_index = 0; cluster_index < cluster_count; ++cluster_index)
		{
			FbxCluster* cluster = fbx_skin->GetCluster(cluster_index);

			auto& bone{ bind_pose.bones.at(cluster_index) };
			bone.name = cluster->GetLink()->GetName();
			bone.unique_id = cluster->GetLink()->GetUniqueID();
			bone.parent_index = bind_pose.indexof(cluster->GetLink()->GetParent()->GetUniqueID());
			bone.node_index = scene_view.indexof(bone.unique_id);

			//'reference_global_init_position' is used to convert from local space of model(mesh) to global space of scene.
			FbxAMatrix reference_global_init_position;
			cluster->GetTransformMatrix(reference_global_init_position);

			//'cluster_global_init_position' is used to convert from local space of bone to global space of scene. 
			FbxAMatrix cluster_global_init_position;
			cluster->GetTransformLinkMatrix(cluster_global_init_position);

			// Matrices are defined using the Column Major scheme. When a FbxAMatrix represents a transformation
			// (translation, rotation and scale), the last row of the matrix represents the translation part of the transformation. 
			// Compose 'bone.offset_transform' matrix that trnasforms position from mesh space to bone space. 
			// This matrix is called the offset matrix. 

			bone.offset_transform = to_xmfloat4x4(cluster_global_init_position.Inverse() * reference_global_init_position);
		}
	}
}

void Skinned_Mesh::Fetch_Animation(FbxScene* fbx_scene, std::vector<Animation>& animations, float sampling_rate)
{
	FbxArray<FbxString*> animation_stack_names;
	fbx_scene->FillAnimStackNameArray(animation_stack_names);
	const int animation_stack_count{ animation_stack_names.GetCount() };
	for (int i = 0; i < animation_stack_count; ++i)
	{
		Animation& animation{ animations.emplace_back() };
		animation.name = animation_stack_names[i]->Buffer();

		FbxAnimStack* animation_stack{ fbx_scene->FindMember<FbxAnimStack>(animation.name.c_str()) };
		fbx_scene->SetCurrentAnimationStack(animation_stack);

		const FbxTime::EMode time_mode{ fbx_scene->GetGlobalSettings().GetTimeMode() };
		FbxTime one_sec;
		one_sec.SetTime(0, 0, 1, 0, 0, time_mode);
		animation.sampling_rate = sampling_rate > 0 ? sampling_rate : static_cast<float>(one_sec.GetFrameRate(time_mode));
		const FbxTime sampling_interval{ static_cast<FbxLongLong>(one_sec.Get() / animation.sampling_rate) };
		const FbxTakeInfo* take_info{ fbx_scene->GetTakeInfo(animation.name.c_str()) };
		const FbxTime start_time{ take_info->mLocalTimeSpan.GetStart() };
		const FbxTime stop_time{ take_info->mLocalTimeSpan.GetStop() };
		for (FbxTime time = start_time; time < stop_time; time += sampling_interval)
		{
			Animation::Keyframe& keyframe{ animation.keyframes.emplace_back()};

			const size_t node_count{ scene_view.nodes.size() };
			keyframe.nodes.resize(node_count);
			for (size_t node_index = 0; node_index < node_count; ++node_index)
			{
				FbxNode* fbx_node{ fbx_scene->FindNodeByName(scene_view.nodes.at(node_index).name.c_str()) };
				if (fbx_node)
				{
					Animation::Keyframe::Node& node{ keyframe.nodes.at(node_index) };
					// 'global_transform' is a transformation matrix of a node with respect to the scene's global coordinate system. 
					node.global_transform = to_xmfloat4x4(fbx_node->EvaluateGlobalTransform(time));
				}
			}
		}
	}

	for (int animation_stack_index = 0; animation_stack_index < animation_stack_count; ++animation_stack_index)
	{
		delete animation_stack_names[animation_stack_index];
	}
}