#include "FrustumCulling.h"

void FrustumCulling::Update(float elapsedTime)
{
	DirectX::XMFLOAT4X4 projectionMatrix = cam.get_projection();

	DirectX::BoundingFrustum::CreateFromMatrix(frustum, DirectX::XMLoadFloat4x4(&projectionMatrix));

	DirectX::XMFLOAT4X4 viewMatrix = cam.get_view();

	frustum.Transform(frustum, DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&viewMatrix)));
}	