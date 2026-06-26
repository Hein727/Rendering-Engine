#include "LevelEditorBridge.h"
#include <filesystem>
#include <imgui.h>
#include <string>
#include <list>

void LevelEditorBridge::Update(float elapsedTime)
{
	grid->Update(elapsedTime);
}	

void LevelEditorBridge::Render(float elapsedTime)
{
	grid->Render(elapsedTime);
}

void LevelEditorBridge::DebugUI()
{
	ImGui::Begin("Level Editor");
	ImGui::Text("This is the level editor bridge.");
	ImGui::Text("PS: only use gltf models ffs");
	if (ImGui::CollapsingHeader("Model Loader"))
	{
		std::list<std::string> modelNames = assetManager.GetAllModelNames();
		if (ImGui::BeginListBox("Loaded Models"))
		{
			for (const auto& name : modelNames)
			{
				if (ImGui::Selectable(name.c_str()))
				{
					selectedModelName = name;
				}
			}
			ImGui::EndListBox();
		}
	}
	ImGui::Text("Selected Model: %s", selectedModelName.c_str());

	ImGui::Checkbox("Placing Model", &placingModel);

	if (ImGui::Button("Save Game State"))
	{
		currentGameObject.SaveGameState();
	}

	ImGui::Text("IT DOESN'T AUTOSAVE");

	ImGui::End();

	auto io = ImGui::GetIO();

	if (!io.WantCaptureMouse && placingModel)
	{
		if (gameContext.input.mouseControl.GetMouseLeftClick())
		{
			currentGameObject.PlaceModel(selectedModelName);
		}
	}
}	