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

void SceneManager::Render()
{
	if (currentScene != nullptr)
	{
		currentScene->Render();

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
		return;
	}

	nextScene = std::move(new_scene);
}