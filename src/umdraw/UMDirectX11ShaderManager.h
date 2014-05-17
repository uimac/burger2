/**
 * @file UMDirectX11Shader.h
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <dxgi.h>
#include <d3d11.h>
#include <vector>
#include <memory>
#include "UMMacro.h"
#include "UMDirectX11Shader.h"

namespace umdraw
{

class UMDirectX11ShaderManager;
typedef std::shared_ptr<UMDirectX11ShaderManager> UMDirectX11ShaderManagerPtr;
typedef std::weak_ptr<UMDirectX11ShaderManager> UMDirectX11ShaderManagerWeakPtr;

/**
 * shader manager
 */
class UMDirectX11ShaderManager 
{
	DISALLOW_COPY_AND_ASSIGN(UMDirectX11ShaderManager);
public:
	typedef std::vector<UMDirectX11ShaderPtr> ShaderList;
	typedef std::vector<ID3D11Buffer*> BufferPointerList;

	enum ShaderType {
		eConstants,
		eBoard,
		eModel,
		eLine
	};

	UMDirectX11ShaderManager();

	~UMDirectX11ShaderManager();
	
	/**
	 * initialize
	 * @param [in] device_pointer directx11 device
	 * @param [in] type shader type
	 */
	bool init(ID3D11Device *device_pointer, ShaderType type);

	/**
	 * get feature level
	 */
	D3D_FEATURE_LEVEL feature_level() const { return feature_level_; }
	
	/**
	 * get constant buffer list
	 */
	const BufferPointerList& constant_buffer_list() const { return constant_buffer_list_; }
	
	/**
	 * get constant buffer list
	 */
	const BufferPointerList& model_constant_buffer_list() const { return model_constant_buffer_list_; }

	/**
	 * get shader list
	 */
	const ShaderList& shader_list() const { return shader_list_; }

	/**
	 * get shader list
	 */
	ShaderList& mutable_shader_list() { return shader_list_; }

	/**
	 * get input layout
	 */
	ID3D11InputLayout *input_layout_pointer() { return input_layout_pointer_; }

	/**
	 * create shader input layout
	 * @param [in] device_pointer directx11 device]
	 * @param [in] shader target vertex shader
	 * @param [in] type shader type
	 * @retval created ID3D11InputLayout
	 */
	ID3D11InputLayout *create_input_layout(
		ID3D11Device *device_pointer, 
		UMDirectX11ShaderPtr shader, 
		ShaderType type);

private:
	ID3D11InputLayout *input_layout_pointer_;
	D3D_FEATURE_LEVEL feature_level_;
	ShaderList shader_list_;
	BufferPointerList constant_buffer_list_;
	BufferPointerList model_constant_buffer_list_;
};

} // umdraw
