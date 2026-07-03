#include "TestScene.h"
#include "System/Shader.h"
#include "System/GameContext.h"
#include "System/Texture.h"
#include "LevelEditor/MouseControl.h"
#include <imgui.h>

TestScene::TestScene(GameContext& gameContext, SceneManager& sceneManager, AssetManager& assetManager) :
	gameContext(&gameContext), sceneManager(&sceneManager)
{
	testObject = std::make_unique<GameObject>(gameContext, assetManager);

	collisionManager = std::make_unique<CollisionManager>(gameContext,testObject->GetDatas());

#ifdef _DEBUG

	levelEditorBridge = std::make_unique<LevelEditorBridge>(gameContext, assetManager, *testObject);

#endif
}

void TestScene::Init()
{
	auto device = gameContext->graphics.GetDevice();

	D3D11_TEXTURE2D_DESC desc{};
	desc.MipLevels = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	loadTextureFromFile(device, L"Data/environments/test texture/skybox.dds", srvs[0].GetAddressOf(), &desc);
	loadTextureFromFile(device, L"Data/environments/test texture/diffuse_iem.dds", srvs[1].GetAddressOf(), &desc);
	loadTextureFromFile(device, L"Data/environments/test texture/specular_pmrem.dds", srvs[2].GetAddressOf(), &desc);
	loadTextureFromFile(device, L"Data/environments/test texture/lut_ggx.dds", srvs[3].GetAddressOf(), &desc);
}

void TestScene::Update(float deltaTime)
{
	testObject->Update(deltaTime);

#ifdef _DEBUG

	levelEditorBridge->Update(deltaTime);

#endif
}

void TestScene::Render(float deltaTime)
{	
	auto context = gameContext->graphics.GetDeviceContext();

	context->PSSetShaderResources(32, 1, srvs[0].GetAddressOf());
	context->PSSetShaderResources(33, 1, srvs[1].GetAddressOf());
	context->PSSetShaderResources(34, 1, srvs[2].GetAddressOf());
	context->PSSetShaderResources(35, 1, srvs[3].GetAddressOf());

	//static std::vector<GltfModel::Node> animatedNodes{ gltfModels[0]->nodes };
	//static float time{ 0 };
	//gltfModels[0]->Animate(0, time += deltaTime, animatedNodes);
	///*if (gltfModels[0]->animations.at(0).duration < time)
	//{
	//	time = 0;
	//}*/
	//gltfModels[0]->Render(world[0], animatedNodes);

	//if (gltfModels[0]->useAABB)
	//{
	//	gltfModels[0]->aabb->Render(deltaTime);
	//}

	testObject->Render(deltaTime);

#ifdef _DEBUG

	levelEditorBridge->Render(deltaTime);
	

#endif
}

void TestScene::Uninit()
{
	
}	

void TestScene::DebugUI()
{
	testObject->DebugUI();

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

void TestScene::HandleInput(std::string input)
{
	if (input.empty()) return;
	
	testObject->HandleInput(input);
}

