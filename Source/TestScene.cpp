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
	DirectX::XMVECTOR S = DirectX::XMLoadFloat3(&scale);
	DirectX::XMVECTOR R = DirectX::XMLoadFloat3(&rotation);
	DirectX::XMVECTOR T = DirectX::XMLoadFloat3(&translation);
	DirectX::XMMATRIX W = DirectX::XMMatrixScalingFromVector(S) * DirectX::XMMatrixRotationRollPitchYawFromVector(R) * DirectX::XMMatrixTranslationFromVector(T);
	DirectX::XMFLOAT4X4 world;
	DirectX::XMStoreFloat4x4(&world, W);
	gltfModels[0]->Render(world);
}

void TestScene::Uninit()
{

}	

void TestScene::DebugUI()
{
	ImGui::Begin("Test Scene");
	ImGui::SliderFloat3("Scale", &scale.x, 0.1f, 10.0f);
	ImGui::SliderFloat3("Rotation", &rotation.x, 0.0f, DirectX::XM_2PI);
	ImGui::SliderFloat3("Translation", &translation.x, -10.0f, 10.0f);
	ImGui::End();

}


