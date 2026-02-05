#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <vector>
#include <string>
#include <fbxsdk.h>

// Structure representing an FBX scene with nodes
struct scene
{
	// Node structure representing a node in the FBX scene
	struct node
	{
		uint64_t unique_id{ 0 };
		std::string name;
		FbxNodeAttribute::EType attribute{ FbxNodeAttribute::eUnknown };	
		int64_t parent_index{ -1 };
	};
	std::vector<node> nodes;
	
	// Function to find the index of a node by its unique ID
	int64_t indexof(uint64_t unique_id) const
	{
		int64_t index{ 0 };
		for (const auto& node : nodes)
		{
			if(node.unique_id == unique_id)
			{
				return index;
			}
			++index;
		}
		return -1;
	}
};

class Skinned_Mesh
{
public :
	// Vertex structure representing a vertex in the skinned mesh
	struct vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal{ 0, 1, 0 };
		DirectX::XMFLOAT2 texcoord{ 0, 0 };
	};

	// Constant buffer structure for shader constants
	struct constants
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4 material_color;
	};

private :
	
	//All the shaders with input layout and constant buffer
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

public :

	Skinned_Mesh(const char* fbx_filename, bool triangulate = false);
	virtual ~Skinned_Mesh() = default;

protected:

	//Hold the scene data here
	scene scene_view;
};
