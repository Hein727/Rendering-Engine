#include "GameObject.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "../Imgui/src/ImGuizmo.h"
#include "Scene.h"
#include <filesystem>
#include <fstream>
#include <sstream>

void GameObject::Init(GameContext& gameContext, AssetManager& assetManager)
{
	this->gameContext = &gameContext;
	this->assetManager = &assetManager;
}

void GameObject::RestoreRuntimeData(GameContext& gameContext, AssetManager& assetManager)
{
	this->gameContext = &gameContext;
	this->assetManager = &assetManager;

	for (auto& [fileName, filePath] : modelFilePaths)
	{
		assetManager.loadModel(gameContext,
			filePath);
	}

	for (auto& [id, index] : dataInfos)
	{
		if (modelFilePaths.empty())
			break;

		datas.resize(dataInfos.size());

		std::string ID = id;
		ID = ID.substr(0, ID.find_last_of('_'));

		auto& data = datas[index];
		data.aabb = std::make_unique<AABB>(gameContext);

		auto model = assetManager.GetModel(gameContext, ID, data.aabb);

		if (!model)
		{
			model = assetManager.GetModel(gameContext, ID, data.aabb);

			if (model)
			{
				data.model = model;
			}
		}
		else
		{
			data.model = model;
		}
	}
	changeInData = true;
}

void GameObject::LoadModelFromFile(const std::string& filePath)
{
	std::filesystem::path path(filePath);
	std::string modelName;
	modelName = path.filename().string().c_str();	
	if (assetManager->GetModel(modelName) == nullptr)
	{
		assetManager->loadModel(*gameContext, filePath); // èCê≥: *gameContext
	}
}

void GameObject::PlaceModel(const std::string ID, Grid& grid)
{
	ModelData modelData;
	DirectX::XMMATRIX M = DirectX::XMMatrixIdentity();
	modelData.aabb = std::make_unique<AABB>(*gameContext);	
	location = grid.GetCurrentCellLocation();
	DirectX::XMFLOAT3 translation = location;
	modelData.translation = translation;
	modelData.scale = { 1, 1, 1 };
	modelData.rotation = { 0, 0, 0 };
	modelData.world = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		location.x, location.y, location.z, 1
	};
	modelData.hasCollider = true;

	auto it = assetManager->GetModel(*gameContext, ID, modelData.aabb);

	if (it != nullptr)
	{
		auto model = assetManager->GetModel(*gameContext, ID, modelData.aabb);
		modelData.model = model;
		std::string newID = ID;
		int count = 1;
		while (dataInfos.contains(newID))
		{
			newID = ID + "_" + std::to_string(count);
			count++;
		}

		datas.push_back(std::move(modelData));

		int index = static_cast<int>(datas.size()) - 1;	

		dataInfos.emplace(newID, index);

		changeInData = true;
	}
}

void GameObject::Update(float deltaTime)
{	
	for (auto& data : datas)
	{
		DirectX::XMMATRIX S{ DirectX::XMMatrixScaling(data.scale.x, data.scale.y, data.scale.z) };
		DirectX::XMMATRIX R{ DirectX::XMMatrixRotationRollPitchYaw(data.rotation.x, data.rotation.y, data.rotation.z) };
		DirectX::XMMATRIX T{ DirectX::XMMatrixTranslation(data.translation.x, data.translation.y, data.translation.z) };
		DirectX::XMMATRIX W = S * R * T;
		DirectX::XMStoreFloat4x4(&data.world, W);
		
		if(data.aabb != nullptr) data.aabb->Update(data.translation, data.scale);
	}
}

void GameObject::Render(float deltaTime)
{
	for (auto& data : datas)
	{
		if (auto modelPtr = data.model.lock())
		{
			modelPtr->Render(data.world);
		}
		if (data.aabb && data.aabb->renderBoundingBox)
		{
			data.aabb->Render(deltaTime);
		}
	}
}

inline void GetID(const std::unordered_map<std::string, int>& dataInfos, std::vector<std::string>& container)
{
	container.resize(dataInfos.size());
	for (const auto& [id, index] : dataInfos)
	{
		container[index] = id;
	}
}

inline void CheckForSelectedModel(const std::vector<ModelData>& datas, int& selectedDataIndex)
{
	for (size_t i = 0; i < datas.size(); ++i)
	{
		if (datas[i].IsSelected())
		{
			selectedDataIndex = static_cast<int>(i);
			break;
		}
	}
}

