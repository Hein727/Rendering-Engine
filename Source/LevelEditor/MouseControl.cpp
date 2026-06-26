#include "MouseControl.h"
#include "CameraControl.h"
#include "../System/Graphics.h"

void MouseControl::Update(float elapsedTime)
{
	bool currentLeftClick = (::GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	bool currentRightClick = (::GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;

	static bool oldLeftClick = false;
	static bool oldRightClick = false;

	leftClick = currentLeftClick && !oldLeftClick; 

	rightClick = currentRightClick && !oldRightClick; 

	leftHold = currentLeftClick;

	rightHold = currentRightClick;

	oldLeftClick = currentLeftClick;
	oldRightClick = currentRightClick;
}

DirectX::XMFLOAT3 MouseControl::GetMouseWorldPos(DirectX::XMMATRIX M)const // raycasted mouse position in world coordinates
{
	POINT cursorPos = cameraControls.get_cursor_position();
	DirectX::XMVECTOR CursorV = DirectX::XMVectorSet(static_cast<float>(cursorPos.x), static_cast<float>(cursorPos.y), 0.0f, 1.0f);
	float viewportX = 0.0f;
	float viewportY = 0.0f;
	float viewportWidth = static_cast<float>(SCREEN_WIDTH);
	float viewportHeight = static_cast<float>(SCREEN_HEIGHT);
	float viewportMinZ = 0.0f;
	float viewportMaxZ = 1.0f;
	DirectX::XMFLOAT4X4 p = cameraControls.get_projection();
	DirectX::XMFLOAT4X4 v = cameraControls.get_view();
	DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&p);
	DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&v);
	DirectX::XMVECTOR Near = DirectX::XMVector3Unproject(CursorV, viewportX, viewportY, viewportWidth, viewportHeight, viewportMinZ, viewportMaxZ, P, V, M);
	CursorV = DirectX::XMVectorSet(static_cast<float>(cursorPos.x), static_cast<float>(cursorPos.y), 1.0f, 1.0f);
	DirectX::XMVECTOR Far = DirectX::XMVector3Unproject(CursorV, viewportX, viewportY, viewportWidth, viewportHeight, viewportMinZ, viewportMaxZ, P, V, M);
	DirectX::XMVECTOR Direction = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(Far, Near));

	float nearY = DirectX::XMVectorGetY(Near);
	float directionY = DirectX::XMVectorGetY(Direction);

	DirectX::XMFLOAT3 cursorWorldPos{ 0, 0, 0 };

	if (fabs(directionY) > 0.00001f)
	{
		float t = -nearY / directionY;
		DirectX::XMVECTOR hitPos = DirectX::XMVectorAdd(Near, DirectX::XMVectorScale(Direction, t));
		DirectX::XMStoreFloat3(&cursorWorldPos, hitPos);
	}
	return cursorWorldPos;
};
