#include "BattleScene.h"
#include "RenderingComponents/Shader.h"
#include "System/GameContext.h"
#include "RenderingComponents/Texture.h"
#include "LevelEditor/MouseControl.h"
#include "System/SceneIO.h"
#include <imgui.h>

BattleScene::BattleScene(GameContext& gameContext, SceneManager& sceneManager, AssetManager& assetManager) :
	gameContext(&gameContext), sceneManager(&sceneManager)
{
	battleObject = std::make_unique<GameObject>();

	battleObject->Init(gameContext, assetManager);

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

	ImGui::Begin(saveFileName);
	if(ImGui::Button("Save Scene"))
	{
		SaveScene();
	}
	if(ImGui::Button("Load Scene"))
	{
		LoadScene();
	}
	ImGui::End();

#endif
}

void BattleScene::HandleInput(std::string input)
{
	if (input.empty()) return;

	battleObject->HandleInput(input);
}

void BattleScene::SaveScene()
{
#if _DEBUG
	SaveSceneData(saveFileName, *this, false);
#else
	SaveSceneData(saveFileName, *this, true);
#endif
}

void BattleScene::LoadScene()
{
#if _DEBUG
	LoadSceneData(saveFileName, *this, false);
#else
	LoadSceneData(saveFileName, *this, true);
#endif
}

