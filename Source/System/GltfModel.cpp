#include "GltfModel.h"
#define TINYGLTF_IMPLEMENTATION
#include "../tinygltf-release/tiny_gltf.h"
#include "Misc.h"
#include <stack>
#include "Shader.h"
#include "Texture.h"

bool null_load_image_data(tinygltf::Image*, const int, std::string*, std::string*, int, int, const unsigned char*, int, void*)
{
	return true;
}

GltfModel::GltfModel(GameContext& gameContext, const std::string& filename) : filename(filename), gameContext(gameContext)
{
	auto device = gameContext.graphics.GetDevice();

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

	FetchNodes(gltfModel);

	FetchMesh(gltfModel);

	D3D11_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "JOINTS", 0, DXGI_FORMAT_R16G16B16A16_UINT, 4, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 5, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	createVsFromCso(device, "Shader\\GltfModel_vs.cso", vertexShader.ReleaseAndGetAddressOf(), 
		inputLayout.ReleaseAndGetAddressOf(), inputElementDescs, _countof(inputElementDescs));
	createPsFromCso(device, "Shader\\GltfModel_ps.cso", pixelShader.ReleaseAndGetAddressOf());

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(PrimitiveConst);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	HRESULT hr = device->CreateBuffer(&bufferDesc, nullptr, primitiveConstBuffer.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	bufferDesc.ByteWidth = sizeof(PrimitiveJointConsts);
	hr = device->CreateBuffer(&bufferDesc, nullptr, primitiveJointCBuffer.ReleaseAndGetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	FetchMaterial(gltfModel);

	FetchTexture(gltfModel);

	FetchAnimation(gltfModel);
}

void GltfModel::FetchNodes(const tinygltf::Model& gltfModel)
{
	DirectX::XMFLOAT3 min{ -FLT_MAX, -FLT_MAX, -FLT_MAX }, max{ FLT_MAX, FLT_MAX, FLT_MAX };

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
	
	auto device = gameContext.graphics.GetDevice();

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


			//	AABB implementation 
			auto& positionVertexBufferView = primitive.vertexBufferViews.at("POSITION");

			const auto& positionAccessor = gltfModel.accessors.at(gltfPrimitive.attributes.at("POSITION"));
			const auto& positionBufferView = gltfModel.bufferViews.at(positionAccessor.bufferView);
			const auto& positionBuffer = gltfModel.buffers.at(positionBufferView.buffer);

			const float* positionData = reinterpret_cast<const float*>(positionBuffer.data.data() + positionBufferView.byteOffset + positionAccessor.byteOffset);
			DirectX::XMFLOAT3 min{ FLT_MAX, FLT_MAX, FLT_MAX }, max{ -FLT_MAX, -FLT_MAX, -FLT_MAX };

			for (int i = 0; i < positionAccessor.count; ++i)
			{
				float x = positionData[i * 3];
				float y = positionData[i * 3 + 1];
				float z = positionData[i * 3 + 2];

				minPos.x = std::min(minPos.x, x);
				minPos.y = std::min(minPos.y, y);
				minPos.z = std::min(minPos.z, z);
				maxPos.x = std::max(maxPos.x, x);
				maxPos.y = std::max(maxPos.y, y);
				maxPos.z = std::max(maxPos.z, z);
			}

		}
	}
}

