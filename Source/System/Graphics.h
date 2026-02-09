#pragma once

#include <d3d11.h>
#include <wrl/client.h> 
#include <DirectXMath.h>	

#define FULLSCREEN false
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

class graphics
{
public:
    static graphics& getInstance()
    {
        static graphics instance;
        return instance;
	}

    ~graphics() = default;
    void initialize(HWND hwnd);
    void uninitialize();

	enum DepthStencilState
	{
		DEPTH_MASK_ALL,
		DEPTH_DISABLED,
		DEPTH_READONLY,
		DEPTH_NONE
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
		POINT_CLAMP,
		POINT_WRAP,
		SHADOW_COMPARISON_SAMPLER,
		NORMAL_MAP_SAMPLER
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
		DirectX::XMFLOAT4 light_direction;
		DirectX::XMFLOAT4 camera_position;
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

	// State Setters
	void SetRasterizerState(RasterizerState state) const { deviceContext->RSSetState(rasterizerStates[state].Get()); }
	void SetDepthStencilState(DepthStencilState state, UINT stencilRef = 0) const { deviceContext->OMSetDepthStencilState(depthStencilStates[state].Get(), stencilRef); }
	void SetSamplerState(SamplerState state) const { deviceContext->PSSetSamplers(0, 1, samplerStates[state].GetAddressOf()); }

	// Prevent copying
	graphics(const graphics&) = delete;
	graphics& operator=(const graphics&) = delete;
	void renderingBegin();
	void renderingEnd();	


private:
    graphics();

    Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilStates[4];
	Microsoft::WRL::ComPtr<ID3D11BlendState> blendStates[4];
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerStates[8];
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerStates[7];
};