#pragma once
#include "MouseControl.h"
#include "../System/GameContext.h"
#include "../System/AssetManager.h"
#include "../System/GameObject.h"
#include "Grid.h"
#include <memory>	
#include <vector>
#include <string>
#include <list>

class LevelEditorBridge
{
public:
	LevelEditorBridge(GameContext& gameContext, AssetManager& assetManager, GameObject& gameObject) : gameContext(gameContext), assetManager(assetManager), currentGameObject(gameObject) 
	{
		grid = std::make_unique<Grid>(gameContext);
	};
	~LevelEditorBridge() = default;

	void Update(float elapsedTime);
	void Render(float elapsedTime);
	void DebugUI();

protected:
	GameContext& gameContext;
	AssetManager& assetManager;
	GameObject& currentGameObject;	
	bool placingModel = false;

	std::list<std::string> loadedModelNames;
	std::string selectedModelName;
	std::unique_ptr<Grid> grid;
};