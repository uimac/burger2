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

#include <memory>
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <string>
#include "UMMacro.h"

namespace umdraw
{

class UMDirectX11Shader;
typedef std::shared_ptr<UMDirectX11Shader> UMDirectX11ShaderPtr;

/**
 * a shader
 */
class UMDirectX11Shader 
{
	DISALLOW_COPY_AND_ASSIGN(UMDirectX11Shader);
public:

	/// shader type
	enum ShaderType {
		vs, /// vertex shader
		ps, /// pixel shader
		gs, /// geometry shader
		cs, /// compute shader
		ds, /// domain shader
		hs, /// hull shader
	};

	UMDirectX11Shader();

	~UMDirectX11Shader();

	/**
	 * create shader from memory
	 */
	bool create_shader_from_memory(
		ID3D11Device *device_pointer,
		const std::string& shader_str, 
		const std::string& entry_point_str,
		const ShaderType type);

	/**
	 * create shader from file
	 */
	bool create_shader_from_file(
		ID3D11Device *device_pointer,
		const std::u16string& file_path,
		const std::string& entry_point_str,
		const ShaderType type);

	/**
	 * get shader type
	 */
	ShaderType shader_type() const { return shader_type_; }
	
	/**
	 * get shader buffer point
	 */
	const void* buffer_pointer() const { return blob_pointer_->GetBufferPointer(); }

	/**
	 * get size of shader buffer pointer 
	 */
	const size_t buffer_size() const { return blob_pointer_->GetBufferSize(); }

	/**
	 * get vertex shader if exists
	 */
	ID3D11VertexShader *vertex_shader_pointer() const { return vertex_shader_pointer_; }

	/**
	 * get pixel shader if exists
	 */
	ID3D11PixelShader *pixel_shader_pointer() const { return pixel_shader_pointer_; }
	
	/**
	 * get compute shader if exists
	 */
	ID3D11ComputeShader *compute_shader_pointer() const { return compute_shader_pointer_; }
	
	/**
	 * get domain shader if exists
	 */
	ID3D11DomainShader *domain_shader_pointer() const { return domain_shader_pointer_; }

	/**
	 * get hull shader if exists
	 */
	ID3D11HullShader *hull_shader_pointer() const { return hull_shader_pointer_; }

private:
	D3D_FEATURE_LEVEL feature_level_;
	ShaderType shader_type_;
	ID3DBlob* blob_pointer_;
	ID3D11VertexShader *vertex_shader_pointer_;
	ID3D11PixelShader *pixel_shader_pointer_;
	ID3D11ComputeShader *compute_shader_pointer_;
	ID3D11DomainShader *domain_shader_pointer_;
	ID3D11HullShader *hull_shader_pointer_;
	
	/**
	 * get valid shader version
	 */
	const std::string& get_valid_shader_version(UMDirectX11Shader::ShaderType type) const;
};

} // umdraw
