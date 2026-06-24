#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <cstdint>
#include <memory>
#include "GameContext.h"

class Fullscreen_Quad
{
public:
	Fullscreen_Quad(GameContext& gameContext);
	virtual ~Fullscreen_Quad() = default;
	float threshold = 0.8f;
	float gs = 0.5f;
	float bi = 0.5f;
	float expo = 0.5f;	

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> embedded_vertex_shader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> embedded_pixel_shader;

	struct LuminanceThresholdBuffer
	{
		float threshold;
		float padding[3];
	};
	Microsoft::WRL::ComPtr<ID3D11Buffer> luminanceThresholdConsantBuffer;

	struct BlurBuffer
	{
		float gs;
		float bi;
		float expo;
		float dummy;
	};
	Microsoft::WRL::ComPtr<ID3D11Buffer> blurConstantBuffer;

	GameContext& gameContext;

public:
	void blit(ID3D11ShaderResourceView** srv, uint32_t start_slot, uint32_t num_views, ID3D11PixelShader* replaced_pixel_shader = nullptr);

};