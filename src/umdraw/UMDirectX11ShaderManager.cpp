/**
 * @file UMDirectX11Shader.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_DIRECTX

#include "UMDirectX11ShaderManager.h"

#include "UMMathTypes.h"
#include "UMVector.h"
#include "UMMatrix.h"

#include <d3dcompiler.h>
#include <tchar.h>
#include <shlwapi.h>
#include "UMStringUtil.h"
#include "UMPath.h"
#include "UMShaderEntry.h"

namespace umdraw
{

/// constructer
UMDirectX11ShaderManager::UMDirectX11ShaderManager()
	: input_layout_pointer_(NULL)
{
}

/// destructor
UMDirectX11ShaderManager::~UMDirectX11ShaderManager()
{
	for (size_t i = 0, size = constant_buffer_list_.size(); i < size; ++i) {
		if (constant_buffer_list_[i]) { 
			constant_buffer_list_[i]->Release();
		}
	}
	constant_buffer_list_.clear();
	for (size_t i = 0, size = model_constant_buffer_list_.size(); i < size; ++i) {
		if (model_constant_buffer_list_[i]) { 
			model_constant_buffer_list_[i]->Release();
		}
	}
	model_constant_buffer_list_.clear();

	SAFE_RELEASE(input_layout_pointer_);
}

/**
 * initialize
 */
bool UMDirectX11ShaderManager::init(ID3D11Device *device_pointer, ShaderType type)
{
	if (!device_pointer) return false;

	feature_level_ = device_pointer->GetFeatureLevel();

	std::u16string vs_path;
	std::u16string ps_path;
	
	if (type == eConstants)
	{
		UINT sizes[] = {
			// world_view_projection, world_view
			sizeof(UMMat44f) * 2,
			// light
			sizeof(UMVec4f) * 3,
		};

		for (int i = 0, isize = (sizeof(sizes) / sizeof(sizes[0])); i < isize; ++i)
		{
			// create contant buffers
			D3D11_BUFFER_DESC buffer_desc;
			buffer_desc.Usage          = D3D11_USAGE_DEFAULT ;
			buffer_desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
			buffer_desc.CPUAccessFlags = 0;
			buffer_desc.MiscFlags      = 0;
			buffer_desc.StructureByteStride = 0;
			buffer_desc.ByteWidth      = sizes[i];
	
			ID3D11Buffer *buffer = NULL;
			if FAILED(device_pointer->CreateBuffer(&buffer_desc, NULL, &buffer))
			{
				return false;
			}
			constant_buffer_list_.push_back(buffer);
		}
	} 
	else if (type == eBoard)
	{
		vs_path = umbase::UMPath::resource_absolute_path(umbase::UMStringUtil::utf8_to_utf16("default_vs.hlsl"));
		ps_path = umbase::UMPath::resource_absolute_path(umbase::UMStringUtil::utf8_to_utf16("default_ps.hlsl"));
	}
	else if (type == eModel || type == eLine)
	{
		UINT sizes[] = {
			// material
			sizeof(UMVec4f) * 4
		};
		
		for (int i = 0, isize = (sizeof(sizes) / sizeof(sizes[0])); i < isize; ++i)
		{
			// create contant buffers
			D3D11_BUFFER_DESC buffer_desc;
			buffer_desc.Usage          = D3D11_USAGE_DEFAULT ;
			buffer_desc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
			buffer_desc.CPUAccessFlags = 0;
			buffer_desc.MiscFlags      = 0;
			buffer_desc.StructureByteStride = 0;
			buffer_desc.ByteWidth      = sizes[i];
	
			ID3D11Buffer *buffer = NULL;
			if FAILED(device_pointer->CreateBuffer(&buffer_desc, NULL, &buffer))
			{
				return false;
			}
			model_constant_buffer_list_.push_back(buffer);
		}

		vs_path = umbase::UMPath::resource_absolute_path(umbase::UMStringUtil::utf8_to_utf16("model_vs.hlsl"));
		ps_path = umbase::UMPath::resource_absolute_path(umbase::UMStringUtil::utf8_to_utf16("model_ps.hlsl"));
	}
	else
	{
		return false;
	}

	// vertex shader
	if (!vs_path.empty())
	{
		UMDirectX11ShaderPtr shader(std::make_shared<UMDirectX11Shader>());

		const std::string& vertex_shader = UMShaderEntry::instance().dx_vertex_shader();
#ifdef NDEBUG
		if (shader->create_shader_from_memory(
			device_pointer,
			vertex_shader,
			"VS_Main",
			UMDirectX11Shader::vs))
		{
			// create input layout
			create_input_layout(device_pointer, shader, type);
			// save shader
			mutable_shader_list().push_back(shader);
		}
		else 
#endif // NDEBUG
		{
			if (shader->create_shader_from_file(
				device_pointer,
				vs_path,
				"VS_Main", 
				UMDirectX11Shader::vs))
			{
				// create input layout
				create_input_layout(device_pointer, shader, type);
				// save shader
				mutable_shader_list().push_back(shader);
			}
		}
	}

	// pixel shader
	if (!ps_path.empty())
	{
		UMDirectX11ShaderPtr shader(std::make_shared<UMDirectX11Shader>());
		
		const std::string& pixel_shader = UMShaderEntry::instance().dx_pixel_shader();
#ifdef NDEBUG
		if (shader->create_shader_from_memory(
			device_pointer,
			pixel_shader,
			"PS_Main",
			UMDirectX11Shader::ps))
		{
			// save shader
			mutable_shader_list().push_back(shader);
		}
		else 
#endif // NDEBUG
		{
			if (shader->create_shader_from_file(
				device_pointer,
				ps_path,
				"PS_Main", 
				UMDirectX11Shader::ps))
			{
				// save shader
				mutable_shader_list().push_back(shader);
			}
		}
	}

	return true;
}

/**
 * create shader input layout
 */
ID3D11InputLayout* UMDirectX11ShaderManager::create_input_layout(
	ID3D11Device *device_pointer,
	UMDirectX11ShaderPtr shader,
	ShaderType type)
{
	if (type == eBoard)
	{
		D3D11_INPUT_ELEMENT_DESC layout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		if FAILED(device_pointer->CreateInputLayout(
			layout, 
			_countof(layout), 
			shader->buffer_pointer(), 
			shader->buffer_size(), 
			&input_layout_pointer_))
		{
			return NULL;
		}
	}
	else if (type == eLine)
	{
		D3D11_INPUT_ELEMENT_DESC layout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		if FAILED(device_pointer->CreateInputLayout(
			layout, 
			_countof(layout), 
			shader->buffer_pointer(), 
			shader->buffer_size(), 
			&input_layout_pointer_))
		{
			return NULL;
		}
	}
	else if (type == eModel)
	{
		D3D11_INPUT_ELEMENT_DESC layout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		if FAILED(device_pointer->CreateInputLayout(
			layout, 
			_countof(layout), 
			shader->buffer_pointer(), 
			shader->buffer_size(), 
			&input_layout_pointer_))
		{
			return NULL;
		}
	}

	return input_layout_pointer_;
}

} // umdraw

#endif // WITH_DIRECTX
