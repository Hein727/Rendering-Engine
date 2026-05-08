#include "TestScene.h"
#include "System/Graphics.h"
#include "System/Shader.h"
#include <imgui.h>


void TestScene::Init()
{
	skinned_meshes[0] = std::make_unique<Skinned_Mesh>(".\\Data\\nico.fbx");
	sprite_batches[0] = std::make_unique<SpriteBatch>(L".\\Data\\screenshot.jpg", 1);
	framebuffers[0] = std::make_unique<Framebuffer>(SCREEN_WIDTH, SCREEN_HEIGHT);
	framebuffers[1] = std::make_unique<Framebuffer>(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
	bit_block_transfer = std::make_unique<Fullscreen_Quad>();
	createPsFromCso(graphics::getInstance().GetDevice(), "Shader\\Luminance_Extraction_ps.cso", pixel_shaders[0].GetAddressOf());
}


void TestScene::Update(float deltaTime)
{

}

void TestScene::Render(float deltaTime)
{

	graphics::getInstance().SetDepthStencilState(graphics::DEPTH_DISABLED);
	graphics::getInstance().SetRasterizerState(graphics::FS_ON_CB_OFF_CW_OFF);
	
	sprite_batches[0]->Begin();
	sprite_batches[0]->Render(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	sprite_batches[0]->End();

	graphics::getInstance().SetDepthStencilState(graphics::DEPTH_MASK_ALL);
	graphics::getInstance().SetRasterizerState(graphics::FS_ON_CB_OFF_CW_OFF);

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

#if 1
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

