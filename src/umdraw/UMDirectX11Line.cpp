/**
 * @file UMDirectX11Line.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_DIRECTX

#include "UMDirectX11Line.h"
#include <dxgi.h>
#include <d3d11.h>
#include <string>

#include "UMMacro.h"
#include "UMStringUtil.h"
#include "UMDirectX11Material.h"
#include "UMDirectX11ShaderManager.h"
#include "UMDirectX11DrawParameter.h"

namespace umdraw
{

/// constructor
UMDirectX11Line::UMDirectX11Line()
	: vertex_count_(0)
{}

/// destructor
UMDirectX11Line::~UMDirectX11Line()
{
	SAFE_RELEASE(vertex_buffer_pointer_);
}

/** 
	* initialize
	*/
bool UMDirectX11Line::init(ID3D11Device *device_pointer)
{
	return true;
}
	
/**
 * update
 */
bool UMDirectX11Line::update(ID3D11Device* device_pointer)
{
	return true;
}

/**
 * draw
 */
void UMDirectX11Line::draw(ID3D11Device* device_pointer, UMDirectX11DrawParameterPtr parameter)
{
	if (!device_pointer) return;
	
	UMDirectX11ShaderManagerPtr shader_manager;
	if (parameter)
	{
		shader_manager = parameter->shader_manager();
	}
	if (!shader_manager) return;

	ID3D11DeviceContext *device_context_pointer = NULL;
	device_pointer->GetImmediateContext(&device_context_pointer);
	
	UINT stride = sizeof(UMVec3f);
	UINT uv_stride = sizeof(UMVec2f);
	UINT vertex_offset = 0;
	UINT index_offset = 0;
		
	device_context_pointer->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	device_context_pointer->IASetVertexBuffers(0, 1, p_vertex_buffer_pointer(), &stride, &vertex_offset);
	ID3D11Buffer* null_buffer = NULL;
	device_context_pointer->IASetVertexBuffers(1, 1, &null_buffer, &stride, &vertex_offset);
	device_context_pointer->IASetVertexBuffers(2, 1, &null_buffer, &uv_stride, &vertex_offset);
	device_context_pointer->IASetIndexBuffer(null_buffer, DXGI_FORMAT_R32_UINT, 0);

	UMDirectX11MaterialList::const_iterator mt = material_list().begin();
	for (; mt != material_list().end(); ++mt)
	{
		UMDirectX11MaterialPtr material = *mt;
	
		// apply constant buffer
		const UMDirectX11ShaderManager::BufferPointerList& constant_buffer = shader_manager->model_constant_buffer_list();
		if (!constant_buffer.empty())
		{
			// put material to constant buffer
			{
				ID3D11Buffer* constant = constant_buffer[0];
				device_context_pointer->UpdateSubresource(constant, 0, NULL, &(*material), 0, 0);
				device_context_pointer->VSSetConstantBuffers( 2, 1, &constant );
				device_context_pointer->PSSetConstantBuffers( 2, 1, &constant );
			}
		}

		// shader list
		const UMDirectX11ShaderManager::ShaderList& shaders = shader_manager->shader_list();

		// apply vertex shader
		if (device_context_pointer && !shaders.empty())
		{
			// apply input layout
			if (shader_manager->input_layout_pointer())
			{
				device_context_pointer->IASetInputLayout(shader_manager->input_layout_pointer());
			}

			// apply vertex shader
			device_context_pointer->VSSetShader( shaders[0]->vertex_shader_pointer(), NULL, 0 );
		
			// apply hull shader
			device_context_pointer->HSSetShader( NULL, NULL, 0 );

			// apply domain shader
			device_context_pointer->DSSetShader( NULL, NULL, 0 );

			// apply geometry shader
			device_context_pointer->GSSetShader( NULL, NULL, 0 );

			// apply pixel shader
			device_context_pointer->PSSetShader( shaders[1]->pixel_shader_pointer(), NULL, 0 );
				
			// texture
			if (ID3D11SamplerState* state = parameter->default_sampler_state())
			{
				device_context_pointer->PSSetSamplers( 0, 1, &state );
			}
			ID3D11ShaderResourceView* null_resource = NULL;
			device_context_pointer->PSSetShaderResources( 0, 1, &null_resource );

			// apply compute shader
			device_context_pointer->CSSetShader( NULL, NULL, 0 );

			// draw
			//UINT index_count = material->polygon_count() * 3;
			UINT index_count = material->polygon_count() * 2;
			
			device_context_pointer->Draw(index_count, index_offset);

			index_offset += index_count;
		}
	}

	SAFE_RELEASE(device_context_pointer);
}

} // umdraw

#endif // WITH_DIRECTX
