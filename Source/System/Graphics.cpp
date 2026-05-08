#include "Graphics.h"
#include "Misc.h"
#include "CameraControl.h"

graphics::graphics()
{
	// Constructor implementation
}

void graphics::initialize(HWND hwnd)
{

	//////////////Devcie, Swap Chain, Render Target View, Depth Stencil View Creation//////////////

	HRESULT hr{ S_OK };

	UINT createDeviceFlags = 0;
	
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL feature_levels{ D3D_FEATURE_LEVEL_11_0 };

	DXGI_SWAP_CHAIN_DESC swap_chain_desc{};	
	swap_chain_desc.BufferCount = 1;
	swap_chain_desc.BufferDesc.Width = SCREEN_WIDTH;
	swap_chain_desc.BufferDesc.Height = SCREEN_HEIGHT;
	swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.BufferDesc.RefreshRate.Numerator = 144;
	swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.OutputWindow = hwnd;
	swap_chain_desc.SampleDesc.Count = 1;
	swap_chain_desc.SampleDesc.Quality = 0;
	swap_chain_desc.Windowed = !FULLSCREEN;

	hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, &feature_levels, 1,
		D3D11_SDK_VERSION, &swap_chain_desc, swapChain.GetAddressOf(), device.GetAddressOf(), NULL, deviceContext.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	ID3D11Texture2D* backBuffer{};
	
	hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	hr = device->CreateRenderTargetView(backBuffer, NULL, renderTargetView.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	backBuffer->Release();

	ID3D11Texture2D* depthStencilBuffer{};
	D3D11_TEXTURE2D_DESC texture2d_desc{};

	texture2d_desc.Width = SCREEN_WIDTH;
	texture2d_desc.Height = SCREEN_HEIGHT;
	texture2d_desc.MipLevels = 1;
	texture2d_desc.ArraySize = 1;
	texture2d_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	texture2d_desc.SampleDesc.Count = 1;
	texture2d_desc.SampleDesc.Quality = 0;
	texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
	texture2d_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texture2d_desc.CPUAccessFlags = 0;
	texture2d_desc.MiscFlags = 0;
	hr = device->CreateTexture2D(&texture2d_desc, NULL, &depthStencilBuffer);
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};	
	depthStencilViewDesc.Format = texture2d_desc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	hr = device->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDesc, depthStencilView.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	depthStencilBuffer->Release();

	///////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////Viewport Creation////////////////////////////////////

	D3D11_VIEWPORT viewport{};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<FLOAT>(SCREEN_WIDTH);
	viewport.Height = static_cast<FLOAT>(SCREEN_HEIGHT);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	deviceContext->RSSetViewports(1, &viewport);

	//////////////////////////////////////////////////////////////////////////////////////

	//////////////Depth Stencil States//////////////////////

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilStates[DEPTH_MASK_ALL].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	depthStencilDesc.DepthEnable = FALSE;
	hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilStates[DEPTH_DISABLED].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	depthStencilDesc.DepthEnable = TRUE;	
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = device->CreateDepthStencilState(&depthStencilDesc, depthStencilStates[DEPTH_READONLY].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	///////////////////////////////////////////////////////////////////////////////////

	///////////////////////////Blend States Creation//////////////////////////////////////////

	D3D11_BLEND_DESC blend_desc{};
	blend_desc.AlphaToCoverageEnable = FALSE;
	blend_desc.IndependentBlendEnable = FALSE;
	blend_desc.RenderTarget[0].BlendEnable = TRUE;
	blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = device->CreateBlendState(&blend_desc, blendStates[0].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	hr = device->CreateBlendState(&blend_desc, blendStates[1].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
	hr = device->CreateBlendState(&blend_desc, blendStates[2].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_DEST_COLOR;
	blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	hr = device->CreateBlendState(&blend_desc, blendStates[3].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	///////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////Scene Constant Buffer Creation//////////////////////////////////////
	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.ByteWidth = sizeof(SceneConstants);
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;	
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;
	hr = device->CreateBuffer(&buffer_desc, NULL, sceneConstantBuffer[0].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	///////////////////////////Rasterizer States Creation//////////////////////////////////////

	// Solid fill, no culling, clockwise winding order
	D3D11_RASTERIZER_DESC rasterizer_desc{};
	rasterizer_desc.FillMode = D3D11_FILL_SOLID;
	rasterizer_desc.CullMode = D3D11_CULL_NONE;
	rasterizer_desc.FrontCounterClockwise = FALSE;
	rasterizer_desc.DepthBias = 0;
	rasterizer_desc.DepthBiasClamp = 0.0f;
	rasterizer_desc.SlopeScaledDepthBias = 0.0f;
	rasterizer_desc.DepthClipEnable = TRUE;
	rasterizer_desc.MultisampleEnable = FALSE;
	rasterizer_desc.AntialiasedLineEnable = FALSE;
	hr = device->CreateRasterizerState(&rasterizer_desc, rasterizerStates[FS_ON_CB_OFF_CW_OFF].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	// Solid fill, back-face culling, clockwise winding order
	rasterizer_desc.CullMode = D3D11_CULL_BACK;
	rasterizer_desc.FrontCounterClockwise = FALSE;
	hr = device->CreateRasterizerState(&rasterizer_desc, rasterizerStates[FS_ON_CB_ON_CW_OFF].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	// Solid fill, back-face culling, counter clockwise winding order
	rasterizer_desc.CullMode = D3D11_CULL_NONE;
	rasterizer_desc.FrontCounterClockwise = TRUE;
	hr = device->CreateRasterizerState(&rasterizer_desc, rasterizerStates[FS_ON_CB_OFF_CW_ON].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	// Solidfill, back-face culling, counter clockwise winding order
	rasterizer_desc.CullMode = D3D11_CULL_BACK;
	rasterizer_desc.FrontCounterClockwise = TRUE;
	hr = device->CreateRasterizerState(&rasterizer_desc, rasterizerStates[FS_ON_CB_ON_CW_ON].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	// Wireframe fill, no culling, clockwise winding order
	rasterizer_desc.FillMode = D3D11_FILL_WIREFRAME;
	rasterizer_desc.CullMode = D3D11_CULL_NONE;
	rasterizer_desc.FrontCounterClockwise = FALSE;
	rasterizer_desc.AntialiasedLineEnable = TRUE;
	hr = device->CreateRasterizerState(&rasterizer_desc, rasterizerStates[FS_OFF_CB_OFF_CW_OFF].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	// Wireframe fill, back-face culling, clockwise winding order
	rasterizer_desc.CullMode = D3D11_CULL_BACK;
	rasterizer_desc.FrontCounterClockwise = FALSE;
	hr = device->CreateRasterizerState(&rasterizer_desc, rasterizerStates[FS_OFF_CB_ON_CW_OFF].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	// Wireframe fill, no culling, counter clockwise winding order
	rasterizer_desc.CullMode = D3D11_CULL_NONE;
	rasterizer_desc.FrontCounterClockwise = TRUE;
	hr = device->CreateRasterizerState(&rasterizer_desc, rasterizerStates[FS_OFF_CB_OFF_CW_ON].GetAddressOf());

	//Wireframe fill, back-face culling, counter clockwise winding order
	rasterizer_desc.CullMode = D3D11_CULL_BACK;
	rasterizer_desc.FrontCounterClockwise = TRUE;
	hr = device->CreateRasterizerState(&rasterizer_desc, rasterizerStates[FS_OFF_CB_ON_CW_ON].GetAddressOf());

	////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////Sampler State//////////////////////////////////

	D3D11_SAMPLER_DESC sampler_desc{};
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampler_desc.MinLOD = 0;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = device->CreateSamplerState(&sampler_desc, samplerStates[LINEAR_WRAP].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	sampler_desc = {};
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;	
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampler_desc.MinLOD = 0;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = device->CreateSamplerState(&sampler_desc, samplerStates[LINEAR_CLAMP].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	// HIGH QUALITY use it for walls and grounds idk 
	sampler_desc = {};
	sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampler_desc.MaxAnisotropy = 16;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampler_desc.MinLOD = 0;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = device->CreateSamplerState(&sampler_desc, samplerStates[ANISOTROPIC_WRAP].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	//PIXEL PERFECT for ui type shit]
	sampler_desc = {};
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampler_desc.AddressU = sampler_desc.AddressV = sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampler_desc.MinLOD = 0;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = device->CreateSamplerState(&sampler_desc, samplerStates[POINT_CLAMP].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	sampler_desc = {};
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampler_desc.AddressU = sampler_desc.AddressV = sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampler_desc.MinLOD = 0;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = device->CreateSamplerState(&sampler_desc, samplerStates[POINT_WRAP].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	sampler_desc = {};
	sampler_desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	sampler_desc.MinLOD = 0;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = device->CreateSamplerState(&sampler_desc, samplerStates[SHADOW_COMPARISON_SAMPLER].GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));
}

//Call at the beginning of every frame for rendering
void graphics::renderingBegin()
{
	// Clear the back buffer and depth stencil view
	HRESULT hr{ S_OK };

	const FLOAT clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	deviceContext->ClearRenderTargetView(renderTargetView.Get(), clearColor);
	deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	deviceContext->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

	// Set default depth stencil state and blend state
	deviceContext->OMSetDepthStencilState(depthStencilStates[DepthStencilState::DEPTH_MASK_ALL].Get(), 1);

	deviceContext->OMSetBlendState(blendStates[BlendState::BLEND_ALPHA].Get(), nullptr, 0xffffffff);

	deviceContext->PSSetSamplers(
		0,
		7,
		samplerStates->GetAddressOf()
	);

	// Get the current viewport dimensions
	D3D11_VIEWPORT viewport{};	
	UINT num_viewports = 1;
	deviceContext->RSGetViewports(&num_viewports, &viewport);

	// Update scene constant buffer with the current screen dimensions
	float  aspect_ratio = viewport.Width / viewport.Height;	
	DirectX::XMMATRIX P{ DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(45.0f), aspect_ratio, 0.1f, 100.0f) };

	DirectX::XMFLOAT4X4 view = camera_controls::instance().get_view();

	DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&view);

	// Set up the scene constants buffer
	SceneConstants scene_constants{};
	DirectX::XMStoreFloat4x4(&scene_constants.view_projection, V * P);
	scene_constants.light_direction = { 0, 0, 1, 0 };
	DirectX::XMFLOAT3 camera_position = camera_controls::instance().get_position();
	scene_constants.camera_position = { camera_position.x, camera_position.y, camera_position.z, 0.0f };
	deviceContext->UpdateSubresource(sceneConstantBuffer[0].Get(), 0, NULL, &scene_constants, 0, 0);
	deviceContext->VSSetConstantBuffers(0, 1, sceneConstantBuffer[0].GetAddressOf());
	deviceContext->PSSetConstantBuffers(0, 1, sceneConstantBuffer[0].GetAddressOf());

	deviceContext->RSSetState(rasterizerStates[RasterizerState::FS_ON_CB_OFF_CW_OFF].Get());
}

//Call at the end of every frame for rendering
void graphics::renderingEnd()
{
	UINT sync_interval{ 1 }; // VSync enabled
	swapChain->Present(sync_interval, 0);
}

void graphics::uninitialize()
{
			
}

DirectX::XMMATRIX graphics::coordinate_system_transform(CoordChange type, float scale_factor/*1.0f for meters 0.01f is centimeters*/)
{
	const DirectX::XMFLOAT4X4 coord_system_transforms[]{
		{-1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },
		{-1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1 },
		{ 1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1 }
	};

	return DirectX::XMMatrixScaling(scale_factor, scale_factor, scale_factor) * DirectX::XMLoadFloat4x4(&coord_system_transforms[type]);
}