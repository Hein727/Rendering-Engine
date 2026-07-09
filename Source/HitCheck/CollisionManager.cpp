#include "CollisionManager.h"
#include <DirectXCollision.h>

void CollisionManager::CheckCursorWithModel()
{
	gameContext.input.mouseControl.UpdateMouseRay(gameContext.graphics);

	using namespace DirectX;

	XMFLOAT3 origin = gameContext.input.mouseControl.GetMouseOrigin();
	XMVECTOR Origin = XMLoadFloat3(&origin);	
	XMFLOAT3 dir = gameContext.input.mouseControl.GetMouseDir();
	XMVECTOR Dir = XMLoadFloat3(&dir);	
	float distance = 10.0f;

	if (datas->empty())
		return;

	for (auto& data : *datas)
	{
		if (XMVector3Equal(Origin, Dir)) break;

		if (data.aabb)
		{
			if(data.aabb->GetCollider().Intersects(Origin, Dir, distance))
			{
				data.AABBvsCursor();
				break;
			}
		}
	}
}