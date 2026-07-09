#pragma once
#include <vector>
#include "../System/GameObject.h"
#include "../System/GameContext.h"

class CollisionManager
{
public:
	CollisionManager(GameContext& gameContext, std::vector<ModelData>& datas) : gameContext(gameContext), datas(&datas)
	{
		for(int i = 0; i < (int)Layer::LAYER_COUNT; ++i)
		{
			for(int j = 0; j < (int)Layer::LAYER_COUNT; ++j)
			{
				collisionMatrix[i][j] = false;
			}
		}	
	};

	~CollisionManager() = default;

	void AddNewCollisionPair(Layer object1, Layer object2)
	{
		collisionMatrix[(int)object1][(int)object2] = true;	
	}

	void CheckCursorWithModel();

	void UpdateDatas(std::vector<ModelData>& datas)
	{
		this->datas = &datas;
	}

private :

	bool collisionMatrix[(int)Layer::LAYER_COUNT][(int)Layer::LAYER_COUNT];

	GameContext& gameContext;

	std::vector<ModelData>* datas = nullptr;
};