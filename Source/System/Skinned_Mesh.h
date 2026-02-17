#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <vector>
#include <string>
#include <fbxsdk.h>
#include <unordered_map>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/unordered_map.hpp>	

namespace DirectX
{
	template<class T>
	void serialize(T& archive, DirectX::XMFLOAT2& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y)
		);
	}

	template<class T>
	void serialize(T& archive, DirectX::XMFLOAT3& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y),
			cereal::make_nvp("z", v.z)
		);
	}

	template<class T>
	void serialize(T& archive, DirectX::XMFLOAT4& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y),
			cereal::make_nvp("z", v.z),
			cereal::make_nvp("w", v.w)
		);
	}

	template<class T>
	void serialize(T& archive, DirectX::XMFLOAT4X4& m)
	{
		archive(
			cereal::make_nvp("_11", m._11), cereal::make_nvp("_12", m._12), cereal::make_nvp("_13", m._13), cereal::make_nvp("_14", m._14),
			cereal::make_nvp("_21", m._21), cereal::make_nvp("_22", m._22), cereal::make_nvp("_23", m._23), cereal::make_nvp("_24", m._24),
			cereal::make_nvp("_31", m._31), cereal::make_nvp("_32", m._32), cereal::make_nvp("_33", m._33), cereal::make_nvp("_34", m._34),
			cereal::make_nvp("_41", m._41), cereal::make_nvp("_42", m._42), cereal::make_nvp("_43", m._43), cereal::make_nvp("_44", m._44)
		);
	}
};

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

		template<class T>
		void serialize(T& archive)
		{
			archive(unique_id, name, attribute, parent_index);
		}
	};
	std::vector<node> nodes;

	template<class T>
	void serialize(T& archive)
	{
		archive(nodes);
	}
	
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

struct Skeleton
{
	struct Bone
	{
		uint64_t unique_id{ 0 };
		std::string name;
		int64_t parent_index{ -1 };
		int64_t node_index{ 0 };
		DirectX::XMFLOAT4X4 offset_transform =
		{
			1,0,0,0,
			0,1,0,0,
			0,0,1,0,
			0,0,0,1.
		};

		template<class T>
		void serialize(T& archive)
		{
			archive(unique_id, name, parent_index, node_index, offset_transform);
		}

		bool is_orphan() const { return parent_index < 0; }
	};

	std::vector<Bone> bones;
	template<class T>
	void serialize(T& archive)
	{
		archive(bones);
	}

	int64_t indexof(uint64_t unique_id) const
	{
		int64_t index{ 0 };
		for (const auto& bone : bones)
		{
			if (bone.unique_id == unique_id)
			{
				return index;
			}
			++index;
		}
		return -1;
	}
};

struct Animation
{
	std::string name;
	float sampling_rate{ 0 };

	struct Keyframe
	{
		struct Node
		{
			// 'global_transform' is used to convert from local space of node to global space of scene. 
			DirectX::XMFLOAT4X4 global_transform{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

			// The transformation data of a node includes its translation, rotation and scaling vectors with respect to its parent.
			DirectX::XMFLOAT3 scaling{ 1, 1, 1 };	
			DirectX::XMFLOAT4 rotation{ 0, 0, 0, 1 };
			DirectX::XMFLOAT3 translation{ 0, 0, 0 };	

			template<class T>
			void serialize(T& archive)
			{
				archive(global_transform, scaling, rotation, translation);
			}
		};
		std::vector<Node> nodes;

		template<class T>
		void serialize(T& archive)
		{
			archive(nodes);
		}
	};
	std::vector<Keyframe> keyframes;

	template<class T>
	void serialize(T& archive)
	{
		archive(name, sampling_rate, keyframes);
	}
};

class Skinned_Mesh
{
public :
	// Vertex structure representing a vertex in the skinned mesh
	static const int MAX_BONE_INFLUENCE{ 4 };
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal{ 0, 1, 0 };
		DirectX::XMFLOAT4 tangent{ 1, 0, 0, 1 };	
		DirectX::XMFLOAT2 texcoord{ 0, 0 };
		float bone_weights[MAX_BONE_INFLUENCE]{ 1, 0, 0, 0 };
		uint32_t bone_indices[MAX_BONE_INFLUENCE]{};

