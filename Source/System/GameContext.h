#pragma once
#include "../RenderingComponents/Graphics.h"
#include "Input.h"
#include "Audio.h"

struct GameContext
{
public:
	graphics graphics;
	Input input;
	Audio audio;
};