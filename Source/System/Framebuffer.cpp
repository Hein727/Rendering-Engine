#include "Framebuffer.h"
#include "Misc.h"

Framebuffer::Framebuffer(GameContext& gameContext, uint32_t width, uint32_t height) : gameContext(gameContext)
{
	HRESULT hr{ S_OK };

	const auto& device = gameContext.graphics.GetDevice();

	// Creating back buffer texture
	Microsoft::WRL::ComPtr<ID3D11Texture2D> buffer;
	D3D11_TEXTURE2D_DESC desc{ 0 };
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	hr = device->CreateTexture2D(&desc, 0, buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	// Creating render target view
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = desc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	hr = device->CreateRenderTargetView(buffer.Get(), &rtvDesc, renderTargetView.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	// Creating shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(buffer.Get(), &srvDesc, shaderResourceViews[0].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	//Creating depth stencil buffer
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
	desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	hr = device->CreateTexture2D(&desc, 0, depthStencilBuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	// Creating depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = 0;
	hr = device->CreateDepthStencilView(depthStencilBuffer.Get(), &dsvDesc, depthStencilView.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	// Creating shader resource view for depth stencil buffer
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	hr = device->CreateShaderResourceView(depthStencilBuffer.Get(), &srvDesc, shaderResourceViews[1].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	// Setting viewport
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
}

void Framebuffer::Clear(float r, float g, float b, float a, float depth)
{
	// Clearing render target and depth stencil view
	const auto& context = gameContext.graphics.GetDeviceContext();
	float color[4] = { r, g, b, a };
	context->ClearRenderTargetView(renderTargetView.Get(), color);
	context->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, depth, 0);
}

void Framebuffer::Activate()
{
	// Setting viewport, render target view and depth stencil view
	const auto& context = gameContext.graphics.GetDeviceContext();
	viewport_count = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	context->RSGetViewports(&viewport_count, cached_viewports);
	context->OMGetRenderTargets(1, cached_render_target_view.GetAddressOf(), cached_depth_stencil_view.GetAddressOf());

	context->RSSetViewports(1, &viewport);
	context->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());
}

void Framebuffer::Deactivate()
{
	// Restoring previous viewport, render target view and depth stencil view
	const auto& context = gameContext.graphics.GetDeviceContext();
	context->RSSetViewports(viewport_count, cached_viewports);
	context->OMSetRenderTargets(1, cached_render_target_view.GetAddressOf(), cached_depth_stencil_view.Get());
}