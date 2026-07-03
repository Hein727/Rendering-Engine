#include "AABB.h"

AABB::AABB(GameContext& gameContext) : gameContext(gameContext)
{

	boxMesh = std::make_unique<StaticMesh>(gameContext, L"./Data/cube.obj");
}

void AABB::SetMinMax(DirectX::XMFLOAT3& min, DirectX::XMFLOAT3& max)
{
	model = std::make_unique<Box>();
	model->min = min;
	model->max = max;

#ifdef _DEBUG

	renderBoundingBox = true;

#endif
}

void AABB::Update(DirectX::XMFLOAT3& pos, DirectX::XMFLOAT3& scale, float uniformScaling)
{
	DirectX::XMFLOAT3 extents
	{
		(model->max.x - model->min.x) * 0.5f, (model->max.y - model->min.y) * 0.5f, (model->max.z - model->min.z) * 0.5f
	};

	DirectX::XMFLOAT3 scaledExtents
	{
		extents.x * scale.x, extents.y * scale.y, extents.z * scale.z
	};

	collider = std::make_unique<DirectX::BoundingBox>(DirectX::XMFLOAT3{ pos.x, pos.y, pos.z },
		DirectX::XMFLOAT3{ scaledExtents.x * uniformScaling, scaledExtents.y * uniformScaling, scaledExtents.z * uniformScaling });

	DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(scaledExtents.x * uniformScaling, scaledExtents.y * uniformScaling, scaledExtents.z * uniformScaling) };
	DirectX::XMMATRIX R{ DirectX::XMMatrixRotationRollPitchYaw(0, 0, 0) };
	DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z) };

	DirectX::XMMATRIX W = S * R * T;

	XMStoreFloat4x4(&world, W);
}

void AABB::Render(float elapsedTime)
{
	//set the rasterizer to wireframe
	auto& graphics = gameContext.graphics;

	graphics.SetRasterizerState(graphics::RasterizerState::FS_OFF_CB_OFF_CW_OFF);

	boxMesh->render(world, { 1, 1, 1, 1 });

	graphics.SetRasterizerState(graphics::RasterizerState::FS_ON_CB_OFF_CW_OFF);
}