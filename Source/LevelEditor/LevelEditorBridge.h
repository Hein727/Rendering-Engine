#pragma once
#include "MouseControl.h"
#include "../System/GameContext.h"
#include "../System/AssetManager.h"
#include "../System/GameObject.h"
#include "../HitCheck/CollisionManager.h"
#include "Grid.h"
#include <memory>	
#include <vector>
#include <string>
#include <list>

class LevelEditorBridge
{
public:
	LevelEditorBridge(GameContext& gameContext, AssetManager& assetManager, GameObject& gameObject, CollisionManager& collisionManager) : gameContext(gameContext), assetManager(assetManager), currentGameObject(&gameObject), collisionManager(collisionManager)
	{
		grid = std::make_unique<Grid>(gameContext);
	};

	~LevelEditorBridge() = default;

	void Update(float elapsedTime);
	void Render(float elapsedTime);
	void DebugUI();
	bool GetPlacingModel() const { return placingModel; }	

protected:
	GameContext& gameContext;
	AssetManager& assetManager;
	GameObject* currentGameObject = nullptr;
	CollisionManager& collisionManager;
	bool placingModel = false;

	std::list<std::string> loadedModelNames;
	std::string selectedModelName;
	std::unique_ptr<Grid> grid;
};