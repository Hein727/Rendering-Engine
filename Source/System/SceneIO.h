#pragma once
#include "Serializer.h"
#include "Scene.h"
#include "SceneIO.h"
#include <fstream>
#include <filesystem>
#include <string>

template<class T>
void SaveSceneData(const std::string& fileName, T& scene, bool isRunTime)
{
	std::string name = isRunTime ? runTimeSaveFilePath + fileName : editedSaveFilePath + fileName;
	name += ".cereal";
	std::ofstream ofs(name, std::ios::binary);

	if (!ofs.is_open()) return;

	cereal::BinaryOutputArchive serialize(ofs);
    serialize(scene);
}

template<class T>
void LoadSceneData(const std::string& fileName, T& scene, bool isRunTime)
{
    std::string saveFileName = isRunTime
        ? runTimeSaveFilePath + fileName
        : editedSaveFilePath + fileName;

    std::filesystem::path cerealFile(saveFileName);
    cerealFile.replace_extension("cereal");

    if (!std::filesystem::exists(cerealFile))
        return;

    std::ifstream ifs(cerealFile, std::ios::binary);
    if (!ifs.is_open())
        return;

    cereal::BinaryInputArchive deserialize(ifs);
    deserialize(scene);
}