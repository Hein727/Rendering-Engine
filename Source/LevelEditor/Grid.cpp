#include "Grid.h"
#include "CameraControl.h"

void Grid::Update(float elapsedTime)
{
	float oldGridRange = gridRange;
	if (oldGridRange - gridRange > 0.01f)
	{
		gridCells.clear();
		renderReady = false;
	}
	for (int x = -gridRange; x <= gridRange; x += gridSize)
	{
		for(int z = -gridRange; z <= gridRange; z += gridSize)
		{
			std::uint8_t cellKey = (static_cast<std::uint8_t>((x + gridRange) / gridSize) << 4) | static_cast<std::uint8_t>((z + gridRange) / gridSize);
			if (gridCells.find(cellKey) == gridCells.end())
			{
				GridCell cell;
				cell.position = { static_cast<float>(x), 0.0f, static_cast<float>(z) };
				cell.scale = { 1.0f, 1.0f, 1.0f };
				cell.color = { 0.5f, 0.5f, 0.5f, 1.0f };
				cell.line = std::make_unique<Geometric_Primitive>(gameContext);
				gridCells[cellKey] = std::move(cell);
			}
		}
	}
}

void Grid::Render(float elapsedTime)
{
	for(auto& [key, cell] : gridCells)
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMVECTOR S = DirectX::XMVectorSet(cell.scale.x, cell.scale.y, cell.scale.z, 1.0f);
		DirectX::XMVECTOR R = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);	
		DirectX::XMVECTOR T = DirectX::XMVectorSet(cell.position.x, cell.position.y, cell.position.z, 1.0f);
		cell.line->render(world, cell.color);
	}
}