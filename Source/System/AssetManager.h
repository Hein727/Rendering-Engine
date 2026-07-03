#pragma once
#include "GltfModel.h"
#include "GameContext.h"
#include "../HitCheck/AABB.h"
#include <string>
#include <unordered_map>
#include <list>
#include <memory>

class AssetManager
{
public :
	void loadModel(GameContext& gameContext, const std::string& filePath);	
	//void loadTexture(const std::string& textureName);
	//void loadSprite(const std::string& spriteName);
	//void loadAudio(const std::string& audioName);

	std::shared_ptr<GltfModel> GetModel(GameContext& gameContext, const std::string& ID, std::unique_ptr<AABB>& aabb) const;
	std::shared_ptr<GltfModel> GetModel(const std::string& ID) const;
	//void GetTexture(const std::string& textureName);	
	//void GetSprite(const std::string& spriteName);
	//void GetAudio(const std::string& audioName);

	std::list<std::string> GetAllModelNames() const;

private :

	std::unordered_map<std::string, std::shared_ptr<GltfModel>> models;
};