#pragma once
#include "../Source/System/GameContext.h"
#include "../Source/System/GameObject.h"
#include "SkyBox.h"
#include "../GltfModel.h"
#include <wrl.h>
#include <d3d11.h>

class EnvironmentProbe
{
public:

	EnvironmentProbe(GameContext& gameContext, GameObject& gameObject, SkyBox& skyBox, std::shared_ptr<GltfModel> model);
	~EnvironmentProbe() = default;
	void Update(float elapsedTime);
	void Render(float elapsedTime);
	void DebugUI();	

	enum options
	{
		ACTIVE = 1 << 0,
		DIRTY = 1 << 1,
		COUNT
	};

	void SetOptions(options flag) { optionFlags |= flag; }
	void ClearOptions(options flag) { optionFlags &= ~flag; }
	void ResetOptions() { optionFlags = 0; }
	void SetRange(float newRange) { range = newRange; SetOptions(DIRTY); }
	void SetUpdateInterval(float newInterval) { dynamicUpdateInterval = newInterval; }	

private:

	GameContext& gameContext;
	GameObject& gameObject;
	SkyBox& skyBox;

	DirectX::XMFLOAT3 probePosition{ 0,0,0 };
	float range{ 30.0f };
	float dynamicUpdateInterval{ 1.0f }; // Update every 1 seconds

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv[6];
	Microsoft::WRL::ComPtr<ID3D11Texture2D> cubeMapTexture;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthTexture;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
	std::shared_ptr<GltfModel> model;
	
	uint8_t optionFlags{ 0 };
};