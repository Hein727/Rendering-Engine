#include "MouseControl.h"
#include "CameraControl.h"
#include "../RenderingComponents/Graphics.h"
#include "imgui.h"

void MouseControl::Update(float elapsedTime)
{
	uint8_t currentButtons = 0;

	currentButtons |= (::GetAsyncKeyState(VK_LBUTTON) & 0x8000) ? LEFT_BUTTON : 0;
	currentButtons |= (::GetAsyncKeyState(VK_RBUTTON) & 0x8000) ? RIGHT_BUTTON : 0;
	currentButtons |= (::GetAsyncKeyState(VK_MBUTTON) & 0x8000) ? MIDDLE_BUTTON : 0;
	
	uint8_t pressed = inputState & ~inputStatePrev;
	uint8_t released = ~inputState & inputStatePrev;	
	uint8_t hold = inputState & inputStatePrev;

	inputStatePrev = inputState;
	inputState = 0;

	inputState |= (pressed & LEFT_BUTTON) ? PRESSED : 0;
	inputState |= (pressed & RIGHT_BUTTON) ? PRESSED : 0;
	inputState |= (pressed & MIDDLE_BUTTON) ? PRESSED : 0;

	inputState |= (released & LEFT_BUTTON) ? RELEASED : 0;
	inputState |= (released & RIGHT_BUTTON) ? RELEASED : 0;
	inputState |= (released & MIDDLE_BUTTON) ? RELEASED : 0;

	inputState |= (hold & LEFT_BUTTON) ? HELD : 0;
	inputState |= (hold & RIGHT_BUTTON) ? HELD : 0;
	inputState |= (hold & MIDDLE_BUTTON) ? HELD : 0;

	buttonType = currentButtons;
}

void MouseControl::UpdateMouseRay(graphics& graphics)
{
	using namespace DirectX;

	D3D11_VIEWPORT viewport{};
	UINT numViewports = 1;
	auto dc = graphics.GetDeviceContext();

	dc->RSGetViewports(&numViewports, &viewport);

	POINT cursorPos = cameraControls.get_cursor_position();
	XMVECTOR CursorNear = XMVectorSet(static_cast<float>(cursorPos.x), static_cast<float>(cursorPos.y), 0.0f, 1.0f);
	float viewportX = viewport.TopLeftX;
	float viewportY = viewport.TopLeftY;
	float viewportWidth = viewport.Width;
	float viewportHeight = viewport.Height;
	float viewportMinZ = viewport.MinDepth;
	float viewportMaxZ = viewport.MaxDepth;
	XMMATRIX M = XMMatrixIdentity();
	XMFLOAT4X4 p = cameraControls.get_projection();
	XMFLOAT4X4 v = cameraControls.get_view();
	XMMATRIX P = XMLoadFloat4x4(&p);
	XMMATRIX V = XMLoadFloat4x4(&v);
	XMVECTOR Near = XMVector3Unproject(CursorNear, viewportX, viewportY, viewportWidth, viewportHeight, viewportMinZ, viewportMaxZ, P, V, M);
	XMVECTOR CursorFar = XMVectorSet(static_cast<float>(cursorPos.x), static_cast<float>(cursorPos.y), 1.0f, 1.0f);
	XMVECTOR World = XMVector3Unproject(CursorFar, viewportX, viewportY, viewportWidth, viewportHeight, viewportMinZ, viewportMaxZ, P, V, M);
	XMVECTOR Direction = XMVector3Normalize(XMVectorSubtract(World, Near));

	origin = XMFLOAT3(XMVectorGetX(Near), XMVectorGetY(Near), XMVectorGetZ(Near));
	dir = DirectX::XMFLOAT3(XMVectorGetX(Direction), XMVectorGetY(Direction), XMVectorGetZ(Direction));
}

DirectX::XMFLOAT3 MouseControl::GetRayHitOnPlane(float planeY)
{
	float originY = origin.y;
	float dirY = dir.y;

	if(fabs(dir.y) < FLT_EPSILON)
		return DirectX::XMFLOAT3(0, planeY, 0);

	float t = (planeY - originY) / dirY;
	
	if(t < 0.0f)
		return DirectX::XMFLOAT3(0, planeY, 0);

	return DirectX::XMFLOAT3(origin.x + dir.x * t, planeY, origin.z + dir.z * t);
}