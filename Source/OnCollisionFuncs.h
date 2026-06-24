#pragma once
#include "System/GameObject.h"

class Player : public ModelData
{
public :
	Player() = default;
	~Player() = default;
	void OnCollisionWith(const std::string& otherID) override
	{
		if (otherID == "Enemy")
		{
			isColliding = true;
		}

		if (otherID == "Cursor")
		{
		
		}
	}
};

class Enemy : public ModelData
{
public :
	Enemy() = default;
	~Enemy() = default;
	void OnCollisionWith(const std::string& otherID) override
	{
		if (otherID == "Player")
		{
			isColliding = true;
		}
	}	
};