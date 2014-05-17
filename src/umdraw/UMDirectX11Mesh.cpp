/**
 * @file UMDirectX11Mesh.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_DIRECTX

#include "UMDirectX11Mesh.h"
#include "UMDirectX11ShaderManager.h"
#include "UMDirectX11DrawParameter.h"
#include "UMMacro.h"

namespace umdraw
{

UMDirectX11Mesh::UMDirectX11Mesh()
	:
	index_buffer_pointer_(NULL),
	vertex_buffer_pointer_(NULL),
	normal_buffer_pointer_(NULL),
	vertex_color_buffer_pointer_(NULL),
	uv_buffer_pointer_(NULL)
{}

UMDirectX11Mesh::~UMDirectX11Mesh()
{
	SAFE_RELEASE(index_buffer_pointer_);
	SAFE_RELEASE(vertex_buffer_pointer_);
	SAFE_RELEASE(normal_buffer_pointer_);
	SAFE_RELEASE(vertex_color_buffer_pointer_);
	SAFE_RELEASE(uv_buffer_pointer_);
}

/** 
 * initialize
 */
bool UMDirectX11Mesh::init(ID3D11Device *device_pointer)
{
	if (!device_pointer) return false;
	return true;
}
	
/** 
 * update
 */
bool UMDirectX11Mesh::update(ID3D11Device *device_pointer)
{
	if (!device_pointer) return false;
	return true;
}
	
/**
 * draw
 */
void UMDirectX11Mesh::draw(ID3D11Device* device_pointer, umdraw::UMDirectX11DrawParameterPtr parameter)
{
	if (!device_pointer) return;
	
	UMDirectX11LightPtr light;
	UMDirectX11CameraPtr camera;
	UMDirectX11ShaderManagerPtr shader_manager;
	if (parameter)
	{
		light = parameter->light();
		camera = parameter->camera();
		shader_manager = parameter->shader_manager();
	}
	if (draw_parameter_)
	{
		light = draw_parameter_->light();
		camera = draw_parameter_->camera();
		shader_manager = draw_parameter_->shader_manager();
	}
	
	if (!shader_manager) return;
	ID3D11DeviceContext *device_context_pointer = NULL;
	device_pointer->GetImmediateContext(&device_context_pointer);
	
	//if (camera || light)
	//{
	//	// apply constant buffer
	//	const UMDirectX11ShaderManager::BufferPointerList& constant_buffer = shader_manager->constant_buffer_list();
	//	if (!constant_buffer.empty())
	//	{
	//		// put camera to constant buffer
	//		if (camera)
	//		{
	//			ID3D11Buffer* constant = constant_buffer[0];
	//			device_context_pointer->UpdateSubresource(constant, 0, NULL, &(*camera), 0, 0);
	//			device_context_pointer->VSSetConstantBuffers( 0, 1, &constant );
	//		}

	//		// put light to constant buffer
	//		if (light)
	//		{
	//			ID3D11Buffer* constant = constant_buffer[1];
	//			device_context_pointer->UpdateSubresource(constant, 0, NULL, &(*light), 0, 0);
	//			device_context_pointer->VSSetConstantBuffers( 1, 1, &constant );
	//			device_context_pointer->PSSetConstantBuffers( 1, 1, &constant );
	//		}
	//	} 
	//}

	UINT stride = sizeof(UMVec3f);
	UINT uv_stride = sizeof(UMVec2f);
	UINT vertex_offset = 0;
	UINT index_offset = 0;
		
	device_context_pointer->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	device_context_pointer->IASetVertexBuffers(0, 1, p_vertex_buffer_pointer(), &stride, &vertex_offset);
	device_context_pointer->IASetVertexBuffers(1, 1, p_normal_buffer_pointer(), &stride, &vertex_offset);
	device_context_pointer->IASetVertexBuffers(2, 1, p_uv_buffer_pointer(), &uv_stride, &vertex_offset);
	if (index_buffer_pointer())
	{
		device_context_pointer->IASetIndexBuffer(index_buffer_pointer(), DXGI_FORMAT_R32_UINT, 0);
	}

	UMDirectX11MaterialList::const_iterator mt = material_list().begin();
	for (; mt != material_list().end(); ++mt)
	{
		UMDirectX11MaterialPtr material = *mt;
	
		// apply constant buffer
		const UMDirectX11ShaderManager::BufferPointerList& constant_buffer = shader_manager->model_constant_buffer_list();
		if (!constant_buffer.empty())
		{
			// set shader flags
			if (material->diffuse_texture() && 
				material->diffuse_texture()->sampler_state_pointer())
			{
				// uv on
				material->set_shader_flags(UMVec4f(1.0f, 0.0f, 0.0f, 0.0f));
			}
			else
			{
				// uv off
				material->set_shader_flags(UMVec4f(-1.0f, 0.0f, 0.0f, 0.0f));
			}

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
			if (UMDirectX11TexturePtr texture = material->diffuse_texture())
			{
				if (ID3D11SamplerState * sampler_state_pointer = texture->sampler_state_pointer())
				{
					device_context_pointer->PSSetSamplers( 0, 1, &sampler_state_pointer );
				}
				if (ID3D11ShaderResourceView* output_view = texture->texture_view_pointer())
				{
					device_context_pointer->PSSetShaderResources( 0, 1, &output_view );
				}
			}

			// apply compute shader
			device_context_pointer->CSSetShader( NULL, NULL, 0 );

			// draw
			//UINT index_count = material->polygon_count() * 3;
			UINT index_count = material->polygon_count() * 3;
			
			if (index_buffer_pointer())
			{
				device_context_pointer->DrawIndexed(index_count, index_offset, 0 );
			}
			else
			{
				device_context_pointer->Draw(index_count, index_offset);
			}

			index_offset += index_count;
		}
	}

	SAFE_RELEASE(device_context_pointer);
}

} // umdraw

#endif // WITH_DIRECTX
