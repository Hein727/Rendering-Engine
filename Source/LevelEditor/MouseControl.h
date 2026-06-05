#pragma once
#include <DirectXMath.h>

// Mouse control functions for the level editor 
// マウスコントロール関数（レベルエディタ用）


class MouseControl
{
private:
	static MouseControl& GetInstance()
	{
		static MouseControl instance;
		return instance;
	}
	MouseControl() = default;

public:
	~MouseControl() = default;
	void Update(float elapsedTime);

private:
	bool leftClick = false;
	bool rightClick = false;
	bool leftHold = false;
	bool rightHold = false;	

public :
	DirectX::XMFLOAT3 GetMouseWorldPos();
	bool GetMouseLeftClick() { return leftClick; }
	bool GetMouseRightClick() { return rightClick; }
	bool GetMouseLeftHold() { return leftHold; }
	bool GetMouseRightHold() { return rightHold; }
};