void GltfModel::FetchMaterial(const tinygltf::Model& gltfModel)
{
	for (std::vector<tinygltf::Material>::const_reference gltfMaterial : gltfModel.materials)
	{
		std::vector<Material>::reference material{ materials.emplace_back() };

		material.name = gltfMaterial.name;

		material.data.emassiveFactor[0] = static_cast<float>(gltfMaterial.emissiveFactor.at(0));
		material.data.emassiveFactor[1] = static_cast<float>(gltfMaterial.emissiveFactor.at(1));
		material.data.emassiveFactor[2] = static_cast<float>(gltfMaterial.emissiveFactor.at(2));

		material.data.alphaMode = gltfMaterial.alphaMode == "OPAQUE" ? 0 : gltfMaterial.alphaMode == "MASK" ? 1 : 
			gltfMaterial.alphaMode == "BLEND" ? 2 : 0 ;
		material.data.alphaCutoff = static_cast<float>(gltfMaterial.alphaCutoff);
		material.data.doubleSided = gltfMaterial.doubleSided ? 1 : 0;

		material.data.pbrMetallicRoughness.baseColorFactor[0] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(0));
		material.data.pbrMetallicRoughness.baseColorFactor[1] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(1));
		material.data.pbrMetallicRoughness.baseColorFactor[2] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(2));
		material.data.pbrMetallicRoughness.baseColorFactor[3] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor.at(3));
		material.data.pbrMetallicRoughness.baseColorTexture.index = gltfMaterial.pbrMetallicRoughness.baseColorTexture.index;
		material.data.pbrMetallicRoughness.baseColorTexture.texcoord = gltfMaterial.pbrMetallicRoughness.baseColorTexture.texCoord;
		material.data.pbrMetallicRoughness.metallicFactor = static_cast<float>(gltfMaterial.pbrMetallicRoughness.metallicFactor);
		material.data.pbrMetallicRoughness.metallicRoughnessTexture.index = gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index;
		material.data.pbrMetallicRoughness.metallicRoughnessTexture.texcoord = gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.texCoord;

		material.data.normalTexture.index = gltfMaterial.normalTexture.index;
		material.data.normalTexture.texcoord = gltfMaterial.normalTexture.texCoord;
		material.data.normalTexture.scale = static_cast<float>(gltfMaterial.normalTexture.scale);

		material.data.occlusionTexture.index = gltfMaterial.occlusionTexture.index;
		material.data.occlusionTexture.texcoord = gltfMaterial.occlusionTexture.texCoord;
		material.data.occlusionTexture.strength = static_cast<float>(gltfMaterial.occlusionTexture.strength);

		material.data.emissiveTexture.index = gltfMaterial.emissiveTexture.index;
		material.data.emissiveTexture.texcoord = gltfMaterial.emissiveTexture.texCoord;
	}

	std::vector<Material::CBuffer> materialData;
	for (std::vector<Material>::const_reference material : materials)
	{
		materialData.emplace_back(material.data);
	}

	HRESULT hr;
	Microsoft::WRL::ComPtr<ID3D11Buffer> materialBuffer;
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Material::CBuffer) * materialData.size());
	bufferDesc.StructureByteStride = sizeof(Material::CBuffer);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	D3D11_SUBRESOURCE_DATA subresourceData{};
	subresourceData.pSysMem = materialData.data();
	auto device = gameContext.graphics.GetDevice();
	hr = device->CreateBuffer(&bufferDesc, &subresourceData, materialBuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.NumElements = static_cast<UINT>(materialData.size());
	hr = device->CreateShaderResourceView(materialBuffer.Get(), &srvDesc, materialResourceView.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));
}

