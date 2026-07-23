#include "LevelEditorBridge.h"
#include <filesystem>
#include <imgui.h>
#include <string>
#include <list>

void LevelEditorBridge::Update(float elapsedTime)
{

}	

void LevelEditorBridge::Render(float elapsedTime)
{
	if(renderGrid)
		grid->Render(elapsedTime);
}

void LevelEditorBridge::DebugUI()
{
	ImGui::Begin("Level Editor");
	ImGui::Text("This is the level editor bridge.");
	ImGui::Text("PS: only use gltf models ffs");
	POINT cursor = gameContext.input.cameraControls.get_cursor_position();
	ImGui::Text("Cursor Position: (%f, %f)", static_cast<float>(cursor.x), static_cast<float>(cursor.y));
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

	ImGui::Checkbox("Render Grid", &renderGrid);

	ImGui::Text("IT DOESN'T AUTOSAVE");

	ImGui::End();

	auto& io = ImGui::GetIO();

	if (!io.WantCaptureMouse && !placingModel)
	{
		if (gameContext.input.mouseControl.GetLeftButtonPressed() && currentGameObject->GetSelectedData() == nullptr)
		{
			collisionManager.CheckCursorWithModel();
		}
	}

	if (!io.WantCaptureMouse && placingModel)
	{
		if (gameContext.input.mouseControl.GetLeftButtonPressed())
		{
			currentGameObject->PlaceModel(selectedModelName, *grid);
		}
	}
}	
