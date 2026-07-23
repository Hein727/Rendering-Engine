#include "TestScene.h"
#include "RenderingComponents/Shader.h"
#include "System/GameContext.h"
#include "RenderingComponents/Texture.h"
#include "LevelEditor/MouseControl.h"
#include "RenderingComponents/ShadingSetup.h"
#include "System/SceneIO.h"
#include <imgui.h>

TestScene::TestScene(GameContext& gameContext, SceneManager& sceneManager, AssetManager& assetManager) :
	gameContext(&gameContext), sceneManager(&sceneManager), assetManager(&assetManager)
{
	testObject = std::make_unique<GameObject>();

	testObject->Init(gameContext, assetManager);

	collisionManager = std::make_unique<CollisionManager>(gameContext, testObject->GetDatas());

	environment = std::make_unique<Environment>();	

#ifdef _DEBUG

	levelEditorBridge = std::make_unique<LevelEditorBridge>(gameContext, assetManager, *testObject, *collisionManager);

#endif
}

void TestScene::Init()
{
	auto device = gameContext->graphics.GetDevice();

	// Setting up sun
	lightConstants.directionalLights.direction = { 0.3f, -1.0f, 0.2f, 0.0f };
	lightConstants.directionalLights.color = { 1.0f, 1.0f, 1.0f, 1.0f };

	environment->Init(*gameContext, *testObject, lightConstants, lightConstantBuffer);

	/*D3D11_TEXTURE2D_DESC desc{};
	desc.MipLevels = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;*/
	//already deleted the textures if in need download new environment textures 
	//loadTextureFromFile(device, L"Data/environments/test texture/skybox.dds", srvs[0].GetAddressOf(), &desc);
	//loadTextureFromFile(device, L"Data/environments/test texture/diffuse_iem.dds", srvs[1].GetAddressOf(), &desc);
	//loadTextureFromFile(device, L"Data/environments/test texture/specular_pmrem.dds", srvs[2].GetAddressOf(), &desc);
	//loadTextureFromFile(device, L"Data/environments/test texture/lut_ggx.dds", srvs[3].GetAddressOf(), &desc);
}

void TestScene::Update(float deltaTime)
{
	testObject->Update(deltaTime);

	environment->CaptureEnvironmentProbes(deltaTime);

#ifdef _DEBUG

	levelEditorBridge->Update(deltaTime);

#endif
}

void TestScene::ShadowRender(float deltaTime)
{
	int lightShaderSlot = 13;
	UpdateConstantBuffer(*gameContext, lightConstantBuffer, lightConstants, lightShaderSlot);

	environment->ShadowRender(deltaTime);
}

void TestScene::MainRender(float deltaTime)
{
	environment->Render(deltaTime);

	testObject->Render(deltaTime);

#ifdef _DEBUG

	levelEditorBridge->Render(deltaTime);
	
#endif
}

//auto context = gameContext->graphics.GetDeviceContext();

	/*context->PSSetShaderResources(32, 1, srvs[0].GetAddressOf());
	context->PSSetShaderResources(33, 1, srvs[1].GetAddressOf());
	context->PSSetShaderResources(34, 1, srvs[2].GetAddressOf());
	context->PSSetShaderResources(35, 1, srvs[3].GetAddressOf());*/

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

void TestScene::Uninit()
{
	
}	

void TestScene::DebugUI()
{
	testObject->DebugUI();

#ifdef _DEBUG

	levelEditorBridge->DebugUI();

	ImGui::Begin(saveFileName);
	if (ImGui::CollapsingHeader("lights"))
	{
		ImGui::ColorEdit3("ambientColor", &lightConstants.ambientColor.x);
		ImGui::DragFloat4("directionalLights.direction", &lightConstants.directionalLights.direction.x, 0.01f, -1.0f, 1.0f);
		ImGui::ColorEdit3("directionalLights.color", &lightConstants.directionalLights.color.x);
	}
	if (ImGui::CollapsingHeader("Scene Save/Load"))
	{
		if (ImGui::Button("Save Scene"))
		{
			SaveScene();
		}
		if (ImGui::Button("Load Scene"))
		{
			LoadScene();
		}
	}
	ImGui::End();

#endif

	environment->DebugUI();
	
}

void TestScene::HandleInput(std::string input)
{
	if (input.empty()) return;
	
	testObject->HandleInput(input);
}

void TestScene::SaveScene()
{
#if _DEBUG
	SaveSceneData(saveFileName, *this, false);
#else
	SaveSceneData(saveFileName, *this, true);
#endif
}

void TestScene::LoadScene()
{
#if _DEBUG
	LoadSceneData(saveFileName, *this, false);
#else
	LoadSceneData(saveFileName, *this, true);
#endif
	testObject->RestoreRuntimeData(*gameContext, *assetManager);

	collisionManager->UpdateDatas(testObject->GetDatas());
}

