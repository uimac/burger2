/**
 * @file UMDirectX11AbcMesh.h
 * any
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

#include <memory>
#include <Alembic/Abc/All.h>
#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreFactory/All.h>

#include "UMMacro.h"
#include "UMAbcCallback.h"
#include "UMMathTypes.h"

namespace umdraw
{
	class UMDirectX11Material;
	typedef std::shared_ptr<UMDirectX11Material> UMDirectX11MaterialPtr;
	typedef std::vector<UMDirectX11MaterialPtr> UMDirectX11MaterialList;

	class UMDirectX11Mesh;
	typedef std::shared_ptr<UMDirectX11Mesh> UMDirectX11MeshPtr;
} // umdraw

namespace umabc
{

class UMAbcMesh;
typedef std::shared_ptr<UMAbcMesh> UMAbcMeshPtr;
typedef std::weak_ptr<UMAbcMesh> UMAbcMeshWeakPtr;

class UMDirectX11AbcMesh;
typedef std::shared_ptr<UMDirectX11AbcMesh> UMDirectX11AbcMeshPtr;

class UMDirectX11AbcMesh
{
	DISALLOW_COPY_AND_ASSIGN(UMDirectX11AbcMesh);
public:
	UMDirectX11AbcMesh(UMAbcMeshPtr abc_mesh);

	~UMDirectX11AbcMesh();

	enum VaryingType {
		eVertexVarying,
		eFaceVarying
	};

	/** 
	 * update normal
	 */
	void update_normal(
		ID3D11Device* device_pointer,
		const std::vector<UMVec3ui >& triangle_index, 
		const std::vector<Imath::V3f>& normal,
		VaryingType type);
	
	/**
	 * update uv
	 */
	void update_uv(
		ID3D11Device* device_pointer, 
		const std::vector<UMVec3ui >& triangle_index,
		const Alembic::AbcGeom::IV2fGeomParam::Sample& uv, 
		VaryingType type);

	/**
	 * update vertex
	 */
	void update_vertex(
		ID3D11Device* device_pointer, 
		const std::vector<UMVec3ui >& triangle_index,
		const Alembic::AbcGeom::P3fArraySamplePtr& vertex);

	///**
	// * update vertex index
	// */
	//void update_vertex_index(ID3D11Device* device_pointer, const std::vector<UMVec3ui >& triangle_index);

	/**
	 * draw
	 */
	void draw(ID3D11Device* device_pointer) const;
	
	/**
	 * get name
	 */
	const std::string& name();

	/**
	 * get directx11 mesh
	 */
	umdraw::UMDirectX11MeshPtr directx_mesh() { return dx_mesh_; }

private:
	umdraw::UMDirectX11MeshPtr dx_mesh_;
	
	UMAbcMeshPtr abc_mesh() const { return abc_mesh_.lock(); }
	UMAbcMeshWeakPtr abc_mesh_;

	unsigned int triagnle_size_;
	unsigned int vertex_size_;

};

} // umabc
