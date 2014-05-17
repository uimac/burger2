/**
 * @file UMDirectX11MeshGroup.h
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include "UMMacro.h"
#include "UMMesh.h"
#include "UMMeshGroup.h"

#include "UMDirectX11Mesh.h"
#include "UMDirectX11ShaderManager.h"

namespace umdraw
{

class UMDirectX11MeshGroup;
typedef std::shared_ptr<UMDirectX11MeshGroup> UMDirectX11MeshGroupPtr;

typedef std::vector<UMDirectX11MeshGroupPtr> UMDirectX11MeshGroupList;

class UMDirectX11DrawParameter;
typedef std::shared_ptr<UMDirectX11DrawParameter> UMDirectX11DrawParameterPtr;

/**
 * directx11 buffers of single model data
 */
class UMDirectX11MeshGroup 
{
	DISALLOW_COPY_AND_ASSIGN(UMDirectX11MeshGroup);
public:

	UMDirectX11MeshGroup() 
		: shader_manager_(std::make_shared<UMDirectX11ShaderManager>())
	{}

	~UMDirectX11MeshGroup() {}
	
	/**
	 * init mesh group
	 */
	bool init(ID3D11Device *device_pointer);
	
	/**
	 * update mesh group
	 */
	bool update(ID3D11Device *device_pointer);

	/**
	 * draw mesh group
	 */
	void draw(ID3D11Device* device_pointer, UMDirectX11DrawParameterPtr parameter);

	/**
	 * get directx11 mesh list
	 */
	const UMDirectX11MeshList& dx11_mesh_list() const { return dx11_mesh_list_; }
	
	/**
	 * get directx11 mesh list
	 */
	UMDirectX11MeshList& mutable_dx11_mesh_list() { return dx11_mesh_list_; }
	
	/**
	 * set ummesh group
	 */
	void set_ummesh_group(UMMeshGroupPtr ummesh_group) { ummesh_group_ = ummesh_group; }

	/**
	 * get ummesh group
	 */
	UMMeshGroupPtr ummesh_group() { return ummesh_group_.lock(); }
	
	/**
	 * set draw parameter
	 */
	void set_draw_parameter(UMDirectX11DrawParameterPtr parameter) { draw_parameter_ = parameter; }
private:
	UMDirectX11ShaderManagerPtr shader_manager_;

	UMDirectX11MeshList dx11_mesh_list_;
	
	UMMeshGroupWeakPtr ummesh_group_;
	UMDirectX11DrawParameterPtr draw_parameter_;
};

} // umdraw