void GltfModel::Render(const DirectX::XMFLOAT4X4 world, const std::vector<Node>& animatedNodes)
{
	using namespace DirectX;

	const std::vector<Node>& nodes{ animatedNodes.size() > 0 ? animatedNodes : GltfModel::nodes };
	
	auto context = gameContext.graphics.GetDeviceContext();

	context->VSSetShader(vertexShader.Get(), nullptr, 0);
	context->PSSetShader(pixelShader.Get(), nullptr, 0);
	context->IASetInputLayout(inputLayout.Get());	
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	std::function<void(int)> traverse{ [&](int nodeIndex) -> void
	{
			const Node& node{ nodes.at(nodeIndex) };
			if (node.mesh > -1)
			{
				const Mesh& mesh{ meshes.at(node.mesh) };
				for (std::vector<Mesh::Primitive>::const_reference primitive : mesh.primitives)
				{
					ID3D11Buffer* vertexBuffers[]{
						primitive.has("POSITION") ?
						buffers.at(primitive.vertexBufferViews.at("POSITION").buffer).Get() : NULL,
						primitive.has("NORMAL") ?
						buffers.at(primitive.vertexBufferViews.at("NORMAL").buffer).Get() : NULL,
						primitive.has("TANGENT") ?
						buffers.at(primitive.vertexBufferViews.at("TANGENT").buffer).Get() : NULL,
						primitive.has("TEXCOORD_0") ?
						buffers.at(primitive.vertexBufferViews.at("TEXCOORD_0").buffer).Get() : NULL,
						primitive.has("JOINTS_0") ?
						buffers.at(primitive.vertexBufferViews.at("JOINTS_0").buffer).Get() : NULL,
						primitive.has("WEIGHTS_0") ?
						buffers.at(primitive.vertexBufferViews.at("WEIGHTS_0").buffer).Get() : NULL
					};

					UINT strides[]{
						primitive.has("POSITION") ?
						static_cast<UINT>(primitive.vertexBufferViews.at("POSITION").strideInBytes) : 0,
						primitive.has("NORMAL") ?
						static_cast<UINT>(primitive.vertexBufferViews.at("NORMAL").strideInBytes) : 0,
						primitive.has("TANGENT") ?
						static_cast<UINT>(primitive.vertexBufferViews.at("TANGENT").strideInBytes) : 0,
						primitive.has("TEXCOORD_0") ?
						static_cast<UINT>(primitive.vertexBufferViews.at("TEXCOORD_0").strideInBytes) : 0,
						primitive.has("JOINTS_0") ?
						static_cast<UINT>(primitive.vertexBufferViews.at("JOINTS_0").strideInBytes) : 0,
						primitive.has("WEIGHTS_0") ?
						static_cast<UINT>(primitive.vertexBufferViews.at("WEIGHTS_0").strideInBytes) : 0
					};

					UINT offsets[]{
						primitive.has("POSITION") ?
						static_cast<UINT>(primitive.vertexBufferViews.at("POSITION").byteOffset) : 0,
						primitive.has("NORMAL") ?
						static_cast<UINT>(primitive.vertexBufferViews.at("NORMAL").byteOffset) : 0,
						primitive.has("TANGENT") ?
						static_cast<UINT>(primitive.vertexBufferViews.at("TANGENT").byteOffset) : 0,
						primitive.has("TEXCOORD_0") ?
						static_cast<UINT>(primitive.vertexBufferViews.at("TEXCOORD_0").byteOffset) : 0,
						primitive.has("JOINTS_0") ?
						static_cast<UINT>(primitive.vertexBufferViews.at("JOINTS_0").byteOffset) : 0,
						primitive.has("WEIGHTS_0") ?
						static_cast<UINT>(primitive.vertexBufferViews.at("WEIGHTS_0").byteOffset) : 0
					};
					context->IASetVertexBuffers(0, _countof(vertexBuffers), vertexBuffers, strides, offsets);

					const Material& material{ materials.at(primitive.material) };
					const int textureIndices[]{
						material.data.pbrMetallicRoughness.baseColorTexture.index,
						material.data.pbrMetallicRoughness.metallicRoughnessTexture.index,
						material.data.normalTexture.index,
						material.data.emissiveTexture.index,
						material.data.occlusionTexture.index,
					};
					ID3D11ShaderResourceView* nullSRV{};
					std::vector<ID3D11ShaderResourceView*> srvs(_countof(textureIndices));
					for (int textureIndex = 0; textureIndex < srvs.size(); ++textureIndex)
					{
						srvs.at(textureIndex) = textureIndices[textureIndex] > -1 ?
							textureResourceViews.at(textureIndices[textureIndex]).Get() : nullSRV;
					}
					context->PSSetShaderResources(1, static_cast<UINT>(srvs.size()), srvs.data());

					if (node.skin > -1)
					{
						const Skin& skin{ skins.at(node.skin) };
						PrimitiveJointConsts primitiveJointConsts{};
						for(size_t jointIndex = 0; jointIndex < skin.joints.size(); ++jointIndex)
						{
							XMStoreFloat4x4(&primitiveJointConsts.matrices[jointIndex],
							XMLoadFloat4x4(&skin.inverseBindMatrices.at(jointIndex)) * 
							XMLoadFloat4x4(&nodes.at(skin.joints.at(jointIndex)).globalTransform) *
								XMMatrixInverse(NULL, XMLoadFloat4x4(&node.globalTransform))
							);
						}
						context->UpdateSubresource(primitiveJointCBuffer.Get(), 0, nullptr, &primitiveJointConsts, 0, 0);
						context->VSSetConstantBuffers(2, 1, primitiveJointCBuffer.GetAddressOf());
					}

					PrimitiveConst primitiveConst{};
					primitiveConst.material = primitive.material;
					primitiveConst.has_tangent = primitive.has("TANGENT");
					primitiveConst.skin = node.skin;
					XMStoreFloat4x4(&primitiveConst.world, XMLoadFloat4x4(&node.globalTransform) * XMLoadFloat4x4(&world));

					context->UpdateSubresource(primitiveConstBuffer.Get(), 0, nullptr, &primitiveConst, 0, 0);
					context->VSSetConstantBuffers(1, 1, primitiveConstBuffer.GetAddressOf());
					context->PSSetConstantBuffers(1, 1, primitiveConstBuffer.GetAddressOf());
					context->PSSetShaderResources(0, 1, materialResourceView.GetAddressOf());

					if (primitive.indexBufferView.buffer > -1)
					{
						context->IASetIndexBuffer(buffers.at(primitive.indexBufferView.buffer).Get(),
							primitive.indexBufferView.format, static_cast<UINT>(primitive.indexBufferView.byteOffset));
						context->DrawIndexed(static_cast<UINT>(primitive.indexBufferView.count), 0, 0);
					}
					else
					{
						context->Draw(static_cast<UINT>(primitive.vertexBufferViews.at("POSITION").count), 0);
					}
				}
			}
			for (std::vector<int>::value_type childIndex : node.children)
			{
				traverse(childIndex);
			}
	} };

	for(std::vector<int>::value_type nodeIndex : scenes.at(defaultScene).nodes)
	{
		traverse(nodeIndex);
	}
}

