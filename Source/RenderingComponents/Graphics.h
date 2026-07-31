#pragma once

#include <d3d11.h>
#include <wrl/client.h> 
#include <DirectXMath.h>
#include <memory>
#include "../LevelEditor/CameraControl.h"
#include "../RenderingComponents/Lights.h"

#define FULLSCREEN false
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

class graphics
{
public:
	graphics();
    ~graphics() = default;
    void initialize(HWND hwnd);
    void uninitialize();

	enum DepthStencilState
	{
		DEPTH_MASK_ALL,
		DEPTH_DISABLED,
		DEPTH_READONLY,
	};

	enum BlendState
	{
		BLEND_ALPHA,
		BLEND_ADDITIVE,
		BLEND_REVERSE_SUBTRACTING,
		BLEND_MULTIPLY
	};

	enum RasterizerState
	{
		FS_ON_CB_OFF_CW_OFF,
		FS_ON_CB_ON_CW_OFF,
		FS_ON_CB_OFF_CW_ON,
		FS_ON_CB_ON_CW_ON,
		FS_OFF_CB_OFF_CW_OFF,
		FS_OFF_CB_ON_CW_OFF,
		FS_OFF_CB_ON_CW_ON,
		FS_OFF_CB_OFF_CW_ON,
	};

	enum SamplerState
	{
		LINEAR_WRAP,
		LINEAR_CLAMP,
		ANISOTROPIC_WRAP,
		ANISOTROPIC_BORDER,
		POINT_CLAMP,
		POINT_WRAP,
		NORMAL_MAP_SAMPLER,
		SAMPLER_STATE_COUNT
	};

	enum ShadowSamplerState
	{
		SHADOW_COMPARISON_SAMPLER,
		SHADOW_BORDER_SAMPLER,
		SHADOW_SAMPLER_STATE_COUNT	
	};	
	
	enum CoordChange
	{
		RHS_Y_UP,
		LHS_Y_UP,	
		RHS_Z_UP,
		LHS_Z_UP
	};
	DirectX::XMMATRIX coordinate_system_transform(CoordChange type, float scale_factor = 1.0f/*1.0f for meters 0.01f is centimeters*/);

	// Scene Constants
	struct SceneConstants
	{
		DirectX::XMFLOAT4X4 view_projection;
		DirectX::XMFLOAT4 camera_position;
		DirectX::XMFLOAT4 options; // xy : mouse coordinates, z : timer, w : flags 
	};
	Microsoft::WRL::ComPtr<ID3D11Buffer> sceneConstantBuffer[8];

	// Accessors
	ID3D11Device* GetDevice() const { return device.Get(); }
	ID3D11DeviceContext* GetDeviceContext() const { return deviceContext.Get(); }
	ID3D11RenderTargetView* GetRenderTargetView() const { return renderTargetView.Get(); }
	ID3D11DepthStencilView* GetDepthStencilView() const { return depthStencilView.Get(); }
	IDXGISwapChain* GetSwapChain() const { return swapChain.Get(); }
	ID3D11DepthStencilState* GetDepthStencilState(DepthStencilState state) const { return depthStencilStates[state].Get(); }
	ID3D11BlendState* GetBlendState(BlendState state) const { return blendStates[state].Get(); }
	ID3D11RasterizerState* GetRasterizerState(RasterizerState state) const { return rasterizerStates[state].Get(); }	

	// State Setters
	void SetRasterizerState(RasterizerState state) const { deviceContext->RSSetState(rasterizerStates[state].Get()); }
	void SetDepthStencilState(DepthStencilState state, UINT stencilRef = 0) const { deviceContext->OMSetDepthStencilState(depthStencilStates[state].Get(), stencilRef); }
	void SetBlendState(BlendState state) const { deviceContext->OMSetBlendState(blendStates[state].Get(), nullptr, 0xFFFFFFFF); }
	void SetReplacementViewProjection(const DirectX::XMFLOAT4X4& viewProjection) { viewProjectionReplacement = viewProjection; sceneReplacements |= REPLACE_VIEW_PROJECTION; }
	void SetReplacementCameraPosition(const DirectX::XMFLOAT4& cameraPosition) { cameraPositionReplacement = cameraPosition; sceneReplacements |= REPLACE_CAMERA_POSITION; }
	void SetReplacementOptions(const DirectX::XMFLOAT4& options) { optionsReplacement = options; sceneReplacements |= REPLACE_OPTIONS; }
	DirectX::XMFLOAT4X4 GetReplacementViewProjection() const { return viewProjectionReplacement; }
	bool IsViewProjectionReplaced() const { return (sceneReplacements & REPLACE_VIEW_PROJECTION) != 0; }	
	void ResetSceneReplacements() { sceneReplacements = 0x00000000; }	
	void SceneConstantsUpdate(camera_controls& cam);

	// Prevent copying
	graphics(const graphics&) = delete;
	graphics& operator=(const graphics&) = delete;
	void renderingBegin(camera_controls& cam, float elapsedTime);
	void render();
	void renderingEnd();

	void ResetViewport();

private:

    Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilStates[4];
	Microsoft::WRL::ComPtr<ID3D11BlendState> blendStates[4];
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerStates[8];
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerStates[7];
	Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSamplerStates[2];

	DirectX::XMFLOAT4X4 viewProjectionReplacement;
	DirectX::XMFLOAT4 cameraPositionReplacement;
	DirectX::XMFLOAT4 optionsReplacement;

	int8_t sceneReplacements = 0x00000000;

	enum replacement_flags
	{
		REPLACE_VIEW_PROJECTION = 1 << 0,
		REPLACE_CAMERA_POSITION = 1 << 1,
		REPLACE_OPTIONS = 1 << 2,
	};
};