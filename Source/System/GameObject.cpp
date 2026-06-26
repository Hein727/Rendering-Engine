#include "GameObject.h"
#include "imgui.h"
#include <filesystem>
#include <fstream>
#include <sstream>

GameObject::GameObject(GameContext& gameContext, AssetManager& assetManager)
	: gameContext(gameContext), assetManager(assetManager)
{
	std::filesystem::path cereal_filename(runtimeSaveFile);
	cereal_filename.replace_extension("cereal");

	if (!std::filesystem::exists(cereal_filename))
	{
		cereal_filename = defaultSaveFile;
		cereal_filename.replace_extension("cereal");
	}

	if (std::filesystem::exists(cereal_filename))
	{
		std::ifstream ifs(cereal_filename, std::ios::binary);
		cereal::BinaryInputArchive deserialization(ifs);
		deserialization(modelFilePaths, dataInfos, datas);

		for (auto& [fileName, filePath] : modelFilePaths)
		{
			assetManager.loadModel(gameContext,
				filePath);
		}

		for (auto& [id, index] : dataInfos)
		{
			if(modelFilePaths.empty())
				break;

			datas.resize(dataInfos.size());

			std::string ID = id;
			ID = ID.substr(0, ID.find_last_of('_'));

			auto model = assetManager.GetModel(ID);

			if (!model)
			{
				model = assetManager.GetModel(
					ID);

				if (model)
				{
					datas[index].model = model;
				}
			}
			else
			{
				datas[index].model = model;
			}
		}
		changeInData = true;
	}
}

void GameObject::LoadModelFromFile(const std::string& filePath)
{
	std::filesystem::path path(filePath);
	std::string modelName;
	modelName = path.filename().string().c_str();
	if (assetManager.GetModel(modelName) == nullptr)
	{
		assetManager.loadModel(gameContext, filePath);
	}
}

void GameObject::PlaceModel(const std::string ID)
{
	ModelData modelData;
	modelData.translation = gameContext.input.mouseControl.GetMouseWorldPos();
	modelData.scale = { 1, 1, 1 };
	modelData.rotation = { 0, 0, 0 };
	modelData.hasCollider = true;

	auto it = assetManager.GetModel(ID);

	if (it != nullptr)
	{
		auto model = assetManager.GetModel(ID);
		modelData.model = model;
		std::string newID = ID;
		int count = 1;
		while (dataInfos.contains(newID))
		{
			newID = ID + "_" + std::to_string(count);
			count++;
		}

		datas.push_back(modelData);

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
		data.model.lock()->aabb->Update(data.translation, data.scale);
	}
}

void GameObject::Render(float deltaTime)
{
	for (auto& data : datas)
	{
		data.model.lock()->Render(data.world);
		if (data.model.lock()->aabb->renderBoundingBox)
		{
			data.model.lock()->aabb->Render(deltaTime);
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

void GameObject::DebugUI()
{

	if (changeInData)
	{
		changeInData = false;
		GetID(dataInfos, container);
	}

	if (ImGui::Begin("TestScene Debug UI"))
	{
		for (auto& data : datas)
		{
			int selectedDataIndex = &data - &datas[0];
			if (ImGui::TreeNode(container[selectedDataIndex].c_str()))
			{
				ImGui::DragFloat3("Translation", &data.translation.x, 0.1f);
				ImGui::DragFloat3("Rotation", &data.rotation.x, 0.1f);
				ImGui::DragFloat3("Scale", &data.scale.x, 0.1f);

				if (auto model = data.model.lock())
				{
					ImGui::Checkbox("Render AABB", &model->useAABB);
				}

				ImGui::Text("Colliding: %s",
					data.isColliding ? "Yes" : "No");

				if(ImGui::Button("Delete Model"))
				{
					DeleteDataByID(container[selectedDataIndex].c_str());
					ImGui::TreePop();
					break;
				}

				ImGui::TreePop();
			}
		}
	}
	ImGui::End();
}

void GameObject::SaveGameState(bool isRuntimeSave)
{
	std::string name = isRuntimeSave ? runtimeSaveFile : defaultSaveFile;
	name = name + ".cereal";	
	std::ofstream ofs(name , std::ios::binary);
	cereal::BinaryOutputArchive serialization(ofs);
	serialization(modelFilePaths, dataInfos, datas);
}

void GameObject::DeleteDataByID(const std::string& ID)
{
	auto it = dataInfos.find(ID);

	if (it == dataInfos.end()) return;

	int index = it->second;
	datas.erase(datas.begin() + index);
	dataInfos.erase(it);
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
			assetManager.loadModel(gameContext, input);
			return;
		}
	}	

	modelFilePaths.emplace(modelName, input);

	LoadModelFromFile(input);
}	