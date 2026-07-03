#include "Grid.h"

constexpr int subdivisions{ 50 };
constexpr int scale{ 1 };

void Grid::Update(float elapsedTime)
{

}

void Grid::Render(float elapsedTime)
{
	lineRenderer->DrawGrid(subdivisions, scale);
	lineRenderer->Render();	
}

DirectX::XMFLOAT3 Grid::GetCurrentCellLocation() const
{
	DirectX::XMFLOAT3 origin = lineRenderer->GetCurrentCellOrigin();

	DirectX::XMFLOAT3 hitPos = gameContext.input.mouseControl.GetRayHitOnPlane(origin.y);

	int x = std::floor((hitPos.x - origin.x) / scale);
	int z = std::floor((hitPos.z - origin.z) / scale);

	DirectX::XMFLOAT3 hitLocation{};
	hitLocation.x = origin.x + (x + 0.5f) * scale;
	hitLocation.y = origin.y;
	hitLocation.z = origin.z + (z + 0.5f) * scale;

	return hitLocation;
}