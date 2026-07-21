#pragma once
#include "../RenderingComponents/GltfModel.h"
#include "SceneManager.h"
#include "AssetManager.h"
#include "../LevelEditor/Grid.h"
#include <DirectXMath.h>
#include <vector>
#include <memory>
#include <unordered_map>
#include "Serializer.h"

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
protected:
	DirectX::XMFLOAT3 translation;
	DirectX::XMFLOAT3 scale;
	DirectX::XMFLOAT3 rotation;
	Layer layer = Layer::DEFAULT;
	std::unique_ptr<AABB> aabb;
	bool hasCollider = false;
	bool hasShadow = true;
	DirectX::XMFLOAT4X4 world = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	std::weak_ptr<GltfModel> model;
	bool selected = false;

	friend class GameObject;

public:
	ModelData() = default;
	ModelData(const ModelData&) = delete;            // can't copy
	ModelData& operator=(const ModelData&) = delete; // can't copy assign
	ModelData(ModelData&&) = default;                // can move
	ModelData& operator=(ModelData&&) = default;     // can move assign

	virtual ~ModelData() = default;

	// Accessors
	bool IsSelected() const { return selected; }
	AABB* GetAABB() const { return aabb.get(); }

	virtual void AABBvsCursor() 
	{
		selected = true;
	};

	virtual void AABBvsCursorRelease()
	{
		selected = false;
	}

	template<class T> 	
	void serialize(T& archive)
	{
		archive(translation, scale, rotation, layer, hasCollider);
	}
};

class GameObject
{
public:
	GameObject() = default;

	virtual ~GameObject() = default;
	void Init(GameContext& gameContext, AssetManager& assetManager);
	void RestoreRuntimeData(GameContext& gameContext, AssetManager& assetManager);

	void PlaceModel(const std::string ID, Grid& grid);
	void Update(float deltaTime);
	void ShadowMapRender(float deltaTime);
	void Render(float deltaTime);
	void DebugUI();	

	void LoadModelFromFile(const std::string& filePath);

	void HandleInput(std::string input);

	// Do not call this function only for loading and saving the game state
	template<class T>
	void serialize(T& archive)
	{
		archive(dataInfos, datas, modelFilePaths);
	}

	void DeleteDataByID(const std::string& ID);
protected:
	GameContext* gameContext = nullptr;
	AssetManager* assetManager = nullptr;

	std::unordered_map<std::string ,int> dataInfos;
	std::vector<ModelData> datas;

	std::unordered_map<std::string, std::string> modelFilePaths;

	// Only for debug menu

	bool changeInData = false;
	std::vector<std::string> container;
	DirectX::XMFLOAT3 location{ 0, 0, 0 };

	std::string saveFileName = " ";

	ModelData* selected = nullptr;

	int selectedDataIndex = -1;

public:

	std::vector<ModelData>& GetDatas() { return datas; }

	ModelData* GetSelectedData() { return selected; }
};