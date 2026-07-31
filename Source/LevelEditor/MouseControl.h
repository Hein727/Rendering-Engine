#pragma once
#include <DirectXMath.h>
#include "CameraControl.h"
#include "../RenderingComponents/Graphics.h"

class MouseControl
{
public :
	MouseControl(camera_controls& cameraControls) : cameraControls(cameraControls) {};
	~MouseControl() = default;
	void Update(float elapsedTime);

	// PS Call this before using ray hit functions
	void UpdateMouseRay(graphics& graphics);
	DirectX::XMFLOAT3 GetRayHitOnPlane(float planeY);

	enum MouseButton
	{
		LEFT_BUTTON = 1 << 0,
		RIGHT_BUTTON = 1 << 1,	
		MIDDLE_BUTTON = 1 << 2,
		COUNT_BUTTON
	};

private:
	uint8_t inputState{ 0 };
	uint8_t inputStatePrev{ 0 };

	camera_controls& cameraControls;

	DirectX::XMFLOAT3 origin{ 0,0,0 };
	DirectX::XMFLOAT3 dir{ 0,0,0 };

public :
	
	bool GetLeftButtonPressed() { return (inputState & LEFT_BUTTON) && !(inputStatePrev & LEFT_BUTTON); }
	bool GetLeftButtonReleased() { return !(inputState & LEFT_BUTTON) && (inputStatePrev & LEFT_BUTTON); }
	bool GetRightButtonPressed() { return (inputState & RIGHT_BUTTON) && !(inputStatePrev & RIGHT_BUTTON); }
	bool GetRightButtonReleased() { return !(inputState & RIGHT_BUTTON) && (inputStatePrev & RIGHT_BUTTON); }
	bool GetMiddleButtonPressed() { return (inputState & MIDDLE_BUTTON) && !(inputStatePrev & MIDDLE_BUTTON); }
	bool GetMiddleButtonReleased() { return !(inputState & MIDDLE_BUTTON) && (inputStatePrev & MIDDLE_BUTTON); }
	DirectX::XMFLOAT3 GetMouseOrigin() { return origin; }
	DirectX::XMFLOAT3 GetMouseDir() { return dir; }
};
