/**
 * @file UMDirectX11Board.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_DIRECTX

#include "UMDirectX11Board.h"

#include <dxgi.h>
#include <d3d11.h>
#include "UMDirectX11Texture.h"
#include "UMDirectX11ShaderManager.h"

namespace umdraw
{
	
UMDirectX11Board::UMDirectX11Board(
	UMVec2f left_top,
	UMVec2f right_bottom,
	float z)
{
	// culling is CCW
	p1_ = UMVec3f(right_bottom.x, left_top.y, z);
	p2_ = UMVec3f(left_top.x, left_top.y, z);
	p3_ = UMVec3f(right_bottom.x, right_bottom.y, z);
	p4_ = UMVec3f(left_top.x, right_bottom.y, z);
	
	uv1_ = UMVec2f(1.0f, 0.0f);
	uv2_ = UMVec2f(0, 0);
	uv3_ = UMVec2f(1.0f, 1.0f);
	uv4_ = UMVec2f(0, 1.0f);
	
	shader_manager_ = std::make_shared<UMDirectX11ShaderManager>();
}

UMDirectX11Board::~UMDirectX11Board()
{
	SAFE_RELEASE(vertex_buffer_pointer_);
}

/** 
 * initialize
 */
bool UMDirectX11Board::init(ID3D11Device *device_pointer)
{
	if (!device_pointer) return false;
	
	// init shader manager
	shader_manager_->init(device_pointer, UMDirectX11ShaderManager::eBoard);
	
	// create vertex buffer
	D3D11_BUFFER_DESC  desc;
	::ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = (sizeof(UMVec3f) + sizeof(UMVec2f)) * 4;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	::ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
	data.pSysMem = this;
	
	ID3D11DeviceContext *device_context_pointer = NULL;
	device_pointer->GetImmediateContext(&device_context_pointer);
	if FAILED(device_pointer->CreateBuffer(&desc, &data, &vertex_buffer_pointer_))
	{
		SAFE_RELEASE(device_context_pointer);
		return false;
	}
	SAFE_RELEASE(device_context_pointer);

	return true;
}

/**
 * draw board
 */
void UMDirectX11Board::draw(
	ID3D11Device* device_pointer,
	UMDirectX11Texture& texture)
{
	if (!device_pointer) return;

	ID3D11SamplerState * sampler_state_pointer = texture.sampler_state_pointer();
	if (!sampler_state_pointer) return;
	
	ID3D11DeviceContext *device_context_pointer = NULL;
	device_pointer->GetImmediateContext(&device_context_pointer);
	
	UINT stride = sizeof(UMVec3f) + sizeof(UMVec2f);
	UINT offset = 0;
	device_context_pointer->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	device_context_pointer->IASetVertexBuffers(0, 1, &vertex_buffer_pointer_, &stride, &offset);

	// shader list
	const UMDirectX11ShaderManager::ShaderList& shaders = shader_manager_->shader_list();
	
	// apply vertex shader
	if (device_context_pointer && !shaders.empty())
	{
		// apply input layout
		if (shader_manager_->input_layout_pointer())
		{
			device_context_pointer->IASetInputLayout(shader_manager_->input_layout_pointer());
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
		device_context_pointer->PSSetSamplers( 0, 1, &sampler_state_pointer );
		ID3D11ShaderResourceView* output_view = texture.texture_view_pointer();
		device_context_pointer->PSSetShaderResources( 0, 1, &output_view );

		// apply compute shader
		device_context_pointer->CSSetShader( NULL, NULL, 0 );

		device_context_pointer->Draw( 4, 0 );
	}
	
	SAFE_RELEASE(device_context_pointer);
}

} // umdraw

#endif // WITH_DIRECTX
