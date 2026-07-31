#pragma once
#include "SkyBox.h"
#include "ShadowMap.h"
#include "../Source/System/GameContext.h"
#include "../Source/System/GameObject.h"
#include "../Source/System/Scene.h"
#include "EnvironmentProbe.h"
#include "../GltfModel.h"
#include <memory>
#include <vector>

class Environment
{
public:
	Environment() = default;
	~Environment() = default;
	void Init(GameContext& gameContext, GameObject& gameObject, Scene::LightConstants& lightConstants, Microsoft::WRL::ComPtr<ID3D11Buffer>& lightConstantBuffer, const wchar_t* skyBoxPath = nullptr);
	void Update(float deltaTime);
	void CaptureEnvironmentProbes(float deltaTime);
	void Render(float deltaTime);
	void ShadowRender(float deltaTime);
	void DebugUI();

private:

	GameContext* gameContext = nullptr;
	GameObject* gameObject = nullptr;
	Scene::LightConstants* lightConstants = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer> lightConstantBuffer;

	std::vector<std::unique_ptr<EnvironmentProbe>> environmentProbes;
	std::unique_ptr<SkyBox> skyBox;
	std::unique_ptr<ShadowMap> shadowMap;
	std::shared_ptr<GltfModel> probeMesh;

	static constexpr const wchar_t* defaultSkyBoxPath = L"./Data/Skybox/overcast_soil_puresky_4k.hdr";
};