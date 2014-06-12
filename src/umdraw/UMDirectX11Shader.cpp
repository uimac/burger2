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

#include "UMDirectX11Shader.h"
#include <dxgi.h>
#include <d3d11.h>
#include <string>
#include <d3dcompiler.h>
#include <D3DX11async.h>

#include "UMMacro.h"
#include "UMShaderEntry.h"
#include "UMStringUtil.h"

namespace
{

	// shader version strs
	// VS
	const std::string vs_5_0("vs_5_0");
	const std::string vs_4_1("vs_4_1");
	const std::string vs_4_0("vs_4_0");
	const std::string vs_4_0_9_3("vs_4_0_level_9_3");
	const std::string vs_4_0_9_1("vs_4_0_level_9_1");
	// PS
	const std::string ps_5_0("ps_5_0");
	const std::string ps_4_1("ps_4_1");
	const std::string ps_4_0("ps_4_0");
	const std::string ps_4_0_9_3("ps_4_0_level_9_3");
	const std::string ps_4_0_9_1("ps_4_0_level_9_1");
	// CS
	const std::string cs_5_0("cs_5_0");
	const std::string cs_4_1("cs_4_1");
	const std::string cs_4_0("cs_4_0");
	// GS
	const std::string gs_5_0("gs_5_0");
	const std::string gs_4_1("gs_4_1");
	const std::string gs_4_0("gs_4_0");
	// DS
	const std::string ds_5_0("ds_5_0");
	// HS
	const std::string hs_5_0("hs_5_0");
	// none
	const std::string shader_none("");
	
	class ShaderInclude : public ID3D10Include {
	public:
		ShaderInclude(){}
		~ShaderInclude(){}
		virtual HRESULT __stdcall  Open(
					D3D10_INCLUDE_TYPE inctype,
					LPCSTR filename,
					LPCVOID parentdata,
					LPCVOID* ppdata,
					UINT* pbyte )
		{
			const std::string& shader = umdraw::UMShaderEntry::instance().dx_include_shader();
			if (!shader.empty())
			{
				*ppdata= shader.c_str();
				*pbyte= static_cast<unsigned int>(shader.size());
				return	S_OK;
			}
			return	E_FAIL;
		}
		virtual HRESULT __stdcall  Close( LPCVOID ppdata )
		{
			return  S_OK;
		}
	};
	
	
	ShaderInclude shader_include;

} // anonymouse namespace

