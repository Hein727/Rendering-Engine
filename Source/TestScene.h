#pragma once

#include "System/Scene.h"
#include <vector>
#include <memory>
#include <DirectXMath.h>
#include "System/SceneManager.h"
#include "System/GameObject.h"
#include "System/AssetManager.h"
#include "LevelEditor/LevelEditorBridge.h"

class TestScene : public Scene
{
public:

	TestScene(GameContext& gameContext, SceneManager& sceneManager, AssetManager& assetManager);
	~TestScene() = default;

	void Init() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) override;
	void Uninit() override;
	void DebugUI() override;
	void HandleInput(std::string input) override;	

private:

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvs[8];

	std::unique_ptr<GameObject> testObject;

	GameContext* gameContext = nullptr;
	SceneManager* sceneManager = nullptr;
	
	std::unique_ptr<LevelEditorBridge> levelEditorBridge;	
};