#pragma once
#include "Scene.h"
#include <d3d11.h>
#include <memory>
#include <string>

class SceneManager
{
public :
	SceneManager() = default;

	~SceneManager() = default;

	void Update(float deltaTime);
	
	void ShadowRender(float deltaTime);

	void MainRender(float deltaTime);

	void ChangeScene(std::unique_ptr<Scene> new_scene);

	void HandleInput(std::string input);

	Scene::LightConstants& GetCurrentSceneLightConstants() { return currentScene->lightConstants; };
	
private :
	std::unique_ptr<Scene> currentScene = nullptr;
	std::unique_ptr<Scene> nextScene = nullptr;	
};