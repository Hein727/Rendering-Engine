#include "Bounding_Box.h"
#include "Graphics.h"

// Send the same position, scale and rotation as the thing you're trying to render bounding box on 
void Bounding_Box::Render_Box(StaticMesh* mesh, const DirectX::XMFLOAT3 position, const DirectX::XMFLOAT3 scale, const DirectX::XMFLOAT3 rotation)
{
	//set the rasterizer to wireframe
	graphics::getInstance().SetRasterizerState(graphics::RasterizerState::FS_OFF_CB_OFF_CW_OFF);

	//get min and max of the box / boxes depending on the mesh count
	const DirectX::XMFLOAT3 min[]{ mesh->bbox.min, box_mesh->bbox.min };
	const DirectX::XMFLOAT3 max[]{ mesh->bbox.max, box_mesh->bbox.max };

	//calculate the dimensions
	DirectX::XMFLOAT3 dimensions[] =
	{
		{max[0].x - min[0].x, max[0].y - min[0].y, max[0].z - min[0].z},
		{max[1].x - min[1].x, max[1].y - min[1].y, max[1].z - min[1].z},
	};

	//calculate the pivot point
	DirectX::XMFLOAT3 pivots[] =
	{
		{(max[0].x + min[0].x) * 0.5f, (max[0].y + min[0].y) * 0.5f, (max[0].z + min[0].z) * 0.5f},
		{(max[1].x + min[1].x) * 0.5f, (max[1].y + min[1].y) * 0.5f, (max[1].z + min[1].z) * 0.5f},
	};

	DirectX::XMFLOAT3 ratio =
	{
		dimensions[0].x / dimensions[1].x, dimensions[0].y / dimensions[1].y, dimensions[0].z / dimensions[1].z
	};

	DirectX::XMFLOAT3 offset{
		pivots[0].x - pivots[1].x, pivots[0].y - pivots[1].y, pivots[0].z - pivots[1].z
	};

	DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(ratio.x * scale.x, ratio.y * scale.y, ratio.z * scale.z) };
	DirectX::XMMATRIX O{ DirectX::XMMatrixTranslation(offset.x , offset.y, offset.z) };
	DirectX::XMMATRIX R{ DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) };
	DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(position.x, position.y, position.z) };
	DirectX::XMMATRIX W = S * O * R * T;
	DirectX::XMFLOAT4X4 world;
	DirectX::XMStoreFloat4x4(&world, W);
	
	box_mesh->render(world, {1, 1, 1, 1});

	graphics::getInstance().SetRasterizerState(graphics::RasterizerState::FS_ON_CB_OFF_CW_OFF);
}

void Bounding_Box::Render_Box(const DirectX::XMFLOAT3(&box)[2], const DirectX::XMFLOAT3 position, const DirectX::XMFLOAT3 rotation, float uniformscale)
{
	using namespace DirectX;

	//set the rasterizer to wireframe
	graphics::getInstance().SetRasterizerState(graphics::RasterizerState::FS_OFF_CB_OFF_CW_OFF);

	XMFLOAT3 center
	{
		(box[0].x + box[1].x) * 0.5f, (box[0].y + box[1].y) * 0.5f, (box[0].z + box[1].z) * 0.5f
	};

	XMFLOAT3 extents
	{
		(box[1].x - box[0].x), (box[1].y - box[0].y), (box[1].z - box[0].z)
	};

	XMMATRIX S{ XMMatrixScaling(extents.x * uniformscale, extents.y * uniformscale, extents.z * uniformscale) };
	XMMATRIX R{ XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) };
	XMMATRIX T{ XMMatrixTranslation(center.x, center.y, center.z) * XMMatrixTranslation(position.x, position.y, position.z)};

	XMMATRIX W = S * R * T;

	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, W);

	box_mesh->render(world, { 1, 1, 1, 1 });	

	graphics::getInstance().SetRasterizerState(graphics::RasterizerState::FS_ON_CB_OFF_CW_OFF);
}
