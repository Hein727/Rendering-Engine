#include "Environment.h"
#include "../ShadingSetup.h"

void Environment::Init(GameContext& gameContext, GameObject& gameObject, Scene::LightConstants& lightConstants, Microsoft::WRL::ComPtr<ID3D11Buffer>& lightConstantBuffer, const wchar_t* skyBoxPath)
{
	this->gameContext = &gameContext;
	this->gameObject = &gameObject;
	this->lightConstants = &lightConstants;

	skyBox = std::make_unique<SkyBox>(gameContext, skyBoxPath == nullptr ? defaultSkyBoxPath : skyBoxPath);
	shadowMap = std::make_unique<ShadowMap>();

	probeMesh = std::make_shared<GltfModel>(gameContext, "./Data/ProbeCube.gltf");
	std::unique_ptr<EnvironmentProbe> probe = std::make_unique<EnvironmentProbe>(gameContext, gameObject, *skyBox, probeMesh);
	environmentProbes.push_back(std::move(probe));

	SetupConstantBuffer(gameContext, lightConstantBuffer, lightConstants);
	this->lightConstantBuffer = lightConstantBuffer;

	shadowMap->Init(gameContext, lightConstants);
}

void Environment::Update(float deltaTime)
{
	// Update environment-related logic here
}

void Environment::CaptureEnvironmentProbes(float deltaTime)
{
}	

void Environment::ShadowRender(float deltaTime)
{
	shadowMap->Begin();

	gameObject->ShadowMapRender(deltaTime);

	shadowMap->End();
}

void Environment::Render(float deltaTime)
{
	gameContext->graphics.ResetSceneReplacements();

	gameContext->graphics.SceneConstantsUpdate(gameContext->input.cameraControls);

	skyBox->Render(deltaTime);

	for (auto& probe : environmentProbes)
	{
		probe->Update(deltaTime);

#ifdef _DEBUG
		probe->Render(deltaTime);
#endif
	}
}

void Environment::DebugUI()
{
	//shadowMap->Debug();

	for(auto& probe : environmentProbes)
	{
		probe->DebugUI();
	}	
}