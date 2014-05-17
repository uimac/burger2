/**
 * @file UMDirectX11Line.h
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
#include <string>
#include <vector>
#include "UMMacro.h"

namespace umdraw
{

class UMDirectX11Line;
typedef std::shared_ptr<UMDirectX11Line> UMDirectX11LinePtr;
typedef std::vector<UMDirectX11LinePtr> UMDirectX11LineList;

class UMDirectX11Material;
typedef std::shared_ptr<UMDirectX11Material> UMDirectX11MaterialPtr;
typedef std::vector<UMDirectX11MaterialPtr> UMDirectX11MaterialList;

class UMDirectX11DrawParameter;
typedef std::shared_ptr<UMDirectX11DrawParameter> UMDirectX11DrawParameterPtr;

/**
 * a shader
 */
class UMDirectX11Line 
{
	DISALLOW_COPY_AND_ASSIGN(UMDirectX11Line);
public:

	UMDirectX11Line();

	~UMDirectX11Line();
	
	/** 
	 * initialize
	 */
	bool init(ID3D11Device *device_pointer);
	
	/**
	 * update
	 */
	bool update(ID3D11Device* device_pointer);

	/**
	 * draw
	 */
	void draw(ID3D11Device* device_pointer, UMDirectX11DrawParameterPtr parameter);
	
	/**
	 * get vertex buffer pointer pointer
	 */
	ID3D11Buffer **p_vertex_buffer_pointer() { return &vertex_buffer_pointer_; }

	/**
	 * get vertex buffer pointer
	 */
	ID3D11Buffer *vertex_buffer_pointer() { return vertex_buffer_pointer_; }
	
	/**
	 * set vertex count
	 */
	void set_vertex_count(unsigned int size) { vertex_count_ = size; }

	/**
	 * get veretx count
	 */
	unsigned int vertex_count() const { return vertex_count_; }
	
	/**
	 * get material list
	 */
	const UMDirectX11MaterialList& material_list() const { return material_list_; }
	
	/**
	 * get material list
	 */
	UMDirectX11MaterialList& mutable_material_list() { return material_list_; }

private:
	ID3D11Buffer *vertex_buffer_pointer_;
	unsigned int vertex_count_;
	UMDirectX11MaterialList material_list_;
};

} // umdraw
