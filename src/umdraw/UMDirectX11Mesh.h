/**
 * @file UMDirectX11Mesh.h
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
#include <string>
#include "UMMacro.h"
#include "UMMesh.h"

#include "UMDirectX11Material.h"
#include "UMDirectX11ShaderManager.h"

namespace umdraw
{

class UMDirectX11Mesh;
typedef std::shared_ptr<UMDirectX11Mesh> UMDirectX11MeshPtr;
typedef std::vector<UMDirectX11MeshPtr> UMDirectX11MeshList;

class UMDirectX11DrawParameter;
typedef std::shared_ptr<UMDirectX11DrawParameter> UMDirectX11DrawParameterPtr;

/**
 * directx11 buffer of a single mesh.
 */
class UMDirectX11Mesh 
{
	DISALLOW_COPY_AND_ASSIGN(UMDirectX11Mesh);
public:

	UMDirectX11Mesh();
	
	~UMDirectX11Mesh();
	
	/** 
	 * initialize
	 */
	bool init(ID3D11Device *device_pointer);
	
	/** 
	 * update
	 */
	bool update(ID3D11Device *device_pointer);
	
	/**
	 * draw
	 */
	void draw(ID3D11Device* device_pointer, umdraw::UMDirectX11DrawParameterPtr parameter);

	/**
	 * get index buffer pointer
	 */
	ID3D11Buffer **p_index_buffer_pointer() { return &index_buffer_pointer_; }

	/**
	 * get vertex buffer pointer pointer
	 */
	ID3D11Buffer **p_vertex_buffer_pointer() { return &vertex_buffer_pointer_; }

	/**
	 * get normal buffer pointer pointer
	 */
	ID3D11Buffer **p_normal_buffer_pointer() { return &normal_buffer_pointer_; }

	/**
	 * get vertex color buffer pointer pointer
	 */
	ID3D11Buffer **p_vertex_color_buffer_pointer() { return &vertex_color_buffer_pointer_; }

	/**
	 * get uv buffer pointer pointer
	 */
	ID3D11Buffer **p_uv_buffer_pointer() { return &uv_buffer_pointer_; }

	/**
	 * get index buffer pointer
	 */
	ID3D11Buffer *index_buffer_pointer() { return index_buffer_pointer_; }

	/**
	 * get vertex buffer pointer
	 */
	ID3D11Buffer *vertex_buffer_pointer() { return vertex_buffer_pointer_; }

	/**
	 * get normal buffer pointer
	 */
	ID3D11Buffer *normal_buffer_pointer() { return normal_buffer_pointer_; }

	/**
	 * get vertex color buffer pointer
	 */
	ID3D11Buffer *vertex_color_buffer_pointer() { return vertex_color_buffer_pointer_; }

	/**
	 * get uv buffer pointer
	 */
	ID3D11Buffer *uv_buffer_pointer() { return uv_buffer_pointer_; }

	/**
	 * get material list
	 */
	const UMDirectX11MaterialList& material_list() const { return material_list_; }
	
	/**
	 * get material list
	 */
	UMDirectX11MaterialList& mutable_material_list() { return material_list_; }

	/**
	 * set ummesh
	 */
	void set_ummesh(UMMeshPtr ummesh) { ummesh_ = ummesh; }

	/**
	 * get ummesh
	 */
	UMMeshPtr ummesh() { return ummesh_.lock(); }
	
	/**
	 * set draw parameter
	 */
	void set_draw_parameter(UMDirectX11DrawParameterPtr parameter) { draw_parameter_ = parameter; }

private:
	ID3D11Buffer *index_buffer_pointer_;
	ID3D11Buffer *vertex_buffer_pointer_;
	ID3D11Buffer *normal_buffer_pointer_;
	ID3D11Buffer *vertex_color_buffer_pointer_;
	ID3D11Buffer *uv_buffer_pointer_;
	UMDirectX11MaterialList material_list_;
	
	UMMeshWeakPtr ummesh_;
	UMDirectX11DrawParameterPtr draw_parameter_;
};

} // umdraw
