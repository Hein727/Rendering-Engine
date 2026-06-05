#include "TestScene.h"
#include "System/Graphics.h"
#include "System/Shader.h"
#include "System/Texture.h"
#include <imgui.h>

void TestScene::Init()
{
	gltfModels[0] = std::make_unique<GltfModel>("Data\\glTF-Sample-Models-main\\2.0\\DamagedHelmet\\glTF\\DamagedHelmet.gltf");

	auto device = graphics::getInstance().GetDevice();

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
	
}

void TestScene::Render(float deltaTime)
{	
	auto context = graphics::getInstance().GetDeviceContext();

	context->PSSetShaderResources(32, 1, srvs[0].GetAddressOf());
	context->PSSetShaderResources(33, 1, srvs[1].GetAddressOf());
	context->PSSetShaderResources(34, 1, srvs[2].GetAddressOf());
	context->PSSetShaderResources(35, 1, srvs[3].GetAddressOf());

	DirectX::XMVECTOR S = DirectX::XMLoadFloat3(&scale);
	DirectX::XMVECTOR R = DirectX::XMLoadFloat3(&rotation);
	DirectX::XMVECTOR T = DirectX::XMLoadFloat3(&translation);
	DirectX::XMMATRIX W = DirectX::XMMatrixScalingFromVector(S) * DirectX::XMMatrixRotationRollPitchYawFromVector(R) * DirectX::XMMatrixTranslationFromVector(T);
	DirectX::XMFLOAT4X4 world;
	DirectX::XMStoreFloat4x4(&world, W);

	static std::vector<GltfModel::Node> animatedNodes{ gltfModels[0]->nodes };
	static float time{ 0 };
	gltfModels[0]->Animate(0, time += deltaTime, animatedNodes);
	/*if (gltfModels[0]->animations.at(0).duration < time)
	{
		time = 0;
	}*/
	gltfModels[0]->Render(world, animatedNodes);
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


