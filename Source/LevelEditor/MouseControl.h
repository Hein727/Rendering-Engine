#pragma once
#include <DirectXMath.h>
#include "CameraControl.h"

// Mouse control functions for the level editor 
// マウスコントロール関数（レベルエディタ用）


class MouseControl
{
public :
	MouseControl(camera_controls& cameraControls) : cameraControls(cameraControls) {};
	~MouseControl() = default;
	void Update(float elapsedTime);

private:
	bool leftClick = false;
	bool rightClick = false;
	bool leftHold = false;
	bool rightHold = false;	
	camera_controls& cameraControls;

public :
	DirectX::XMFLOAT3 GetMouseWorldPos(DirectX::XMMATRIX M = DirectX::XMMatrixIdentity()) const;
	bool GetMouseLeftClick() { return leftClick; }
	bool GetMouseRightClick() { return rightClick; }
	bool GetMouseLeftHold() { return leftHold; }
	bool GetMouseRightHold() { return rightHold; }
};
