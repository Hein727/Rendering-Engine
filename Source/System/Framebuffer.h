#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <cstdint>
#include "GameContext.h"

class Framebuffer
{
public :
	Framebuffer(GameContext& gameContext, uint32_t width, uint32_t height);
	virtual ~Framebuffer() = default;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceViews[2];
	D3D11_VIEWPORT viewport;

	void Clear(float r = 0, float g = 0, float b = 0, float a = 1, float depth = 1);
	void Activate();
	void Deactivate();

private :
	
	UINT viewport_count{ D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE };
	D3D11_VIEWPORT cached_viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{};
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> cached_render_target_view;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> cached_depth_stencil_view;

	GameContext& gameContext;
};