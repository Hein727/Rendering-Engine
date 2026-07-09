#pragma once
#include <DirectXMath.h>
#include <wrl.h>
#include <d3d11.h>

struct DirectionalLight
{
	DirectX::XMFLOAT4 direction{ 1.0f, -1.0f, -1.0f, 1.0f };
	DirectX::XMFLOAT4 color{ 1.0, 1.0, 1.0, 1.0 };
	template<class T>
	void serialize(T& archive)
	{
		archive(direction, color);
	}
};

struct PointLight
{
	DirectX::XMFLOAT4 position{ 0.0, 0.0, 0.0, 0.0 };
	DirectX::XMFLOAT4 color{ 1.0, 1.0, 1.0, 1.0 };
	float range{ 0 };
	DirectX::XMFLOAT3 padding;
	template<class T>
	void serialize(T& archive)
	{
		archive(position, color, range);
	}

};

struct SpotLight
{
	DirectX::XMFLOAT4 position{ 0.0, 0.0, 0.0, 0.0 };
	DirectX::XMFLOAT4 direction{ 0.0, 0.0, 1.0, 0.0 };
	DirectX::XMFLOAT4 color{ 1.0, 1.0, 1.0, 1.0 };
	float range{ 0 };
	float innerCone{ DirectX::XMConvertToRadians(30) };
	float outerCone{ DirectX::XMConvertToRadians(45) };
	DirectX::XMFLOAT3 padding;
	template<class T>
	void serialize(T& archive)
	{
		archive(position, direction, color, range, innerCone, outerCone);;
	}
};

struct HemisphereLight
{
	DirectX::XMFLOAT4 skyColor{ 0.0, 0.0, 0.0, 1.0 };
	DirectX::XMFLOAT4 groundColor{ 0.0, 0.0, 1.0, 1.0 };
	float weight{ 0.0 };	
	DirectX::XMFLOAT3 padding;
	template<class T>
	void serialize(T& archive)
	{
		archive(skyColor, groundColor, weight);
	}

};