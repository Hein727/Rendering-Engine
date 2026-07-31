#pragma once
#include "../RenderingComponents/Graphics.h"
#include "Input.h"
#include "Audio.h"
#include "FrustumCulling.h"

struct GameContext
{
public:
	graphics graphics;
	Input input;
	Audio audio;
	FrustumCulling frustumCulling{ input.cameraControls };
};