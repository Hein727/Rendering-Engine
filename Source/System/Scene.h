#pragma once
#include <d3d11.h>
#include <string>
#include <wrl.h>
#include "Serializer.h"
#include "../RenderingComponents/Lights.h"

class Scene
{
public :

	Scene() = default;
	virtual ~Scene() = default;
	virtual void Init() = 0;
	virtual void Update(float deltaTime) = 0;	
	virtual void Render(float deltaTime) = 0;
	virtual void Uninit() = 0;
	virtual void DebugUI() = 0;
	virtual void HandleInput(std::string input) = 0;
	virtual std::string GetFileName() = 0;
	virtual void SaveScene() = 0;
	virtual void LoadScene() = 0;

	struct LightConstants
	{
		static constexpr int lightMax = 8;
		DirectX::XMFLOAT4 ambientColor;
		DirectX::XMUINT4 lightCount;
		DirectionalLight directionalLights;
		PointLight pointLights[lightMax];
		SpotLight hemisphereLights[lightMax];
		template<class T>
		void serialize(T& archive)
		{
			archive(ambientColor, lightCount, directionalLights, pointLights, hemisphereLights);
		}
	};
	LightConstants lightConstants;
	
protected:

	Microsoft::WRL::ComPtr<ID3D11Buffer> lightConstantBuffer;
};