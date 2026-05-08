#include "Fullscreen_Quad.h"
#include "Shader.h"
#include "Misc.h"
#include "Graphics.h"

Fullscreen_Quad::Fullscreen_Quad()
{
	//Creating shaders
	const auto& device = graphics::getInstance().GetDevice();

	createVsFromCso(device, "Shader\\Fullscreen_Quad_vs.cso", embedded_vertex_shader.GetAddressOf(), nullptr, nullptr, 0);
	createPsFromCso(device, "Shader\\Fullscreen_Quad_ps.cso", embedded_pixel_shader.GetAddressOf());
}

void Fullscreen_Quad::blit(ID3D11ShaderResourceView** srv, uint32_t start_slot, uint32_t num_views, ID3D11PixelShader* replaced_pixel_shader)
{
	const auto& context = graphics::getInstance().GetDeviceContext();

	//Clear the buffers, Change the topology and reset input layout
	context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context->IASetInputLayout(nullptr);

	//Set the 2D vs and change the ps if there's a replacement 
	context->VSSetShader(embedded_vertex_shader.Get(), 0, 0);
	replaced_pixel_shader ? context->PSSetShader(replaced_pixel_shader, 0, 0) : context->PSSetShader(embedded_pixel_shader.Get(), 0, 0);

	//Set the SRV
	context->PSSetShaderResources(start_slot, num_views, srv);

	//Render
	context->Draw(4, 0);
}