void GameObject::DebugUI()
{
	static ImGuizmo::OPERATION currentGizmoOperation = ImGuizmo::TRANSLATE;

	if (changeInData)
	{
		changeInData = false;
		GetID(dataInfos, container);
	}

	int selectedDataIndex = -1;
	if(selected == nullptr) CheckForSelectedModel(datas, selectedDataIndex);
	std::string selectedID = selectedDataIndex >= 0 ? container[selectedDataIndex] : "None";
	if (!datas.empty() && selectedDataIndex >= 0)
	{
		int index = dataInfos[selectedID];

		selected = &datas[index];
	}

	if (gameContext->input.mouseControl.GetMouseRightClick() && !(::GetAsyncKeyState(VK_LMENU) & 0x8000))
	{
		if (selected != nullptr)
		{
			selected->AABBvsCursorRelease();
			selected = nullptr;
		}
	}

	if (ImGui::Begin("TestScene Debug UI"))
	{
		ImGui::Text("ImGui Version: %s", IMGUI_VERSION);

		if (!datas.empty())
		{
			for (auto& data : datas)
			{
				int index = &data - &datas[0];
				if (ImGui::TreeNode(container[index].c_str()))
				{
					ImGui::DragFloat3("Translation", &data.translation.x, 0.1f);
					ImGui::DragFloat3("Rotation", &data.rotation.x, 0.1f);
					ImGui::DragFloat3("Scale", &data.scale.x, 0.1f);

					if (auto model = data.model.lock())
					{
						ImGui::Checkbox("Render AABB", &data.aabb->renderBoundingBox);
					}
					if (ImGui::Button("Delete Model"))
					{
						DeleteDataByID(container[index].c_str());
					}
					ImGui::TreePop();
				}
			}
		}
	}
	ImGui::End();
	

	if (ImGui::Begin("Selected Model"))
	{
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
		ImGuizmo::SetRect(0, 0,
			SCREEN_WIDTH, SCREEN_HEIGHT);

		if (::GetAsyncKeyState('W') & 0x8000)
			currentGizmoOperation = ImGuizmo::TRANSLATE;

		if (::GetAsyncKeyState('E') & 0x8000)
			currentGizmoOperation = ImGuizmo::ROTATE;

		if (::GetAsyncKeyState('R') & 0x8000)
			currentGizmoOperation = ImGuizmo::SCALE;

		auto& view = gameContext->input.cameraControls.get_view();
		auto& projection = gameContext->input.cameraControls.get_projection();

		DirectX::XMMATRIX Proj = DirectX::XMLoadFloat4x4(&projection);

		if (!datas.empty())
		{
			if (selected == nullptr)
			{
				ImGui::End();
				return;
			}

			ImGuizmo::SetGizmoSizeClipSpace(0.2f);

			ImGui::Text("Selected Model ID: %s", selectedID.c_str());
			ImGui::DragFloat3("Translation", &selected->translation.x, 0.1f);
			ImGui::DragFloat3("Rotation", &selected->rotation.x, 0.1f);
			ImGui::DragFloat3("Scale", &selected->scale.x, 0.1f);

			if (ImGuizmo::Manipulate(
				&view._11, &projection._11,
				currentGizmoOperation, ImGuizmo::LOCAL,
				&selected->world._11))
			{
				float t[3], r[3], s[3];
				ImGuizmo::DecomposeMatrixToComponents(&selected->world._11, t, r, s);

				selected->translation = { t[0], t[1], t[2] };
				selected->rotation = {
					DirectX::XMConvertToRadians(r[0]),
					DirectX::XMConvertToRadians(r[1]),
					DirectX::XMConvertToRadians(r[2])
				};
				selected->scale = { s[0], s[1], s[2] };
			}
			if (ImGui::Button("Delete Model"))
			{
				DeleteDataByID(container[selectedDataIndex].c_str());
			}
		}
	}
	ImGui::End();
}

void GameObject::DeleteDataByID(const std::string& ID)
{
	auto it = dataInfos.find(ID);

	if(it == dataInfos.end())
		return;

	int index = it->second;

	std::string lastID;
	for(auto& [id, idx] : dataInfos)
	{
		if(idx == datas.size() - 1)
		{
			lastID = id;
			break;
		}
	}

	dataInfos.erase(it);

	std::swap(datas[index], datas.back());
	datas.pop_back();

	if (lastID != ID)
	{
		dataInfos[lastID] = index;
	}

	changeInData = true;	
}

void GameObject::HandleInput(std::string input)
{
	std::filesystem::path path(input);

	std::string modelName;
	modelName = path.filename().string();	

	for(auto& filePath : modelFilePaths)
	{
		if(filePath.first == modelName)
		{
			assetManager->loadModel(*gameContext, input);
			return;
		}
	}	

	modelFilePaths.emplace(modelName, input);

	LoadModelFromFile(input);
}