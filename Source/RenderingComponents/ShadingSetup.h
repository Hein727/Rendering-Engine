#pragma once
#include <d3d11.h>
#include <wrl.h>
#include "../System/GameContext.h"
#include "../System/Misc.h"

template<class T>
void SetupConstantBuffer(GameContext& gameContext, Microsoft::WRL::ComPtr<ID3D11Buffer>& constBuffer, const T& constants)
{
	HRESULT hr{ S_OK };

	auto device = gameContext.graphics.GetDevice();

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = sizeof(T);
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	hr = device->CreateBuffer(&bufferDesc, nullptr, constBuffer.GetAddressOf());
	SUCCEEDED(hr, trace_back(hr));
}

template<class T>
void UpdateConstantBuffer(GameContext& gameContext, Microsoft::WRL::ComPtr<ID3D11Buffer>& constBuffer, const T& constants, UINT slot)
{
	auto deviceContext = gameContext.graphics.GetDeviceContext();
	deviceContext->UpdateSubresource(constBuffer.Get(), 0, nullptr, &constants, 0, 0);
	deviceContext->PSSetConstantBuffers(slot, 1, constBuffer.GetAddressOf());
}
