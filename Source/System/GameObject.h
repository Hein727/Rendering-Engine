#pragma once
#include "GltfModel.h"
#include "SceneManager.h"
#include "AssetManager.h"
#include "../LevelEditor/Grid.h"
#include <DirectXMath.h>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/unordered_map.hpp>
#include <vector>
#include <memory>
#include <unordered_map>

constexpr char editedSaveFilePath[] = { "Data\\Saves\\Edited\\" };
constexpr char runTimeSaveFilePath[] = { "Data\\Saves\\Runtime\\" };

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

enum Layer
{
	DEFAULT = 0,
	PLAYER = 1,
	ENEMY = 2,
	ENVIRONMENT = 3,
	INTERACTIVE = 4,
	LAYER_COUNT
};

struct ModelData
{
	DirectX::XMFLOAT3 translation;
	DirectX::XMFLOAT3 scale;
	DirectX::XMFLOAT3 rotation;
	Layer layer = Layer::DEFAULT;
	std::unique_ptr<AABB> aabb;
	bool hasCollider = false;
	DirectX::XMFLOAT4X4 world = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	std::weak_ptr<GltfModel> model;
	bool selected = false;

	ModelData() = default;
	ModelData(const ModelData&) = delete;            // can't copy
	ModelData& operator=(const ModelData&) = delete; // can't copy assign
	ModelData(ModelData&&) = default;                // can move
	ModelData& operator=(ModelData&&) = default;     // can move assign

	virtual ~ModelData() = default; 
	virtual void AABBvsCursor() 
	{
		selected = true;
	};

	template<class T> 	
	void serialize(T& archive)
	{
		archive(translation, scale, rotation, layer, hasCollider);
	}
};

class GameObject
{
public:
	GameObject(GameContext& gameContext, AssetManager& assetManager, const char* fileName);
	virtual ~GameObject() = default;

	void PlaceModel(const std::string ID, Grid& grid);
	void Update(float deltaTime);
	void Render(float deltaTime);
	void DebugUI();	

	void LoadModelFromFile(const std::string& filePath);

	void SaveGameState(bool isRuntimeSave = false);	

	void HandleInput(std::string input);

	// Do not call this function only for loading and saving the game state
	template<class T>
	void serialize(T& archive)
	{
		archive(dataInfos, datas, modelFilePaths);
	}

	void DeleteDataByID(const std::string& ID);
protected:
	GameContext& gameContext;
	AssetManager& assetManager;

	std::unordered_map<std::string ,int> dataInfos;
	std::vector<ModelData> datas;

	std::unordered_map<std::string, std::string> modelFilePaths;

	// Only for debug menu

	bool changeInData = false;
	std::vector<std::string> container;
	DirectX::XMFLOAT3 location;

	std::string saveFileName = " ";

public:

	std::vector<ModelData>& GetDatas() { return datas; }
};