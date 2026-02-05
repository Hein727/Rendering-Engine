#pragma once

#include "Misc.h"
#include <d3d11.h>

HRESULT createVsFromCso(ID3D11Device* device, const char* filename, ID3D11VertexShader** vertex_shader,
	ID3D11InputLayout** input_layout, D3D11_INPUT_ELEMENT_DESC* input_element_desc, UINT num_elements);

HRESULT createPsFromCso(ID3D11Device* device, const char* filename, ID3D11PixelShader** pixel_shader);