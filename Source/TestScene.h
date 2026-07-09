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
#include "System/Serializer.h"	

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
	std::string GetFileName() { return testSceneSaveFileName; }
	void SaveScene();
	void LoadScene();

	template<class T>
	void serialize(T& archive)
	{
		archive(*testObject, lightConstants);
	}

private:

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srvs[8];

	std::unique_ptr<GameObject> testObject;

	GameContext* gameContext = nullptr;
	SceneManager* sceneManager = nullptr;
	AssetManager* assetManager = nullptr;
	
	std::unique_ptr<LevelEditorBridge> levelEditorBridge;

	std::unique_ptr<CollisionManager> collisionManager;
	static constexpr const char* testSceneSaveFileName = "TestScene";
};