void GltfModel::FetchTexture(const tinygltf::Model& gltfModel)
{
	auto device = gameContext.graphics.GetDevice();

	HRESULT hr{ S_OK };
	for (const tinygltf::Texture& gltfTexture : gltfModel.textures)
	{
		Texture& texture{ textures.emplace_back() };	
		texture.name = gltfTexture.name;
		texture.source = gltfTexture.source;
	}

	for(const tinygltf::Image& gltfImage : gltfModel.images)
	{
		Image& image{ images.emplace_back() };
		image.name = gltfImage.name;
		image.width = gltfImage.width;
		image.height = gltfImage.height;
		image.component = gltfImage.component;
		image.bits = gltfImage.bits;
		image.pixelType = gltfImage.pixel_type;
		image.bufferView = gltfImage.bufferView;
		image.mimeType = gltfImage.mimeType;
		image.uri = gltfImage.uri;
		image.asIs = gltfImage.as_is;

		if (gltfImage.bufferView > -1)
		{
			const tinygltf::BufferView& gltfBufferView{ gltfModel.bufferViews.at(gltfImage.bufferView) };
			const tinygltf::Buffer& buffer{ gltfModel.buffers.at(gltfBufferView.buffer) };
			const BYTE* data = buffer.data.data() + gltfBufferView.byteOffset;

			ID3D11ShaderResourceView* trv{};
			hr = loadTextureFromMemory(device , data, static_cast<size_t>(gltfBufferView.byteLength), &trv);
			if (hr == S_OK)
			{
				textureResourceViews.emplace_back().Attach(trv);
			}
		}
		else
		{
			const std::filesystem::path path(filename);
			ID3D11ShaderResourceView* srv{};
			D3D11_TEXTURE2D_DESC textureDesc{};
			std::wstring gltfImageUri = std::wstring(gltfImage.uri.begin(), gltfImage.uri.end());
			std::wstring name;
			if (std::filesystem::path(gltfImageUri).is_absolute())
			{
				name = { gltfImageUri };
			}
			else
			{
				name = path.parent_path().concat(L"/").concat(gltfImageUri);
			}
			

			hr = loadTextureFromFile(device, name.c_str(), &srv, &textureDesc);
			
			if (hr == S_OK)
			{
				textureResourceViews.emplace_back().Attach(srv);
			}
		}
	}
}

