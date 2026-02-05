#include "TestScene.h"
#include "System/Graphics.h"
#include <imgui.h>


void TestScene::Init()
{
	skinned_meshes[0] = std::make_unique<Skinned_Mesh>(".\\Data\\plantune.fbx");
}


void TestScene::Update(float deltaTime)
{

}

void TestScene::Render()
{
	DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) };
	DirectX::XMMATRIX R{ DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) };
	DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z) };
	DirectX::XMMATRIX W = S * R * T;
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMStoreFloat4x4(&worldMatrix, W);

}

void TestScene::Uninit()
{

}	

void TestScene::DebugUI()
{
	ImGui::Begin("Test Scene");
	ImGui::SliderFloat3("Scaling", &scale.x, 1.0f, 10.0f);
	ImGui::SliderFloat3("Rotation", &rotation.x, 0.0f, DirectX::XM_2PI);	
	ImGui::SliderFloat3("Translation", &translation.x, -10.0f, 10.0f);
	ImGui::Separator();
	ImGui::ColorPicker4("Color", &color.x);
	ImGui::End();
}