#pragma once
#include "Shader.h"
#include "../System/GameContext.h"
#include "../System/Scene.h"
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>

CONST float SHADOW_DRAWRECT{ 30 };

class ShadowMap
{
public:

	ShadowMap() = default;
	~ShadowMap() = default;

	void Init(GameContext& gameContext, Scene::LightConstants& lightConstants ,UINT width = 1024, UINT height = 1024);
	void Begin();
	void End();
	void Debug();

protected:

	GameContext* gameContext = nullptr;
	Scene::LightConstants* lightConstants = nullptr;	

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> shadowInputLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> shadowVertexShader;

	struct ShadowMapConstants
	{
		DirectX::XMFLOAT4X4 lightViewProjection;
		DirectX::XMFLOAT3	shadowColor;
		float shadowBias;
	};

	Microsoft::WRL::ComPtr<ID3D11Buffer> shadowMapConstantBuffer;

	DirectX::XMFLOAT4X4 lightViewProjection;
	DirectX::XMFLOAT3 shadowColor{ 0.0f, 0.0f, 0.0f };
	float shadowBias{ 0.008f };	

	UINT shadowMapWidth;
	UINT shadowMapHeight;	
};