#pragma once
#define NOMINMAX
#include <d3d11.h>	
#include <wrl.h>
#include <directxmath.h>
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "../tinygltf-release/tiny_gltf.h"
#include "Graphics.h"

class GltfModel
{
	std::string	filename;
public :
	GltfModel(const std::string& filename);
	virtual ~GltfModel() = default;

private :


	// Scene Data
	struct Scene
	{
		std::string name;
		std::vector<int> nodes; // Array of root nodes
	};

	std::vector<Scene> scenes;
	int defaultScene = 0;

	// Node Data
	struct Node
	{
		std::string name;
		int skin{ -1 }; // index of skin referenced by this node
		int mesh{ -1 }; // index of mesh referenced by this node	

		std::vector<int> children; // Array of indices of child nodes of this node

		// Local transform
		DirectX::XMFLOAT4 rotation{ 0, 0, 0, 1 }; // Rotation in quaternion
		DirectX::XMFLOAT3 scale{ 1, 1, 1 }; // Non-uniform scale	
		DirectX::XMFLOAT3 translation{ 0, 0, 0 }; // Translation

		DirectX::XMFLOAT4X4 globalTransform{ 1, 0, 0, 0,
									   0, 1, 0, 0,
									   0, 0, 1, 0,
									   0, 0, 0, 1 }; 
	};
	std::vector<Node> nodes;

	// Buffer View Data
	struct BufferView
	{
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		int buffer{ -1 };
		size_t strideInBytes{ 0 };
		size_t byteOffset{ 0 };
		size_t count{ 0 };
	};

	// Mesh Data
	struct Mesh
	{
		std::string name;
		struct Primitive
		{
			int material;
			std::map<std::string, BufferView> vertexBufferViews;
			BufferView indexBufferView;

			bool has(const char* attribute)const
			{
				return vertexBufferViews.find(attribute) != vertexBufferViews.end() &&
					vertexBufferViews.at(attribute).buffer != -1;
			}
		};
		std::vector<Primitive> primitives;	
	};
	std::vector<Mesh> meshes;
	std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> buffers;

	void FetchNodes(const tinygltf::Model& gltfModel);

	void CumulateTransform(std::vector<Node>& nodes);

	DXGI_FORMAT ConvertFormat(const tinygltf::Accessor& accessor);

	void FetchMesh(const tinygltf::Model& gltfModel);	
};	