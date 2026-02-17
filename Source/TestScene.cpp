#include "TestScene.h"
#include "System/Graphics.h"
#include <imgui.h>


void TestScene::Init()
{
	skinned_meshes[0] = std::make_unique<Skinned_Mesh>(".\\Data\\AimTest\\MNK_Mesh.fbx");
	skinned_meshes[0]->Append_Animation(".\\Data\\AimTest\\Aim_Space.fbx");
}


void TestScene::Update(float deltaTime)
{

}

void TestScene::Render(float deltaTime)
{
	DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) };
	DirectX::XMMATRIX R{ DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) };
	DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z) };
	DirectX::XMMATRIX C = graphics::getInstance().coordinate_system_transform(graphics::RHS_Z_UP, 0.01f);
	DirectX::XMMATRIX W = C * S * R * T;
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMStoreFloat4x4(&worldMatrix, W);

	int clip_index{ 0 };
	int frame_index{ 0 };
	static float animation_tick{ 0 };

#if 0
	if (!skinned_meshes[0]->animations.empty())
	{
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
		skinned_meshes[0]->Render(worldMatrix, color, &keyframe);
	}
	else
		skinned_meshes[0]->Render(worldMatrix, color, nullptr);
#else 
	Animation::Keyframe keyframe{};
	const Animation::Keyframe* keyframes[2]
	{
		&skinned_meshes[0]->animations.at(0).keyframes.at(40),
		&skinned_meshes[0]->animations.at(0).keyframes.at(80)
	};
	skinned_meshes[0]->Blend_Animation(keyframes, blend_factor, keyframe);
	skinned_meshes[0]->Update_Animation(keyframe);

	skinned_meshes[0]->Render(worldMatrix, color, &keyframe);
#endif
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
	ImGui::Separator();
	ImGui::SliderFloat("Blend Factor", &blend_factor, 0.0f, 1.0f);	
	ImGui::End();
}

