#include "Fullscreen_Quad.h"
#include "Shader.h"
#include "Misc.h"

Fullscreen_Quad::Fullscreen_Quad(GameContext& gameContext) : gameContext(gameContext)
{
	//Creating shaders
	const auto& device = gameContext.graphics.GetDevice();

	createVsFromCso(device, "Shader\\Fullscreen_Quad_vs.cso", embedded_vertex_shader.GetAddressOf(), nullptr, nullptr, 0);
	createPsFromCso(device, "Shader\\Fullscreen_Quad_ps.cso", embedded_pixel_shader.GetAddressOf());

	// Creating constant buffer

	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(LuminanceThresholdBuffer);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;
	device->CreateBuffer(&buffer_desc, nullptr, luminanceThresholdConsantBuffer.GetAddressOf());
	HRESULT hr{ S_OK };
	SUCCEEDED(hr, trace_back(hr));

	buffer_desc.ByteWidth = sizeof(BlurBuffer);
	device->CreateBuffer(&buffer_desc, nullptr, blurConstantBuffer.GetAddressOf());
	SUCCEEDED(hr, trace_back(hr));
}

void Fullscreen_Quad::blit(ID3D11ShaderResourceView** srv, uint32_t start_slot, uint32_t num_views, ID3D11PixelShader* replaced_pixel_shader)
{
	const auto& context = gameContext.graphics.GetDeviceContext();

	//Clear the buffers, Change the topology and reset input layout
	context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context->IASetInputLayout(nullptr);

	LuminanceThresholdBuffer luminanceBuffer{};
	luminanceBuffer.threshold = threshold;
	context->UpdateSubresource(luminanceThresholdConsantBuffer.Get(), 0, nullptr, &luminanceBuffer, 0, 0);
	context->PSSetConstantBuffers(1, 1, luminanceThresholdConsantBuffer.GetAddressOf());

	BlurBuffer blurBufferData{};
	blurBufferData.gs = gs;
	blurBufferData.bi = bi;
	blurBufferData.expo = expo;
	context->UpdateSubresource(blurConstantBuffer.Get(), 0, nullptr, &blurBufferData, 0, 0);
	context->PSSetConstantBuffers(2, 1, blurConstantBuffer.GetAddressOf());

	//Set the 2D vs and change the ps if there's a replacement 
	context->VSSetShader(embedded_vertex_shader.Get(), 0, 0);
	replaced_pixel_shader ? context->PSSetShader(replaced_pixel_shader, 0, 0) : context->PSSetShader(embedded_pixel_shader.Get(), 0, 0);

	//Set the SRV
	context->PSSetShaderResources(start_slot, num_views, srv);

	//Render
	context->Draw(4, 0);
}