void GltfModel::FetchAnimation(const tinygltf::Model& gltfModel)
{
	using namespace std;
	using namespace DirectX;
	using namespace tinygltf;

	for (vector<tinygltf::Skin>::const_reference transmissionSkin : gltfModel.skins)
	{
		Skin& skin{ skins.emplace_back() };
		const Accessor& gltfAccessor{ gltfModel.accessors.at(transmissionSkin.inverseBindMatrices) };
		const tinygltf::BufferView& gltfBufferView{ gltfModel.bufferViews.at(gltfAccessor.bufferView) };
		skin.inverseBindMatrices.resize(gltfAccessor.count);
		memcpy(skin.inverseBindMatrices.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() +
			gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(XMFLOAT4X4));
		skin.joints = transmissionSkin.joints;
	}

	for (vector<tinygltf::Animation>::const_reference gltfAnimation : gltfModel.animations)
	{
		Animation& animation{ animations.emplace_back() };
		animation.name = gltfAnimation.name;

		for (vector<tinygltf::AnimationSampler>::const_reference gltfSampler : gltfAnimation.samplers)
		{
			Animation::Sampler& sampler{ animation.samplers.emplace_back() };
			sampler.input = gltfSampler.input;
			sampler.output = gltfSampler.output;
			sampler.interpolation = gltfSampler.interpolation;

			const Accessor& gltfAccessor{ gltfModel.accessors.at(gltfSampler.input) };
			const tinygltf::BufferView& gltfBufferView{ gltfModel.bufferViews.at(gltfAccessor.bufferView) };
			const pair<unordered_map<int, vector<float>>::iterator, bool>& timelines{
				animation.timelines.emplace(gltfSampler.input, gltfAccessor.count)
			};
			if (timelines.second)
			{
				memcpy(timelines.first->second.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() +
					gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(FLOAT));
			}
		}
		for (vector<tinygltf::AnimationChannel>::const_reference gltfChannel : gltfAnimation.channels)
		{
			Animation::Channel& channel{ animation.channels.emplace_back() };
			channel.sampler = gltfChannel.sampler;
			channel.targetNode = gltfChannel.target_node;
			channel.targetPath = gltfChannel.target_path;
	
			const AnimationSampler& gltfSampler{ gltfAnimation.samplers.at(channel.sampler) };
			const Accessor& gltfAccessor{ gltfModel.accessors.at(gltfSampler.output) };
			const tinygltf::BufferView& gltfBufferView{ gltfModel.bufferViews.at(gltfAccessor.bufferView) };
			if (gltfChannel.target_path == "scale")
			{
				const pair<unordered_map<int, vector<XMFLOAT3>>::iterator, bool>& scales{
					animation.scales.emplace(gltfSampler.output, gltfAccessor.count)
				};

				if (scales.second)
				{
					memcpy(scales.first->second.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() +
						gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(XMFLOAT3));
				}
			}

			else if (gltfChannel.target_path == "rotation")
			{
				const pair<unordered_map<int, vector<XMFLOAT4>>::iterator, bool>& rotations{
					animation.rotation.emplace(gltfSampler.output, gltfAccessor.count)
				};

				if (rotations.second)
				{
					memcpy(rotations.first->second.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() +
						gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(XMFLOAT4));
				}
			}

			else if (gltfChannel.target_path == "translation")
			{
				const pair<unordered_map<int, vector<XMFLOAT3>>::iterator, bool>& translations{
					animation.translations.emplace(gltfSampler.output, gltfAccessor.count)
				};

				if (translations.second)
				{
					memcpy(translations.first->second.data(), gltfModel.buffers.at(gltfBufferView.buffer).data.data() +
						gltfBufferView.byteOffset + gltfAccessor.byteOffset, gltfAccessor.count * sizeof(XMFLOAT3));
				}
			}
		}
	}

	for (decltype(animations)::reference animation : animations)
	{
		for(decltype(animation.timelines)::reference timelines : animation.timelines)
		{
			animation.duration = max<float>(animation.duration, timelines.second.back());
		}
	}
}

