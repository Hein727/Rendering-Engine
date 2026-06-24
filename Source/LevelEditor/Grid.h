#pragma once
#include "../System/GameContext.h"
#include "../System/Geometric_Primitive.h"	
#include <unordered_map>
#include <memory>
#include <DirectXMath.h>

class Grid
{
public :
	Grid(GameContext& gameContext) : gameContext(gameContext) {};
	~Grid() = default;
	void Update(float elapsedTime);
	void Render(float elapsedTime);
	void SetGridSize(float size) { gridSize = size; }	
	void SetGridRange(float range) { gridRange = range; }
	float GetGridSize() const { return gridSize; }
	float GetGridCount() const { return gridRange; }

private :

	struct GridCell
	{
		std::unique_ptr<Geometric_Primitive> line;
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 scale;
		DirectX::XMFLOAT4 color;
	};

	GameContext& gameContext;
	float gridSize{ 5.0f };
	float gridRange{ 500.0f };
	std::unordered_map<std::uint8_t, GridCell> gridCells;
	bool renderReady{ false };
	DirectX::XMFLOAT3 centerPosition{ 0.0f, 0.0f, 0.0f };
};