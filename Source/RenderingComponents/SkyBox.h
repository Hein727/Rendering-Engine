#pragma once
#include "../System/GameContext.h"
#include "Sprite.h"

#include <d3d11.h>
#include <wrl.h>

class SkyBox
{
public :
	SkyBox(GameContext& gameContext, const wchar_t* textureFilePath);
	~SkyBox() = default;

	void Render(float elapsedTime);

private :
	// Skymap resources
	Microsoft::WRL::ComPtr<ID3D11Buffer> skyMapConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> skyMapVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> skyMapPixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> skyMapInputLayout;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> skyMapDepthStencilView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skyMapShaderResourceView;
	std::unique_ptr<Sprite> skyMapSprite;

	GameContext& gameContext;

	struct SkyMapConstants
	{
		DirectX::XMFLOAT4X4 inverse_view_projection;
	};
};