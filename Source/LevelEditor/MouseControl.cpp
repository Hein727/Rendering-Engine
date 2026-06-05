#include "MouseControl.h"
#include "CameraControl.h"
#include "../System/Graphics.h"

void MouseControl::Update(float elapsedTime)
{
	bool currentLeftClick = (::GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	bool currentRightClick = (::GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;

	static bool oldLeftClick = false;
	static bool oldRightClick = false;

	leftClick = currentLeftClick && !oldLeftClick; // 前フレームではクリックされていなかったが、現在はクリックされている場合

	rightClick = currentRightClick && !oldRightClick; // 前フレームではクリックされていなかったが、現在はクリックされている場合

	leftHold = currentLeftClick; // 現在クリックされているかどうか

	rightHold = currentRightClick; // 現在クリックされているかどうか

	oldLeftClick = currentLeftClick;
	oldRightClick = currentRightClick;
}

DirectX::XMFLOAT3 MouseControl::GetMouseWorldPos()
{
	DirectX::XMFLOAT2 mousePos; // マウスのスクリーン座標	

	POINT cursorPos = camera_controls::instance().get_cursor_position();	
	mousePos.x = static_cast<float>(cursorPos.x);
	mousePos.y = static_cast<float>(cursorPos.y);

	// スクリーン座標をワールド座標に変換

	float ndcX = (2.0f * mousePos.x) / SCREEN_WIDTH - 1.0f;
	float ndcY = 1.0f - (2.0f * mousePos.y) / SCREEN_HEIGHT;

	DirectX::XMVECTOR nearClip = DirectX::XMVectorSet(ndcX, ndcY, 0.0, 1.0);
	DirectX::XMVECTOR farClip = DirectX::XMVectorSet(ndcX, ndcY, 1.0, 1.0);
	
	DirectX::XMFLOAT4X4 V = camera_controls::instance().get_view();
	DirectX::XMFLOAT4X4 P = camera_controls::instance().get_projection();

	DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&V);
	DirectX::XMMATRIX projection = DirectX::XMLoadFloat4x4(&P);

	DirectX::XMMATRIX invViewProj = DirectX::XMMatrixInverse(nullptr, DirectX::XMMatrixMultiply(view, projection));

	DirectX::XMVECTOR nearWorld = DirectX::XMVector4Transform(nearClip, invViewProj);
	DirectX::XMVECTOR farWorld = DirectX::XMVector4Transform(farClip, invViewProj);	

	// ワールド座標を正規化
	{
		using namespace DirectX;
		nearWorld /= DirectX::XMVectorGetW(nearWorld);
		farWorld /= DirectX::XMVectorGetW(farWorld);
	}

	DirectX::XMVECTOR origin = nearWorld;
	DirectX::XMVECTOR dir = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(farWorld, nearWorld));

	DirectX::XMVECTOR RayEnd = DirectX::XMVectorAdd(origin, DirectX::XMVectorScale(dir, 1000.0f)); // 適当な距離で終点を設定

	DirectX::XMFLOAT3 rayEnd;
	DirectX::XMStoreFloat3(&rayEnd, RayEnd);

	bool intersect = false;

	if (intersect)
	{
		return DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f); // 仮の値
	}
	return rayEnd;
};
