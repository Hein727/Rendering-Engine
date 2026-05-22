#include "TestScene.h"
#include "System/Graphics.h"
#include "System/Shader.h"
#include <imgui.h>

void TestScene::Init()
{
	gltfModels[0] = std::make_unique<GltfModel>("Data\\glTF-Sample-Models-main\\2.0\\2CylinderEngine\\glTF\\2CylinderEngine.gltf");
}

void TestScene::Update(float deltaTime)
{

}

void TestScene::Render(float deltaTime)
{

}

void TestScene::Uninit()
{

}	

void TestScene::DebugUI()
{
	ImGui::Begin("Test Scene");
	
	ImGui::End();

}


