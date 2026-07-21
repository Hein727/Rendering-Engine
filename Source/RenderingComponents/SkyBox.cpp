#include "SkyBox.h"
#include "Shader.h"
#include "../System/Misc.h"
#include "Texture.h"

SkyBox::SkyBox(GameContext& gameContext, const wchar_t* textureFilePath) : gameContext(gameContext)
{
	HRESULT hr{ S_OK };

	auto device = gameContext.graphics.GetDevice();
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = sizeof(SkyMapConstants);
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;
		hr = device->CreateBuffer(&bufferDesc, nullptr, skyMapConstantBuffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));
	}

	{
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		hr = device->CreateDepthStencilState(&depthStencilDesc, skyMapDepthStencilView.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));
	}

	{
		LoadHDRTextureFromFile(device, textureFilePath, skyMapShaderResourceView.GetAddressOf());
		skyMapSprite = std::make_unique<Sprite>(gameContext, skyMapShaderResourceView);	
	}

	{
		D3D11_INPUT_ELEMENT_DESC input_element_desc[]
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		const char* csoName{ "Shader\\SkyMap_vs.cso" };
		createVsFromCso(device, csoName, skyMapVertexShader.GetAddressOf(), skyMapInputLayout.GetAddressOf(), input_element_desc, ARRAYSIZE(input_element_desc));
		csoName = { "Shader\\SkyMap_ps.cso" };
		createPsFromCso(device, csoName, skyMapPixelShader.GetAddressOf());
	}
}

void SkyBox::Render(float elapsedTime)
{
	auto context = gameContext.graphics.GetDeviceContext();
	{
		SkyMapConstants skyMapConstants{};
		DirectX::XMFLOAT4X4 v = gameContext.input.cameraControls.get_view();
		DirectX::XMFLOAT4X4 p = gameContext.input.cameraControls.get_projection();

		DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&v);
		DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&p);

		DirectX::XMStoreFloat4x4(&skyMapConstants.inverse_view_projection, DirectX::XMMatrixInverse(nullptr, V * P));

		context->UpdateSubresource(skyMapConstantBuffer.Get(), 0, 0, &skyMapConstants, 0, 0);
		context->VSSetConstantBuffers(1, 1, skyMapConstantBuffer.GetAddressOf());
		context->PSSetConstantBuffers(1, 1, skyMapConstantBuffer.GetAddressOf());
	}
	
	if (skyMapSprite)
	{
		context->IASetInputLayout(skyMapInputLayout.Get());	
		context->OMSetDepthStencilState(skyMapDepthStencilView.Get(), 0);

		skyMapSprite->Render(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, skyMapVertexShader.Get(), skyMapPixelShader.Get());

		gameContext.graphics.SetDepthStencilState(graphics::DEPTH_MASK_ALL);	
	}
}