void GltfModel::Animate(size_t animationIndex, float time, std::vector<Node>& animatedNodes)
{
	using namespace DirectX;
	using namespace std;

	function<size_t(const vector<float>&, float, float&)> Indexof{
		[](const vector<float>& timeline, float time, float& interpolationFactor)->size_t {
			const size_t keyframeCount{ timeline.size() };
			if (time > timeline.at(keyframeCount - 1))
			{
				interpolationFactor = 1.0f;
				return keyframeCount - 2;
			}
			else if (time < timeline.at(0))
			{
				interpolationFactor = 0.0f;
				return 0;
			}
			size_t keyframeIndex{ 0 };
			for (size_t timeIndex = 1; timeIndex < keyframeCount; ++timeIndex)
			{
				if (time < timeline.at(timeIndex))
				{
					keyframeIndex = max<size_t>(0LL, timeIndex - 1);
					break;
				}
			}

			interpolationFactor = (time - timeline.at(keyframeIndex)) / (timeline.at(keyframeIndex + 1) - timeline.at(keyframeIndex));

			return keyframeIndex;
		}
	};

	if (animations.size() > 0)
	{
		const Animation& animation{ animations.at(animationIndex) };
		for (vector<Animation::Channel>::const_reference channel : animation.channels)
		{
			const Animation::Sampler& sampler{ animation.samplers.at(channel.sampler) };
			const vector<float>& timeline{ animation.timelines.at(sampler.input) };
			if (timeline.size() == 0)
			{
				continue;
			}
			float interpolationFactor{ 0.0f };
			size_t keyframeIndex{ Indexof(timeline, time, interpolationFactor) };
			if (channel.targetPath == "scale")
			{
				const vector<XMFLOAT3>& scales{ animation.scales.at(sampler.output) };
				XMStoreFloat3(&animatedNodes.at(channel.targetNode).scale, 
					XMVectorLerp(XMLoadFloat3(&scales.at(keyframeIndex + 0)),
					XMLoadFloat3(&scales.at(keyframeIndex + 1)), interpolationFactor));
			}
			else if (channel.targetPath == "rotation")
			{
				const vector<XMFLOAT4>& rotations{ animation.rotation.at(sampler.output) };
				DirectX::XMStoreFloat4(&animatedNodes.at(channel.targetNode).rotation,
					XMQuaternionSlerp(XMLoadFloat4(&rotations.at(keyframeIndex)), XMLoadFloat4(&rotations.at(keyframeIndex + 1)), interpolationFactor));
			}
			else if(channel.targetPath == "translation")
			{
				const vector<XMFLOAT3>& translations = animation.translations.at(sampler.output);
				XMStoreFloat3(&animatedNodes.at(channel.targetNode).translation,
					XMVectorLerp(XMLoadFloat3(&translations.at(keyframeIndex)), XMLoadFloat3(&translations.at(keyframeIndex + 1)), interpolationFactor));
			}
		}
		CumulateTransform(animatedNodes);
	}
}