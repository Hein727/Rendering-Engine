#pragma once

#include "Scene.h"
#include <vector>
#include <memory>
#include <DirectXMath.h>
#include "System/GltfModel.h"

class TestScene : public Scene
{
public:

	TestScene() {};
	~TestScene() {};

	void Init() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) override;
	void Uninit() override;
	void DebugUI() override;

private :

	DirectX::XMFLOAT3 scale = { 0.1f, 0.1f, 0.1f };
	DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 translation = { 0.0f, 0.0f, 0.0f };	
	DirectX::XMFLOAT4 color{ 1, 1, 1, 1 };

	std::unique_ptr<GltfModel> gltfModels[8];
};