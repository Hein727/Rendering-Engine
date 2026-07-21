#pragma once
#include "SkyBox.h"
#include "../System/GameContext.h"
#include <memory>

class Environment
{
public :

	Environment(GameContext& gameContext);
	~Environment() = default;

	void Update(float deltaTime);
	void Render(float deltaTime);
	void DebugUI();	

private:

	GameContext& gameContext;	

	std::unique_ptr<SkyBox> skyBox;
};