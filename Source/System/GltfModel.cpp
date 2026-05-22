#include "GltfModel.h"
#define TINYGLTF_IMPLEMENTATION
#include "../tinygltf-release/tiny_gltf.h"
#include "Misc.h"
#include <stack>

bool null_load_image_data(tinygltf::Image*, const int, std::string*, std::string*, int, int, const unsigned char*, int, void*)
{
	return true;
}

GltfModel::GltfModel(const std::string& filename) : filename(filename)
{
	auto device = graphics::getInstance().GetDevice();

	tinygltf::TinyGLTF tinyGltf;
	tinyGltf.SetImageLoader(null_load_image_data, nullptr);	

	tinygltf::Model gltfModel;
	std::string error, warning;
	bool succeeded{ false };
	if (filename.find(".glb") != std::string::npos)
	{
		succeeded = tinyGltf.LoadBinaryFromFile(&gltfModel, &error, &warning, filename.c_str());
	}
	else if (filename.find(".gltf") != std::string::npos)
	{
		succeeded = tinyGltf.LoadASCIIFromFile(&gltfModel, &error, &warning, filename.c_str());
	}

	_ASSERT_EXPR(warning.empty(), warning.c_str());
	_ASSERT_EXPR(error.empty(), error.c_str());
	_ASSERT_EXPR(succeeded, L"Filed to load gltf file");

	for(std::vector<tinygltf::Scene>::const_reference gltfScene : gltfModel.scenes)
	{
		Scene& scene{ scenes.emplace_back() };
		scene.name = gltfScene.name;
		scene.nodes = gltfScene.nodes;
	}

	defaultScene = gltfModel.defaultScene < 0 ? 0 : gltfModel.defaultScene;

	// fetch nodes before meshes and skins, as nodes contain mesh and skin indices
	FetchNodes(gltfModel);

	// fetch meshes before skins, as skins contain inverse bind matrices which are stored in mesh's buffer views
	FetchMesh(gltfModel);
}

void GltfModel::FetchNodes(const tinygltf::Model& gltfModel)
{
	for (std::vector<tinygltf::Node>::const_reference gltfNode : gltfModel.nodes)
	{
		Node& node{ nodes.emplace_back() };
		node.name = gltfNode.name;
		node.skin = gltfNode.skin;
		node.mesh = gltfNode.mesh;
		node.children = gltfNode.children;
		if (!gltfNode.matrix.empty())
		{
			DirectX::XMFLOAT4X4 matrix;
			for (size_t row = 0; row < 4; ++row)
			{
				for(size_t column = 0; column < 4; ++column)
				{
					matrix(row, column) = static_cast<float>(gltfNode.matrix.at(4 * row + column));
				}	
			}

			DirectX::XMVECTOR S, T, R;

			bool succeeded = DirectX::XMMatrixDecompose(&S, &R, &T, DirectX::XMLoadFloat4x4(&matrix));
			_ASSERT_EXPR(succeeded, L"Failed to decompose transform matrix");

			DirectX::XMStoreFloat3(&node.scale, S);
			DirectX::XMStoreFloat4(&node.rotation, R);
			DirectX::XMStoreFloat3(&node.translation, T);
		}
		else
		{
			if (gltfNode.scale.size() > 0)
			{
				node.scale.x = static_cast<float>(gltfNode.scale.at(0));
				node.scale.y = static_cast<float>(gltfNode.scale.at(1));
				node.scale.z = static_cast<float>(gltfNode.scale.at(2));
			}
			if (gltfNode.rotation.size() > 0)
			{
				node.rotation.x = static_cast<float>(gltfNode.rotation.at(0));
				node.rotation.y = static_cast<float>(gltfNode.rotation.at(1));
				node.rotation.z = static_cast<float>(gltfNode.rotation.at(2));
				node.rotation.w = static_cast<float>(gltfNode.rotation.at(3));
			}
			if (gltfNode.translation.size() > 0)
			{
				node.translation.x = static_cast<float>(gltfNode.translation.at(0));
				node.translation.y = static_cast<float>(gltfNode.translation.at(1));
				node.translation.z = static_cast<float>(gltfNode.translation.at(2));
			}
		}
	}
	CumulateTransform(nodes);
}

void GltfModel::CumulateTransform(std::vector<Node>& nodes)
{
	using namespace DirectX;

	std::stack<XMFLOAT4X4> parentGlobalTransforms;	
	std::function<void(int)> traverse{ [&](int nodeIndex)->void
		{
			Node& node{ nodes.at(nodeIndex)};
			XMMATRIX S{ XMMatrixScaling(node.scale.x, node.scale.y, node.scale.z) };
			XMMATRIX R{ XMMatrixRotationQuaternion(XMLoadFloat4(&node.rotation)) };
			XMMATRIX T{ XMMatrixTranslation(node.translation.x, node.translation.y, node.translation.z) };
			XMStoreFloat4x4(&node.globalTransform, S * R * T * XMLoadFloat4x4(&parentGlobalTransforms.top()));
			for (int childIndex : node.children)
			{
				parentGlobalTransforms.push(node.globalTransform);
				traverse(childIndex);
				parentGlobalTransforms.pop();
			}
	}	};
	for (std::vector<int>::value_type nodeIndex : scenes.at(defaultScene).nodes)
	{
		parentGlobalTransforms.push(XMFLOAT4X4{ 1, 0, 0, 0,
											   0, 1, 0, 0,
											   0, 0, 1, 0,
											   0, 0, 0, 1 });
		traverse(nodeIndex);
		parentGlobalTransforms.pop();
	}
}

