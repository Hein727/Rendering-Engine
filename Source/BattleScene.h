#pragma once

#include "System/Scene.h"
#include <vector>
#include <memory>
#include <DirectXMath.h>
#include "System/SceneManager.h"
#include "System/GameObject.h"
#include "System/AssetManager.h"
#include "LevelEditor/LevelEditorBridge.h"
#include "HitCheck/CollisionManager.h"	

constexpr const char* battleSceneSaveFileName = "BattleScene";	

class BattleScene : public Scene
{
public:

	BattleScene(GameContext& gameContext, SceneManager& sceneManager, AssetManager& assetManager);
	~BattleScene() = default;

	void Init() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) override;
	void Uninit() override;
	void DebugUI() override;
	void HandleInput(std::string input) override;

private:

	std::unique_ptr<GameObject> battleObject;

	GameContext* gameContext = nullptr;
	SceneManager* sceneManager = nullptr;

	std::unique_ptr<LevelEditorBridge> levelEditorBridge;

	std::unique_ptr<CollisionManager> collisionManager;
};
