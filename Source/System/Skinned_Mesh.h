#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <vector>
#include <string>
#include <fbxsdk.h>
#include <unordered_map>

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
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal{ 0, 1, 0 };
		DirectX::XMFLOAT2 texcoord{ 0, 0 };
	};

	// Constant buffer structure for shader constants
	struct Constants
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4 material_color;
	};

	// Mesh data
	struct Mesh
	{
		uint64_t unique_id{ 0 };
		std::string name;
		// 'node_index' is an index that refers to the node array of the scene. 
		int64_t node_index{ 0 };

		std::vector<Vertex>vertices;
		std::vector<uint32_t> indices;

	private :
		Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer;
		friend class Skinned_Mesh;
	};
	std::vector<Mesh> meshes;

	// Material data
	struct Material
	{
		uint64_t unique_id{ 0 };
		std::string name;

		DirectX::XMFLOAT4 ka{ 0.2f, 0.2f, 0.2f, 1.0f }; // Ambient color
		DirectX::XMFLOAT4 kd{ 0.8f, 0.8f, 0.8f, 1.0f };    // Diffuse color
		DirectX::XMFLOAT4 ks{ 1.0f, 1.0f, 1.0f, 1.0f };    // Specular color

		std::string texture_filenames[4]; // Texture filename
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvs[4];
	};
	std::unordered_map<uint64_t, Material> materials;	

private :
	
	// All the shaders with input layout and constant buffer
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

public :

	Skinned_Mesh(const char* fbx_filename, bool triangulate = false);
	virtual ~Skinned_Mesh() = default;

	// Fetch meshes from the FBX scene
	void Fetch_meshes(FbxScene* fbx_scene, std::vector<Mesh>& meshes);

	// Fetch materials from the FBX scene
	void Fetch_materials(FbxScene* fbx_scene, std::unordered_map<uint64_t, Material>& materials);
	
	// Create COM objects for rendering
	void Create_com_object(const char* fbx_filename);

	// Render the skinned mesh
	void Render(const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& material_color = {0, 0, 0, 1});

protected:

	// Hold the scene data here
	scene scene_view;
};