DXGI_FORMAT GltfModel::ConvertFormat(const tinygltf::Accessor& accessor)
{
	switch (accessor.type)
	{
		case TINYGLTF_TYPE_SCALAR:
			switch (accessor.componentType)
			{
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
				return DXGI_FORMAT_R8_UINT;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
				return DXGI_FORMAT_R16_UINT;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
				return DXGI_FORMAT_R32_UINT;
			default:
				_ASSERT_EXPR(FALSE, L"Unsupported component type for scalar accessor");
				return DXGI_FORMAT_UNKNOWN;
			}

		case TINYGLTF_TYPE_VEC2:
			switch (accessor.componentType)
			{
			case TINYGLTF_COMPONENT_TYPE_FLOAT:
				return DXGI_FORMAT_R32G32_FLOAT;
			default:
				_ASSERT_EXPR(FALSE, L"Unsupported component type for vec2 accessor");
				return DXGI_FORMAT_UNKNOWN;
			}

		case TINYGLTF_TYPE_VEC3:
			switch (accessor.componentType)
			{
			case TINYGLTF_COMPONENT_TYPE_FLOAT:
				return DXGI_FORMAT_R32G32B32_FLOAT;
			default:
				_ASSERT_EXPR(FALSE, L"Unsupported component type for vec3 accessor");
				return DXGI_FORMAT_UNKNOWN;
			}

		case TINYGLTF_TYPE_VEC4:
			switch (accessor.componentType)
			{
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
					return DXGI_FORMAT_R8G8B8A8_UINT;
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
					return DXGI_FORMAT_R16G16B16A16_UINT;
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
					return DXGI_FORMAT_R32G32B32A32_UINT;
				case TINYGLTF_COMPONENT_TYPE_FLOAT:
					return DXGI_FORMAT_R32G32B32A32_FLOAT;
				default:
					_ASSERT_EXPR(FALSE, L"Unsupported component type for vec4 accessor");
					return DXGI_FORMAT_UNKNOWN;
			}
			break;

		default:
			_ASSERT_EXPR(FALSE, L"Unsupported accessor type");
			return DXGI_FORMAT_UNKNOWN;
	}
}

void GltfModel::FetchMesh(const tinygltf::Model& gltfModel)
{
	HRESULT hr;
	
	auto device = graphics::getInstance().GetDevice();

	size_t gltfBufferCount = gltfModel.buffers.size();
	buffers.resize(gltfBufferCount);

	//Create buffers
	for (size_t gltfBufferIndex = 0; gltfBufferIndex < gltfBufferCount; ++gltfBufferIndex)
	{
		const tinygltf::Buffer& gltfBuffer = gltfModel.buffers.at(gltfBufferIndex);

		D3D11_BUFFER_DESC bufferDesc{};	
		bufferDesc.ByteWidth = static_cast<UINT>(gltfBuffer.data.size());
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_INDEX_BUFFER;
		D3D11_SUBRESOURCE_DATA subresourceData{};
		subresourceData.pSysMem = gltfBuffer.data.data();
		hr = device->CreateBuffer(&bufferDesc, &subresourceData, buffers.at(gltfBufferIndex).GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));
	}

	for (std::vector<tinygltf::Mesh>::const_reference gltfMesh : gltfModel.meshes)
	{
		Mesh& mesh{ meshes.emplace_back() };
		mesh.name = gltfMesh.name;
		
		for (std::vector<tinygltf::Primitive>::const_reference gltfPrimitive : gltfMesh.primitives)
		{
			Mesh::Primitive& primitive{ mesh.primitives.emplace_back() };
			primitive.material = gltfPrimitive.material;

			// Create index buffer view
			if (gltfPrimitive.indices > -1)
			{
				const tinygltf::Accessor& gltfAccessor{ gltfModel.accessors.at(gltfPrimitive.indices) };
				const tinygltf::BufferView& gltfBufferView{ gltfModel.bufferViews.at(gltfAccessor.bufferView) };

				primitive.indexBufferView.format = ConvertFormat(gltfAccessor);
				primitive.indexBufferView.buffer = gltfBufferView.buffer;
				primitive.indexBufferView.strideInBytes = gltfAccessor.ByteStride(gltfBufferView);
				primitive.indexBufferView.byteOffset = gltfBufferView.byteOffset + gltfAccessor.byteOffset;
				primitive.indexBufferView.count = gltfAccessor.count;
			}

			// Create vertex buffer views
			for (std::map<std::string, int>::const_reference gltfAttribute : gltfPrimitive.attributes)
			{
				const tinygltf::Accessor& gltfAccessor{ gltfModel.accessors.at(gltfAttribute.second) };
				const tinygltf::BufferView& gltfBufferView{ gltfModel.bufferViews.at(gltfAccessor.bufferView) };

				BufferView vertexBufferView{};
				vertexBufferView.format = ConvertFormat(gltfAccessor);
				vertexBufferView.buffer = gltfBufferView.buffer;
				vertexBufferView.strideInBytes = gltfAccessor.ByteStride(gltfBufferView);
				vertexBufferView.byteOffset = gltfBufferView.byteOffset + gltfAccessor.byteOffset;
				vertexBufferView.count = gltfAccessor.count;

				primitive.vertexBufferViews.emplace(std::make_pair(gltfAttribute.first, vertexBufferView));
			}
		}
	}
}