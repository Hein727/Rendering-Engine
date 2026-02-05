#pragma once
#include "Scene.h"
#include <d3d11.h>
#include <memory>

class SceneManager
{
private:
	SceneManager() {};

public :
	static SceneManager& GetInstance()
	{
		static SceneManager instance;
		return instance;
	}

	~SceneManager() {};

	void Update(float deltaTime);
	
	void Render();

	void ChangeScene(std::unique_ptr<Scene> new_scene);
	
private :
	std::unique_ptr<Scene> currentScene = nullptr;
	std::unique_ptr<Scene> nextScene = nullptr;
};