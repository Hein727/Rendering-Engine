#include "AssetManager.h"	
#include <filesystem>

void AssetManager::loadModel(GameContext& gameContext, const std::string& filePath)
{
	std::filesystem::path path(filePath);

	std::string modelName = path.filename().string();	

	if (models.find(modelName) == models.end())
	{
		models.emplace(std::make_pair(modelName, std::make_shared<GltfModel>(gameContext, filePath)));
	}
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
