#pragma once
#include <DirectXMath.h>
#include <memory>
#include "../RenderingComponents/Static_Mesh.h"
#include "../System/GameContext.h"
#include <DirectXCollision.h>

class AABB// Axis-Aligned Bounding Box 
{
public:
	AABB(GameContext& gameContext);
	void SetMinMax(DirectX::XMFLOAT3& min, DirectX::XMFLOAT3& max);
	void Update(DirectX::XMFLOAT3& pos, DirectX::XMFLOAT3& scale, float uniformScaling = 1.0f);
	void Render(float elapsedTime);
	~AABB() = default;

	bool GetCollisionWith(const DirectX::BoundingBox& other) const
	{
		return collider->Intersects(other);
	};

	DirectX::BoundingBox GetCollider() const
	{
		return *collider;
	}

	bool renderBoundingBox = false;

private:
	struct Box
	{
		DirectX::XMFLOAT3 min{ FLT_EPSILON, FLT_EPSILON, FLT_EPSILON };
		DirectX::XMFLOAT3 max{ -FLT_EPSILON, -FLT_EPSILON, -FLT_EPSILON };
	};
	std::unique_ptr<Box> model;

	std::unique_ptr<DirectX::BoundingBox> collider;

	DirectX::XMFLOAT4X4 world;
	std::unique_ptr<StaticMesh> boxMesh;

	GameContext& gameContext;
};