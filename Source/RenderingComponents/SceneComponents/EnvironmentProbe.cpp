#include "EnvironmentProbe.h"
#include "../Source/System/Misc.h"
#include <imgui.h>

static constexpr int CUBE_MAP_SIZE = 512;

EnvironmentProbe::EnvironmentProbe(GameContext& gameContext, GameObject& gameObject, SkyBox& skyBox) : gameContext(gameContext), gameObject(gameObject), skyBox(skyBox)	
{
	auto device = gameContext.graphics.GetDevice();	

	HRESULT hr{ S_OK };

	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = CUBE_MAP_SIZE;
	textureDesc.Height = CUBE_MAP_SIZE;
	textureDesc.MipLevels = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;
	textureDesc.ArraySize = 6; 
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	hr = device->CreateTexture2D(&textureDesc, nullptr, cubeMapTexture.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	for(int i = 0; i < 6; ++i)
	{
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format = textureDesc.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.ArraySize = 1;
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		rtvDesc.Texture2DArray.MipSlice = 0;
		hr = device->CreateRenderTargetView(cubeMapTexture.Get(), &rtvDesc, rtv[i].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = UINT(-1);
	hr = device->CreateShaderResourceView(cubeMapTexture.Get(), &srvDesc, srv.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	D3D11_TEXTURE2D_DESC depthDesc{};
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.Width = CUBE_MAP_SIZE;
	depthDesc.Height = CUBE_MAP_SIZE;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	hr = device->CreateTexture2D(&depthDesc, nullptr, depthTexture.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	hr = device->CreateDepthStencilView(depthTexture.Get(), &dsvDesc, depthStencilView.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	SetOptions(DIRTY);
}

void EnvironmentProbe::Update(float elapsedTime)
{
	using namespace DirectX;

	static const XMVECTOR directions[6] =
	{
		DirectX::XMVectorSet(1, 0, 0, 0), // +X
		DirectX::XMVectorSet(-1, 0, 0, 0), // -X
		DirectX::XMVectorSet(0, 1, 0, 0), // +Y
		DirectX::XMVectorSet(0,-1, 0, 0), // -Y
		DirectX::XMVectorSet(0, 0, 1, 0), // +Z
		DirectX::XMVectorSet(0, 0,-1, 0), // -Z
	};

	static const XMVECTOR ups[6] =
	{
		DirectX::XMVectorSet(0,1,0,0),
		DirectX::XMVectorSet(0,1,0,0),
		DirectX::XMVectorSet(0,0,-1,0),
		DirectX::XMVectorSet(0,0, 1,0),
		DirectX::XMVectorSet(0,1,0,0),
		DirectX::XMVectorSet(0,1,0,0)
	};

	auto context = gameContext.graphics.GetDeviceContext();	

	XMVECTOR eye = XMLoadFloat3(&probePosition);

	XMMATRIX projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, 1.0f, 0.1f, 100.0f);

	if (optionFlags & DIRTY)
	{
		for (int i = 0; i < 6; ++i)
		{
			XMMATRIX view = XMMatrixLookAtLH(eye, eye + directions[i], ups[i]);

			XMMATRIX VP = view * projection;

			XMFLOAT4X4 viewProjection;
			XMStoreFloat4x4(&viewProjection, VP);
			gameContext.graphics.SetReplacementViewProjection(viewProjection);
			gameContext.graphics.SceneConstantsUpdate(gameContext.input.cameraControls);

			context->OMSetRenderTargets(1, rtv[i].GetAddressOf(), depthStencilView.Get());

			D3D11_VIEWPORT viewport{};
			viewport.Width = CUBE_MAP_SIZE;
			viewport.Height = CUBE_MAP_SIZE;
			viewport.MaxDepth = 1.0f;
			context->RSSetViewports(1, &viewport);

			float clearColor[4] = { 0, 0, 0, 1 };
			context->ClearRenderTargetView(rtv[i].Get(), clearColor);
			context->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

			skyBox.Render(elapsedTime);
			gameObject.Render(elapsedTime);
		}
		context->GenerateMips(srv.Get());
		context->OMSetRenderTargets(0, nullptr, nullptr);

		gameContext.graphics.ResetViewport();
		gameContext.graphics.ResetSceneReplacements();
		gameContext.graphics.SceneConstantsUpdate(gameContext.input.cameraControls);

		ClearOptions(DIRTY);

		auto rtv = gameContext.graphics.GetRenderTargetView();
		context->OMSetRenderTargets(1, &rtv, gameContext.graphics.GetDepthStencilView());
	}

	if (optionFlags & DYNAMIC)
	{
		dynamicUpdateInterval -= elapsedTime;
		if(dynamicUpdateInterval <= 0.0f)
		{
			SetOptions(DIRTY);
			dynamicUpdateInterval = 1.0f;
		}
	}

	context->PSSetShaderResources(127, 1, srv.GetAddressOf());
}

void EnvironmentProbe::DebugUI()
{
	ImGui::Begin("Environment Probe");
	ImGui::Text("Position: (%.2f, %.2f, %.2f)", probePosition.x, probePosition.y, probePosition.z);
	ImGui::SliderFloat3("Probe Position", &probePosition.x, -10.0f, 10.0f);
	ImGui::SliderFloat("Radius", &radius, 0.1f, 10.0f);
	ImGui::Checkbox("Dynamic Update", reinterpret_cast<bool*>(&optionFlags));
	ImGui::End();
}