namespace umdraw
{

/// constructor
UMDirectX11Shader::UMDirectX11Shader()
	: feature_level_(D3D_FEATURE_LEVEL_9_1),
	blob_pointer_(NULL),
	vertex_shader_pointer_(NULL),
	pixel_shader_pointer_(NULL),
	compute_shader_pointer_(NULL),
	domain_shader_pointer_(NULL),
	hull_shader_pointer_(NULL)
	{}

/// destructor
UMDirectX11Shader::~UMDirectX11Shader()
{
	SAFE_RELEASE(blob_pointer_);
	SAFE_RELEASE(vertex_shader_pointer_);
	SAFE_RELEASE(pixel_shader_pointer_);
	SAFE_RELEASE(compute_shader_pointer_);
	SAFE_RELEASE(domain_shader_pointer_);
	SAFE_RELEASE(hull_shader_pointer_);
}

/**
 * get valid shader version
 */
const std::string& UMDirectX11Shader::get_valid_shader_version(UMDirectX11Shader::ShaderType type) const
{
	D3D_FEATURE_LEVEL level = feature_level_;
	if (level == D3D_FEATURE_LEVEL_11_0)
	{
		if (type == UMDirectX11Shader::vs) { return vs_5_0; }
		if (type == UMDirectX11Shader::ps) { return ps_5_0; }
		if (type == UMDirectX11Shader::cs) { return cs_5_0; }
		if (type == UMDirectX11Shader::gs) { return gs_5_0; }
		if (type == UMDirectX11Shader::ds) { return ds_5_0; }
		if (type == UMDirectX11Shader::hs) { return hs_5_0; }
	}
	if (level == D3D_FEATURE_LEVEL_10_1)
	{
		if (type == UMDirectX11Shader::vs) { return vs_4_1; }
		if (type == UMDirectX11Shader::ps) { return ps_4_1; }
		if (type == UMDirectX11Shader::cs) { return cs_4_1; }
		if (type == UMDirectX11Shader::gs) { return gs_4_1; }
	}
	if (level == D3D_FEATURE_LEVEL_10_0)
	{
		if (type == UMDirectX11Shader::vs) { return vs_4_0; }
		if (type == UMDirectX11Shader::ps) { return ps_4_0; }
		if (type == UMDirectX11Shader::cs) { return cs_4_0; }
		if (type == UMDirectX11Shader::gs) { return gs_4_0; }
	}
	if (level == D3D_FEATURE_LEVEL_9_3)
	{
		if (type == UMDirectX11Shader::vs) { return vs_4_0_9_3; }
		if (type == UMDirectX11Shader::ps) { return ps_4_0_9_3; }
	}
	if (level == D3D_FEATURE_LEVEL_9_2)
	{
		if (type == UMDirectX11Shader::vs) { return vs_4_0_9_1; }
		if (type == UMDirectX11Shader::ps) { return ps_4_0_9_1; }
	}
	if (level == D3D_FEATURE_LEVEL_9_1)
	{
		if (type == UMDirectX11Shader::vs) { return vs_4_0_9_1; }
		if (type == UMDirectX11Shader::ps) { return ps_4_0_9_1; }
	}
	return shader_none; //defeult empty
}

/**
 * create shader from string
 */
bool UMDirectX11Shader::create_shader_from_memory(
	ID3D11Device *device_pointer,
	const std::string& shader_str, 
	const std::string& entry_point_str,
	const ShaderType type)
{
	if (!device_pointer) return false;

	feature_level_ = device_pointer->GetFeatureLevel();

	ID3DBlob* error_blob(NULL);

	HRESULT hr = D3DX11CompileFromMemory(
		shader_str.data(), 
		shader_str.size(), 
		NULL, // shader filename
		NULL, // macro
		&shader_include, // include
		// entry point function name
		entry_point_str.c_str(),
		// target 
		get_valid_shader_version(type).c_str(),
		// effect compile option
		D3D10_SHADER_OPTIMIZATION_LEVEL1,
		0,
		// threadpump
		NULL,
		(LPD3DBLOB*)&blob_pointer_,
		(LPD3DBLOB*)&error_blob,
		NULL);

	if ( FAILED(hr) )
	{
		return false;
	}

	if (error_blob) {
		error_blob->Release();
		error_blob = NULL;
	}
	ID3DBlob* blob = blob_pointer_;

	if (type == UMDirectX11Shader::vs) {
		hr = device_pointer->CreateVertexShader(
			blob->GetBufferPointer(), 
			blob->GetBufferSize(), NULL, &vertex_shader_pointer_);
		if (FAILED(hr)) {
			return false;
		}
	} else if (type == UMDirectX11Shader::ps) {
		hr = device_pointer->CreatePixelShader(
			blob->GetBufferPointer(), 
			blob->GetBufferSize(), NULL, &pixel_shader_pointer_);
		if (FAILED(hr)) {
			return false;
		}
	} else if (type == UMDirectX11Shader::cs) {
		hr = device_pointer->CreateComputeShader(
			blob->GetBufferPointer(), 
			blob->GetBufferSize(), NULL, &compute_shader_pointer_);
		if (FAILED(hr)) {
			return false;
		}
	} else if (type == UMDirectX11Shader::ds) {
		hr = device_pointer->CreateDomainShader(
			blob->GetBufferPointer(), 
			blob->GetBufferSize(), NULL, &domain_shader_pointer_);
		if (FAILED(hr)) {
			return false;
		}
	} else if (type == UMDirectX11Shader::hs) {
		hr = device_pointer->CreateHullShader(
			blob->GetBufferPointer(), 
			blob->GetBufferSize(), NULL, &hull_shader_pointer_);
		if (FAILED(hr)) {
			return false;
		}
	}

	return true;
}

/**
 * create shader from file
 */
bool UMDirectX11Shader::create_shader_from_file(
	ID3D11Device *device_pointer,
	const umstring& file_path, 
	const std::string& entry_point_str,
	const ShaderType type)
{
	if (!device_pointer) return false;
	
	feature_level_ = device_pointer->GetFeatureLevel();

	ID3DBlob* error_blob(NULL);

	HRESULT hr = D3DX11CompileFromFileW(
		umbase::UMStringUtil::utf16_to_wstring(file_path).c_str(), 
		NULL, 
		NULL, // include
		//D3D_COMPILE_STANDARD_FILE_INCLUDE, // include
		// entry point function name
		entry_point_str.c_str(),
		// target 
		get_valid_shader_version(type).c_str(),
		// flag1: shader compile option
		D3D10_SHADER_OPTIMIZATION_LEVEL1,
		// flag2:effect compile option
		0,
		// threadpump
		NULL,
		(LPD3DBLOB*)&blob_pointer_,
		(LPD3DBLOB*)&error_blob,
		NULL);

	if ( FAILED(hr) )
	{
		return false;
	}

	if (error_blob) {
		error_blob->Release();
		error_blob = NULL;
	}

	ID3DBlob* blob = blob_pointer_;

	if (type == UMDirectX11Shader::vs) {
		hr = device_pointer->CreateVertexShader(
			blob->GetBufferPointer(), 
			blob->GetBufferSize(), NULL, &vertex_shader_pointer_);
		if (FAILED(hr)) {
			return false;
		}
	} else if (type == UMDirectX11Shader::ps) {
		hr = device_pointer->CreatePixelShader(
			blob->GetBufferPointer(), 
			blob->GetBufferSize(), NULL, &pixel_shader_pointer_);
		if (FAILED(hr)) {
			return false;
		}
	} else if (type == UMDirectX11Shader::cs) {
		hr = device_pointer->CreateComputeShader(
			blob->GetBufferPointer(), 
			blob->GetBufferSize(), NULL, &compute_shader_pointer_);
		if (FAILED(hr)) {
			return false;
		}
	} else if (type == UMDirectX11Shader::ds) {
		hr = device_pointer->CreateDomainShader(
			blob->GetBufferPointer(), 
			blob->GetBufferSize(), NULL, &domain_shader_pointer_);
		if (FAILED(hr)) {
			return false;
		}
	} else if (type == UMDirectX11Shader::hs) {
		hr = device_pointer->CreateHullShader(
			blob->GetBufferPointer(), 
			blob->GetBufferSize(), NULL, &hull_shader_pointer_);
		if (FAILED(hr)) {
			return false;
		}
	}
	return true;
}


} // umdraw

#endif // WITH_DIRECTX
