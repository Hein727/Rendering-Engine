#pragma once
#include "../LevelEditor/CameraControl.h"
#include <DirectXMath.h>
#include <DirectXCollision.h>

class FrustumCulling
{
public:
	FrustumCulling(camera_controls& cam) : cam(cam) {};
	~FrustumCulling() = default;
	void Update(float elapsedTime);
	DirectX::BoundingFrustum& GetFrustum() { return frustum; }

private:
	camera_controls& cam;
	DirectX::BoundingFrustum frustum;
};