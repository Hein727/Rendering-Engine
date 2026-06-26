#include "Grid.h"
#include "CameraControl.h"

constexpr int subdivisions{ 100 };
constexpr int scale{ 4 };

void Grid::Update(float elapsedTime)
{

}

void Grid::Render(float elapsedTime)
{
	lineRenderer->DrawGrid(subdivisions, scale);
	lineRenderer->Render();	
}