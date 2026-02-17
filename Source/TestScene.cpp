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

void TestScene::Render(float deltaTime)
{
	DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) };
	DirectX::XMMATRIX R{ DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) };
	DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z) };
	DirectX::XMMATRIX C = graphics::getInstance().coordinate_system_transform(graphics::RHS_Y_UP, 0.01f);
	DirectX::XMMATRIX W = C * S * R * T;
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMStoreFloat4x4(&worldMatrix, W);

	int clip_index{ 0 };
	int frame_index{ 0 };
	static float animation_tick{ 0 };

	Animation& animation{ skinned_meshes[0]->animations.at(clip_index) };
	frame_index = static_cast<int>(animation_tick * animation.sampling_rate);
	if (frame_index > animation.keyframes.size() - 1)
	{
		frame_index = 0;
		animation_tick = 0;
	}
	else
	{
		animation_tick += deltaTime;
	}
	Animation::Keyframe& keyframe{ animation.keyframes.at(frame_index) };
#if 1 
	DirectX::XMStoreFloat4(&keyframe.nodes.at(30).rotation, DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(1, 0, 0, 0), 1.5f));
	keyframe.nodes.at(30).translation.x = 300.0f;	
	skinned_meshes[0]->Update_Animation(keyframe);
#endif
	skinned_meshes[0]->Render(worldMatrix, color, &keyframe);
}

void TestScene::Uninit()
{

}	

void TestScene::DebugUI()
{
	ImGui::Begin("Test Scene");
	ImGui::SliderFloat3("Scaling", &scale.x, 1.0f, 10.0f);
	ImGui::SliderFloat3("Rotation", &rotation.x, -DirectX::XM_PI, DirectX::XM_PI);	
	ImGui::SliderFloat3("Translation", &translation.x, -10.0f, 10.0f);
	ImGui::Separator();
	ImGui::ColorPicker4("Color", &color.x);
	ImGui::End();
}

