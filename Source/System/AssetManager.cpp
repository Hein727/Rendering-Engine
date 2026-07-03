#include "AssetManager.h"	
#include <filesystem>
#include <memory>

void AssetManager::loadModel(GameContext& gameContext, const std::string& filePath)
{
	std::filesystem::path path(filePath);

	std::string modelName = path.filename().string();	

	size_t pos = modelName.find_last_of(".");
	if (pos != std::string::npos)
	{
		modelName = modelName.substr(0, pos);
	}

	if (models.find(modelName) == models.end())
	{
		models.emplace(std::make_pair(modelName, std::make_shared<GltfModel>(gameContext, filePath)));
	}
}

std::shared_ptr<GltfModel> AssetManager::GetModel(GameContext& gameContext, const std::string& ID, std::unique_ptr<AABB>& aabb) const
{
	auto it = models.find(ID);

	if(it == models.end())
	{
		return nullptr;
	}

	if (aabb != nullptr)
	{
		aabb->SetMinMax(it->second->minPos, it->second->maxPos);	
	}
	
	return it != models.end() ? it->second : nullptr;
}

std::shared_ptr<GltfModel> AssetManager::GetModel(const std::string& ID) const
{
	auto it = models.find(ID);
	return it != models.end() ? it->second : nullptr;
}

std::list<std::string> AssetManager::GetAllModelNames() const
{
	std::list<std::string> modelNames;
	for (const auto& pair : models)
	{
		modelNames.push_back(pair.first);
	}
	return modelNames;
}
