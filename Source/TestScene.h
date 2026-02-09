#pragma once

#include "Scene.h"
#include <vector>
#include <memory>
#include "System/Geometric_Primitive.h"
#include <DirectXMath.h>
#include "System/Skinned_Mesh.h"
#include "System/Bounding_Box.h"

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

	std::unique_ptr<Geometric_Primitive> primitives[8];

	DirectX::XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 translation = { 0.0f, 0.0f, 0.0f };	
	DirectX::XMFLOAT4 color{ 1, 1, 1, 1 };

	std::unique_ptr<Skinned_Mesh> skinned_meshes[8];

	std::unique_ptr<Bounding_Box> bounding_boxes[8];
};