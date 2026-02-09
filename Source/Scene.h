#pragma once
#include <d3d11.h>

class Scene
{
public :

	Scene() = default;
	virtual ~Scene() = default;
	virtual void Init() = 0;
	virtual void Update(float deltaTime) = 0;	
	virtual void Render(float deltaTime) = 0;
	virtual void Uninit() = 0;
	virtual void DebugUI() = 0; // Renders Imgui for debugging purposes
};