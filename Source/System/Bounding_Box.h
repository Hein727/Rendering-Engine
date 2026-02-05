#pragma once
#include "Static_Mesh.h"
#include <memory>
#include <DirectXMath.h>

class Bounding_Box
{
public :
	Bounding_Box() 
	{
		box_mesh = std::make_unique<StaticMesh>(L"./Data/cube.obj");
	};
	virtual ~Bounding_Box() = default;
	
	void Render_Box(StaticMesh* mesh, const DirectX::XMFLOAT3 position, const DirectX::XMFLOAT3 scale, const DirectX::XMFLOAT3 rotation);

private :

	std::unique_ptr<StaticMesh> box_mesh;
};