		template<class T>
		void serialize(T& archive)
		{
			archive(position, normal, tangent, texcoord, bone_weights, bone_indices);
		}
	};

	// Constant buffer structure for shader constants
	static const int MAX_BONES{ 256 };
	struct Constants
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4 material_color;
		DirectX::XMFLOAT4X4 bone_transforms[MAX_BONES]
		{
			{1,0,0,0,
			 0,1,0,0,
			 0,0,1,0,
			 0,0,0,1.}
		};
	};	

	// Mesh data
	struct Mesh
	{
		DirectX::XMFLOAT4X4 default_gobal_transform =
		{
			1,0,0,0,
			0,1,0,0,
			0,0,1,0,
			0,0,0,1.
		};
		uint64_t unique_id{ 0 };
		std::string name;
		// 'node_index' is an index that refers to the node array of the scene. 
		int64_t node_index{ 0 };

		std::vector<Vertex>vertices;
		std::vector<uint32_t> indices;

		// Subset data
		struct Subsets
		{
			uint64_t material_unique_id{ 0 };
			std::string material_name;

			uint32_t index_start{ 0 };
			uint32_t index_count{ 0 };

			template<class T>
			void serialize(T& archive)
			{
				archive(material_unique_id, material_name, index_start, index_count);
			}
		};
		std::vector<Subsets> subsets;


	private :
		Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer;
		friend class Skinned_Mesh;

		Skeleton bind_pose;

	public :

		DirectX::XMFLOAT3 bounding_box[2] =
		{
			{ +D3D11_FLOAT32_MAX, +D3D11_FLOAT32_MAX, +D3D11_FLOAT32_MAX},
			{ -D3D11_FLOAT32_MAX, -D3D11_FLOAT32_MAX, -D3D11_FLOAT32_MAX},
		};

		template<class T>
		void serialize(T& archive)
		{
			archive(unique_id, name, node_index, vertices, indices, subsets, default_gobal_transform, bind_pose, bounding_box);
		}

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

		template<class T>
		void serialize(T& archive)
		{
			archive(unique_id, name, ka, kd, ks, texture_filenames);
		}	
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

	// Fetch skeleton from the FBX mesh
	void Fetch_skeleton(FbxMesh* fbx_mesh, Skeleton& bind_pose);

	// Fetch animation data from the FBX scene. The animation data will be sampled at the specified sampling rate. 
	// If the sampling rate is 0, the animation data will be sampled at the default frame rate.
	void Fetch_Animation(FbxScene* fbx_scene, std::vector<Animation>& animations,
		float sampling_rate = 0.0f/*If this value is 0, the animation data will be sampled at the default frame rate.*/);

	// Used for updating animation keyframe data
	void Update_Animation(Animation::Keyframe& keyframe);

	// Adding in animation data from another FBX file. The animation data will be sampled at the specified sampling rate. 
	// If the sampling rate is 0, the animation data will be sampled at the default frame rate.
	bool Append_Animation(const char* animation_filename, float sampling_rate = 0);

	// Blends two animation keyframes together based on the specified blending factor. The resulting blended keyframe is stored in 'resulting_keyframe'. 
	// The blending factor should be in the range [0, 1], where 0 corresponds to the first keyframe and 1 corresponds to the second keyframe.	
	void Blend_Animation(const Animation::Keyframe* keyframes[2], float factor, Animation::Keyframe& resulting_keyframe);
	
	// Create COM objects for rendering
	void Create_com_object(const char* fbx_filename);

	// Render the skinned mesh
	void Render(const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& material_color, const Animation::Keyframe* keyframe);

	std::vector<Animation> animations; // it's animation clips ffs


protected:

	// Hold the scene data here
	scene scene_view;

	
};

struct Bone_Influence
{
	uint32_t bone_index;
	float bone_weight;
};
using Bone_Influences_Per_Control_Point = std::vector<Bone_Influence>;
