#include "CollisionManager.h"

void CollisionManager::Update(float deltaTime)
{
	for (auto& [id, modelData1] : modelDatas)
	{
		if (!modelData1.hasCollider)
			continue;

		for (auto& [otherID, modelData2] : modelDatas)
		{
			if (id == otherID)
				continue;

			if (!modelData2.hasCollider)
				continue;

			if (!collisionMatrix[(int)modelData1.layer][(int)modelData2.layer])
				continue;

			auto model1 = modelData1.model.lock();
			auto model2 = modelData2.model.lock();

			if (model1 || model2)
			{
				modelData1.OnCollisionWith(otherID);
				modelData2.OnCollisionWith(id);
			}
		}
	}
}