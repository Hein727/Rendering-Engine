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

DirectX::XMFLOAT3 MouseControl::GetMouseWorldPos() // raycasted mouse position in world coordinates
{
	DirectX::XMFLOAT2 mousePos;

	POINT cursorPos = cameraControls.get_cursor_position();
	mousePos.x = static_cast<float>(cursorPos.x);
	mousePos.y = static_cast<float>(cursorPos.y);

	float ndcX = (2.0f * mousePos.x) / SCREEN_WIDTH - 1.0f;
	float ndcY = 1.0f - (2.0f * mousePos.y) / SCREEN_HEIGHT;

	DirectX::XMVECTOR nearClip = DirectX::XMVectorSet(ndcX, ndcY, 0.0, 1.0);
	DirectX::XMVECTOR farClip = DirectX::XMVectorSet(ndcX, ndcY, 1.0, 1.0);
	
	DirectX::XMFLOAT4X4 V = cameraControls.get_view();
	DirectX::XMFLOAT4X4 P = cameraControls.get_projection();

	DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&V);
	DirectX::XMMATRIX projection = DirectX::XMLoadFloat4x4(&P);

	DirectX::XMMATRIX invViewProj = DirectX::XMMatrixInverse(nullptr, DirectX::XMMatrixMultiply(view, projection));

	DirectX::XMVECTOR nearWorld = DirectX::XMVector3TransformCoord(nearClip, invViewProj);
	DirectX::XMVECTOR farWorld = DirectX::XMVector3TransformCoord(farClip, invViewProj);	

	DirectX::XMFLOAT3 cameraPos = cameraControls.get_position();
	DirectX::XMVECTOR origin = DirectX::XMLoadFloat3(&cameraPos);
	DirectX::XMVECTOR dir = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(farWorld, nearWorld));

	DirectX::XMVECTOR RayEnd = DirectX::XMVectorAdd(origin, DirectX::XMVectorScale(dir, 100.0f)); // ìKìñÇ»ãóó£Ç≈èIì_Çê›íË

	DirectX::XMFLOAT3 rayEnd;
	DirectX::XMStoreFloat3(&rayEnd, RayEnd);

	return rayEnd;
};
