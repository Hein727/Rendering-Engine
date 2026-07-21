#include "SceneManager.h"

void SceneManager::Update(float deltaTime)
{
	if (nextScene != nullptr)
	{
		currentScene->Uninit();
		
		currentScene = std::move(nextScene);
		currentScene->Init();

		nextScene = nullptr;
	}

	if (currentScene != nullptr)
	{
		currentScene->Update(deltaTime);
	}
}

void SceneManager::ShadowRender(float deltaTime)
{
	if (currentScene != nullptr)
	{
		currentScene->ShadowRender(deltaTime);
	}
}	

void SceneManager::MainRender(float deltaTime)
{
	if (currentScene != nullptr)
	{
		currentScene->MainRender(deltaTime);

#ifdef _DEBUG

		currentScene->DebugUI();

#endif
	}
}

void SceneManager::ChangeScene(std::unique_ptr<Scene> new_scene)
{
	if (!currentScene)
	{
		currentScene = std::move(new_scene);
		currentScene->Init();
		currentScene->LoadScene();
		return;
	}

	nextScene = std::move(new_scene);
}

void SceneManager::HandleInput(std::string input)
{
	if (input.empty()) return;

	if (currentScene != nullptr)
	{
		currentScene->HandleInput(input);
	}
}