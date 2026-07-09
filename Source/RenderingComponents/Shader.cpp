#include "Shader.h"
#include <sstream>
#include <memory>

HRESULT createVsFromCso(ID3D11Device* device, const char* filename, ID3D11VertexShader** vertex_shader,
	ID3D11InputLayout** input_layout, D3D11_INPUT_ELEMENT_DESC* input_element_desc, UINT num_elements)
{
	// find the cso file
	FILE* file{ nullptr };
	fopen_s(&file, filename, "rb");
	_ASSERT_EXPR(file, "Failed to open shader file.");

	// get the file size
	fseek(file, 0, SEEK_END);
	long file_size{ ftell(file) };
	fseek(file, 0, SEEK_SET);

	// read the file data
	std::unique_ptr<unsigned char[]> cso_data{ std::make_unique<unsigned char[]>(file_size) };
	fread(cso_data.get(), file_size, 1, file);
	fclose(file);

	// create the vertex shader
	HRESULT hr{ S_OK };
	hr = device->CreateVertexShader(cso_data.get(), file_size, nullptr, vertex_shader);
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	// create the input layout
	if (input_layout)
	{
		hr = device->CreateInputLayout(input_element_desc, num_elements, cso_data.get(), file_size, input_layout);
		_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));
	}

	return hr;
}

HRESULT createPsFromCso(ID3D11Device* device, const char* filename, ID3D11PixelShader** pixel_shader)
{
	//find the cso file
	FILE* file{ nullptr };
	fopen_s(&file, filename, "rb");

	// get the file size
	fseek(file, 0, SEEK_END);
	long file_size{ ftell(file) };
	fseek(file, 0, SEEK_SET);

	// read the file data
	std::unique_ptr<unsigned char[]> cso_data{ std::make_unique<unsigned char[]>(file_size) };
	fread(cso_data.get(), file_size, 1, file);
	fclose(file);

	// create the pixel shader
	HRESULT hr{ S_OK };
	hr = device->CreatePixelShader(cso_data.get(), file_size, nullptr, pixel_shader);
	_ASSERT_EXPR(SUCCEEDED(hr), trace_back(hr));

	return hr;
}