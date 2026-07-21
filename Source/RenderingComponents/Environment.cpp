#include "Environment.h"

Environment::Environment(GameContext& gameContext) : gameContext(gameContext)
{
	skyBox = std::make_unique<SkyBox>(gameContext, L"./Data/Skybox/overcast_soil_puresky_4k.hdr");
}

void Environment::Update(float deltaTime)
{
	// Update environment-related logic here
}

void Environment::Render(float deltaTime)
{
	skyBox->Render(deltaTime);
}