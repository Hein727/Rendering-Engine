#pragma once
#include <DirectXMath.h>
#include <Windows.h>
#include <d3d11.h>

class camera_controls
{
public:
	camera_controls() = default;
	~camera_controls() = default;
	void Update(HWND hwnd, float elapsedTime, ID3D11DeviceContext* dc);
	void setWheel(float wheel) { this->wheel = wheel; }	
	DirectX::XMFLOAT4X4 get_view() const& { return view; }
	DirectX::XMFLOAT4X4 get_projection() const& { return projection; }
	POINT get_cursor_position() const { return cursor_position; }
	DirectX::XMFLOAT3 get_position() const { return position; }

private :
	void Update_transform();

	DirectX::XMFLOAT3 position = { 0, 10, -10 };
	DirectX::XMFLOAT4 focus = { 0.0f, 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3 up = { 0, 1, 0 };
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
	POINT cursor_position{ 0, 0 };
	float rotateX{ 0.0f };
	float rotateY{ DirectX::XMConvertToRadians(180.0f) };
	float distance{ 10.0f };
	float wheel{ 0.0f };
};