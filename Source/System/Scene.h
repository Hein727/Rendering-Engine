#pragma once
#include <d3d11.h>
#include <string>
#include <wrl.h>
#include "Serializer.h"
#include "../RenderingComponents/Lights.h"

class Scene
{
private :
	static constexpr int lightMax = 8;

public :

	Scene() = default;
	virtual ~Scene() = default;
	virtual void Init() = 0;
	virtual void Update(float deltaTime) = 0;	
	virtual void ShadowRender(float deltaTime) = 0;	
	virtual void MainRender(float deltaTime) = 0;
	virtual void Uninit() = 0;
	virtual void DebugUI() = 0;
	virtual void HandleInput(std::string input) = 0;
	virtual std::string GetFileName() = 0;
	virtual void SaveScene() = 0;
	virtual void LoadScene() = 0;

	struct LightConstants
	{
		DirectX::XMFLOAT4 ambientColor;
		DirectX::XMUINT4 lightCount;
		DirectionalLight directionalLights;
		PointLight pointLights[lightMax];
		SpotLight spotLights[lightMax];
		HemisphereLight hemisphereLights[lightMax];
		template<class T>
		void serialize(T& archive)
		{
			archive(ambientColor, lightCount, directionalLights, pointLights, spotLights, hemisphereLights);
		}
	};
	LightConstants lightConstants;
	
protected:

	Microsoft::WRL::ComPtr<ID3D11Buffer> lightConstantBuffer;
};