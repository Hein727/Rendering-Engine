#include "CameraControl.h"
#include <d3d11.h>

camera_controls::camera_controls()
{

}


void camera_controls::Update(HWND hwnd, float elapsedTime, ID3D11DeviceContext* dc)
{
	POINT point;
	RECT rc;
	::GetCursorPos(&point);
	::ScreenToClient(hwnd, &point);
	::GetClientRect(hwnd, &rc);
	UINT screenW = rc.right - rc.left;
	UINT screenH = rc.bottom - rc.top;
	POINT old_cursor;
	old_cursor.x = cursor_position.x;
	old_cursor.y = cursor_position.y;
	cursor_position.x = (LONG)(point.x / static_cast<float>(1280) * static_cast<float>(screenW));
	cursor_position.y = (LONG)(point.y / static_cast<float>(720) * static_cast<float>(screenH));

	float moveX = (cursor_position.x - old_cursor.x) * 0.5f * elapsedTime;
	float moveY = (cursor_position.y - old_cursor.y) * 0.5f * elapsedTime;
	if (::GetAsyncKeyState(VK_LMENU) & 0x8000)
	{

		if (::GetAsyncKeyState(VK_LBUTTON) & 0x8000)
		{
			rotateY += moveX * 0.5f;
			if (rotateY > DirectX::XM_PI)
			{
				rotateY -= DirectX::XM_2PI;
			}
			else if (rotateY < -DirectX::XM_PI)
			{
				rotateY += DirectX::XM_2PI;
			}

			rotateX += moveY * 0.5f;
			if (rotateX > DirectX::XMConvertToRadians(89.9f))
			{
				rotateX = DirectX::XMConvertToRadians(89.9f);
			}
			else if (rotateX < -DirectX::XMConvertToRadians(89.9f))
			{
				rotateX = -DirectX::XMConvertToRadians(89.9f);
			}
		}
		if (::GetAsyncKeyState(VK_MBUTTON) & 0x8000)
		{
			Update_transform();

			DirectX::XMFLOAT4X4 W;
			DirectX::XMStoreFloat4x4(&W, DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&view)));

			float s = distance * 0.1f;
			float x = moveX * s;
			float y = moveY * s;
			focus.x -= W._11 * x;
			focus.y -= W._12 * x;
			focus.z -= W._13 * x;

			focus.x += W._21 * y;
			focus.y += W._22 * y;
			focus.z += W._23 * y;
		}
		if (wheel != 0)
		{
			distance -= static_cast<float>(wheel) * distance * 0.001f;
			wheel = 0;
		}
	}
	float sx = ::sinf(rotateX), cx = ::cosf(rotateX);
	float sy = ::sinf(rotateY), cy = ::cosf(rotateY);
	DirectX::XMVECTOR Focus = DirectX::XMLoadFloat4(&focus);
	DirectX::XMVECTOR Front = DirectX::XMVectorSet(-cx * sy, -sx, -cx * cy, 0.0f);
	DirectX::XMVECTOR Distance = DirectX::XMVectorSet(distance, distance, distance, 0.0f);
	Front = DirectX::XMVectorMultiply(Front, Distance);
	DirectX::XMVECTOR Eye = DirectX::XMVectorSubtract(Focus, Front);
	DirectX::XMStoreFloat3(&position, Eye);
	Update_transform();
	
	DirectX::XMMATRIX P;
	{
		D3D11_VIEWPORT viewport;
		UINT num_viewports{ 1 };
		dc->RSGetViewports(&num_viewports, &viewport);
		float aspect{ viewport.Width / viewport.Height };
		P = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(30.0f), aspect, 0.1f, 100.0f);
		DirectX::XMStoreFloat4x4(&projection, P);
	}
}


void camera_controls::Update_transform()
{
	using namespace DirectX;

	XMVECTOR Position = XMLoadFloat3(&position);
	XMVECTOR Focus = XMLoadFloat4(&focus);
	XMVECTOR Up = XMLoadFloat3(&up);

	XMMATRIX V = XMMatrixLookAtLH(Position, Focus, Up);
	XMStoreFloat4x4(&view, V);
}