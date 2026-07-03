#pragma once
#include <DirectXMath.h>
#include "CameraControl.h"
#include "../System/Graphics.h"

class MouseControl
{
public :
	MouseControl(camera_controls& cameraControls) : cameraControls(cameraControls) {};
	~MouseControl() = default;
	void Update(float elapsedTime);

	// PS Call this before using ray hit functions
	void UpdateMouseRay(graphics& graphics);
	DirectX::XMFLOAT3 GetRayHitOnPlane(float planeY);

private:
	bool leftClick = false;
	bool rightClick = false;
	bool leftHold = false;
	bool rightHold = false;	
	camera_controls& cameraControls;

	DirectX::XMFLOAT3 origin{ 0,0,0 };
	DirectX::XMFLOAT3 dir{ 0,0,0 };

public :
	bool GetMouseLeftClick() { return leftClick; }
	bool GetMouseRightClick() { return rightClick; }
	bool GetMouseLeftHold() { return leftHold; }
	bool GetMouseRightHold() { return rightHold; }
	DirectX::XMFLOAT3 GetMouseOrigin() { return origin; }
	DirectX::XMFLOAT3 GetMouseDir() { return dir; }
};
