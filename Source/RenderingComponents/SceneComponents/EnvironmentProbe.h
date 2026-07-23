#pragma once
#include "../Source/System/GameContext.h"
#include "../Source/System/GameObject.h"
#include "SkyBox.h"
#include <wrl.h>
#include <d3d11.h>

class EnvironmentProbe
{
public:

	EnvironmentProbe(GameContext& gameContext, GameObject& gameObject, SkyBox& skyBox);
	~EnvironmentProbe() = default;
	void Update(float elapsedTime);
	void Render(float elapsedTime);
	void DebugUI();	

	enum options
	{
		DYNAMIC = 1 << 0,
		DIRTY = 1 << 1,
		UP = 1 << 2,
		DOWN = 1 << 3,
		LEFT = 1 << 4,
		RIGHT = 1 << 5,
		FRONT = 1 << 6,
		BACK = 1 << 7,
		COUNT
	};

	void SetOptions(options flag) { optionFlags |= flag; }
	void ClearOptions(options flag) { optionFlags &= ~flag; }
	void ResetOptions() { optionFlags = 0; }	

private:

	GameContext& gameContext;
	GameObject& gameObject;
	SkyBox& skyBox;

	DirectX::XMFLOAT3 probePosition{ 0,0,0 };
	float radius{ 1.0f };
	float dynamicUpdateInterval{ 1.0f }; // Update every 1 seconds

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv[6];
	Microsoft::WRL::ComPtr<ID3D11Texture2D> cubeMapTexture;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthTexture;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
	
	int8_t optionFlags{ 0 };
};