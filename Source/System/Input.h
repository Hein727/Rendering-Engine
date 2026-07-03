#pragma once
#include "../LevelEditor/CameraControl.h"
#include "../LevelEditor/MouseControl.h"
#include "Graphics.h"	

class Input
{
public :
	Input()
		: mouseControl(cameraControls) {
	};
	~Input() = default;

	void Update(float deltaTime);

	camera_controls cameraControls;
	MouseControl mouseControl;
};