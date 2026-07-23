#include "ShadowMap.h"
#include "../Source/System/Misc.h"
#include <imgui.h>

void ShadowMap::Init(GameContext& gameContext, Scene::LightConstants& lightConstants, UINT width, UINT height)
{
	HRESULT hr{ S_OK };
	this->gameContext = &gameContext;
	this->lightConstants = &lightConstants;
	auto device = gameContext.graphics.GetDevice();
	shadowMapWidth = width;
	shadowMapHeight = height;
	{
		Microsoft::WRL::ComPtr<ID3D11Texture2D> depthBuffer{};
		D3D11_TEXTURE2D_DESC depthBufferDesc{};
		depthBufferDesc.Width = width;
		depthBufferDesc.Height = height;
		depthBufferDesc.MipLevels = 1;
		depthBufferDesc.ArraySize = 1;
		depthBufferDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.SampleDesc.Quality = 0;
		depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		depthBufferDesc.CPUAccessFlags = 0;
		depthBufferDesc.MiscFlags = 0;
		hr = device->CreateTexture2D(&depthBufferDesc, nullptr, depthBuffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;
		hr = device->CreateDepthStencilView(depthBuffer.Get(), &dsvDesc, shadowDSV.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		hr = device->CreateShaderResourceView(depthBuffer.Get(), &srvDesc, shadowSRV.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));
	}

	{
		D3D11_INPUT_ELEMENT_DESC input_element_desc[]
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		const char* csoName{ "Shader\\ShadowMapCaster_vs.cso" };
		createVsFromCso(device, csoName, shadowVertexShader.GetAddressOf(), shadowInputLayout.GetAddressOf(), input_element_desc, ARRAYSIZE(input_element_desc));
	}

	{
		D3D11_BUFFER_DESC bufferDesc{};	
		bufferDesc.ByteWidth = sizeof(ShadowMapConstants);
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		hr = device->CreateBuffer(&bufferDesc, nullptr, shadowMapConstantBuffer.GetAddressOf());	
		_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));
	}
}

void ShadowMap::Begin()
{
	HRESULT hr{ S_OK };

	auto context = gameContext->graphics.GetDeviceContext();
	{
		context->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		context->OMSetRenderTargets(0, nullptr, shadowDSV.Get());	

		D3D11_VIEWPORT viewport{};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(shadowMapWidth);
		viewport.Height = static_cast<float>(shadowMapHeight);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		context->RSSetViewports(1, &viewport);

		gameContext->graphics.SetDepthStencilState(graphics::DEPTH_MASK_ALL);
		gameContext->graphics.SetRasterizerState(graphics::FS_ON_CB_ON_CW_OFF);

		context->IASetInputLayout(shadowInputLayout.Get());
		context->VSSetShader(shadowVertexShader.Get(), nullptr, 0);
		context->PSSetShader(nullptr, nullptr, 0);

		{
			using namespace DirectX;

			XMVECTOR lightPosition = XMLoadFloat4(&lightConstants->directionalLights.direction);
			lightPosition = XMVectorScale(lightPosition, -50.0f);
			XMMATRIX V = XMMatrixLookAtLH(lightPosition, XMVectorZero(), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			XMMATRIX P = XMMatrixOrthographicLH(SHADOW_DRAWRECT, SHADOW_DRAWRECT, 0.1f, 200.0f);
			XMMATRIX VP = V * P;
			XMStoreFloat4x4(&lightViewProjection, VP);
			gameContext->graphics.SetReplacementViewProjection(lightViewProjection);
			gameContext->graphics.SceneConstantsUpdate(gameContext->input.cameraControls);
		}
	}
}

void ShadowMap::End()
{
	gameContext->graphics.ResetViewport();

	auto context = gameContext->graphics.GetDeviceContext();

	context->OMSetRenderTargets(0, nullptr, nullptr);

	context->PSSetShaderResources(126, 1, shadowSRV.GetAddressOf());

	ShadowMapConstants shadowMap{};
	shadowMap.lightViewProjection = lightViewProjection;
	shadowMap.shadowColor = shadowColor;
	shadowMap.shadowBias = shadowBias;
	context->UpdateSubresource(shadowMapConstantBuffer.Get(), 0, nullptr, &shadowMap, 0, 0);
	context->VSSetConstantBuffers(12, 1, shadowMapConstantBuffer.GetAddressOf());
	context->PSSetConstantBuffers(12, 1, shadowMapConstantBuffer.GetAddressOf());
	
	auto rtv = gameContext->graphics.GetRenderTargetView();
	context->OMSetRenderTargets(1, &rtv, gameContext->graphics.GetDepthStencilView());
}

void ShadowMap::Debug()
{
	ImGui::Begin("Shadow Map");
	{
		if(ImGui::CollapsingHeader("Details"))
		{
			ImGui::Text("shadow map");
			ImGui::Image(shadowSRV.Get(), ImVec2(256, 256), ImVec2(0, 0), ImVec2(1, 1));
			ImGui::ColorEdit3("shadow_color", &shadowColor.x);
			ImGui::SliderFloat("shadow_bias", &shadowBias, 0.0f, +0.01f);
		}
	}
	ImGui::End();
}	

