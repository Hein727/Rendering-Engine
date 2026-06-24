#pragma once
#include <vector>
#include "../System/GameObject.h"

class CollisionManager
{
public:
	CollisionManager(std::unordered_map<std::string, ModelData>& modelData) : modelDatas(modelData)
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

	void Update(float deltaTime);
	void AddNewCollisionPair(Layer object1, Layer object2)
	{
		collisionMatrix[(int)object1][(int)object2] = true;	
	}

private :

	bool collisionMatrix[(int)Layer::LAYER_COUNT][(int)Layer::LAYER_COUNT];

	std::unordered_map<std::string, ModelData>& modelDatas;
};