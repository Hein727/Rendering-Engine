#include "BattleScene.h"
#include "System/Shader.h"
#include "System/GameContext.h"
#include "System/Texture.h"
#include "LevelEditor/MouseControl.h"
#include <imgui.h>

BattleScene::BattleScene(GameContext& gameContext, SceneManager& sceneManager, AssetManager& assetManager) :
	gameContext(&gameContext), sceneManager(&sceneManager)
{
	battleObject = std::make_unique<GameObject>(gameContext, assetManager);

	collisionManager = std::make_unique<CollisionManager>(gameContext, battleObject->GetDatas());

#ifdef _DEBUG

	levelEditorBridge = std::make_unique<LevelEditorBridge>(gameContext, assetManager, *battleObject);

#endif
}

void BattleScene::Init()
{
	auto device = gameContext->graphics.GetDevice();
}

void BattleScene::Update(float deltaTime)
{
	battleObject->Update(deltaTime);

#ifdef _DEBUG

	levelEditorBridge->Update(deltaTime);

#endif
}

void BattleScene::Render(float deltaTime)
{
	auto context = gameContext->graphics.GetDeviceContext();

	battleObject->Render(deltaTime);

#ifdef _DEBUG

	levelEditorBridge->Render(deltaTime);


#endif
}

void BattleScene::Uninit()
{

}

void BattleScene::DebugUI()
{
	battleObject->DebugUI();

#ifdef _DEBUG

	levelEditorBridge->DebugUI();
	auto& io = ImGui::GetIO();

	if (!io.WantCaptureMouse && !levelEditorBridge->GetPlacingModel())
	{
		if (gameContext->input.mouseControl.GetMouseLeftClick())
		{
			collisionManager->CheckCursorWithModel();
		}
	}

#endif
}

void BattleScene::HandleInput(std::string input)
{
	if (input.empty()) return;

	battleObject->HandleInput(input);
}

