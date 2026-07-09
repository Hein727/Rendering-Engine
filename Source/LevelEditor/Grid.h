#pragma once
#include "../System/GameContext.h"
#include "../RenderingComponents/Geometric_Primitive.h"
#include "../RenderingComponents/LineRenderer.h"
#include <unordered_map>
#include <memory>
#include <DirectXMath.h>

class Grid
{
public :
	Grid(GameContext& gameContext) : gameContext(gameContext)
	{
		lineRenderer = std::make_unique<LineRenderer>(gameContext);
	}
	~Grid() = default;
	void Update(float elapsedTime);
	void Render(float elapsedTime);
	DirectX::XMFLOAT3 GetCurrentCellLocation() const;

private :

	GameContext& gameContext;
	std::unique_ptr<LineRenderer> lineRenderer;
};