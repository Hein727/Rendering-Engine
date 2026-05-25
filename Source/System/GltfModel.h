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
	void Render(const DirectX::XMFLOAT4X4 world);

private :

	struct Scene
	{
		std::string name;
		std::vector<int> nodes;
	};

	std::vector<Scene> scenes;
	int defaultScene = 0;

	struct Node
	{
		std::string name;
		int skin{ -1 }; 
		int mesh{ -1 }; 

		std::vector<int> children;

		DirectX::XMFLOAT4 rotation{ 0, 0, 0, 1 };
		DirectX::XMFLOAT3 scale{ 1, 1, 1 };	
		DirectX::XMFLOAT3 translation{ 0, 0, 0 }; 

		DirectX::XMFLOAT4X4 globalTransform{ 1, 0, 0, 0,
									   0, 1, 0, 0,
									   0, 0, 1, 0,
									   0, 0, 0, 1 }; 
	};
	std::vector<Node> nodes;

	struct BufferView
	{
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		int buffer{ -1 };
		size_t strideInBytes{ 0 };
		size_t byteOffset{ 0 };
		size_t count{ 0 };
	};

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

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;	
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	struct PrimitiveConst
	{
		DirectX::XMFLOAT4X4 world;
		int material{ -1 };
		int has_tangent{ 0 };	
		int skin{ -1 };
		int pad;
	};
	Microsoft::WRL::ComPtr<ID3D11Buffer> primitiveConstBuffer;
};	