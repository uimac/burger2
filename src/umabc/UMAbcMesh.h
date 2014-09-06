/**
 * @file UMAbcMesh.h
 * any
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <memory>
#include <Alembic/Abc/All.h>
#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreFactory/All.h>

#include "UMMacro.h"
#include "UMAbcObject.h"

namespace umdraw
{
	class UMMaterial;
	typedef std::shared_ptr<UMMaterial> UMMaterialPtr;
	typedef std::vector<UMMaterialPtr> UMMaterialList;
}

namespace umabc
{
	
class UMDirectX11AbcMesh;
typedef std::shared_ptr<UMDirectX11AbcMesh> UMDirectX11AbcMeshPtr;

class UMOpenGLAbcMesh;
typedef std::shared_ptr<UMOpenGLAbcMesh> UMOpenGLAbcMeshPtr;

class UMAbcMesh;
typedef std::shared_ptr<UMAbcMesh> UMAbcMeshPtr;
typedef std::weak_ptr<UMAbcMesh> UMAbcMeshWeakPtr;

class UMAbcMesh : public UMAbcObject
{
	DISALLOW_COPY_AND_ASSIGN(UMAbcMesh);
public:
	typedef std::vector<UMVec3ui > IndexList;

	/**
	 * crate instance
	 */
	static UMAbcMeshPtr create(Alembic::AbcGeom::IPolyMesh poly_mesh);

	~UMAbcMesh();

	/**
	 * initialize
	 * @param [in] recursive do children recursively
	 * @retval succsess or fail
	 */
	virtual bool init(bool recursive);
	
	/**
	 * set current time
	 * @param [in] time time
	 * @param [in] recursive do children recursively
	 */
	virtual void set_current_time(unsigned long time, bool recursive);
	
	/**
	 * update box
	 * @param [in] recursive do children recursively
	 */
	virtual void update_box(bool recursive);

	/**
	 * draw
	 * @param [in] recursive do children recursively
	 */
	virtual void draw(bool recursive, UMAbc::DrawType type);

	/**
	 * get opengl mesh
	 */
	UMOpenGLAbcMeshPtr opengl_mesh() { return opengl_mesh_; }

	/**
	 * get directx mesh
	 */
	UMDirectX11AbcMeshPtr directx_mesh() { return directx_mesh_; }

	/**
	 * get polygon count
	 */
	int polygon_count() const;
	
	/** 
	 * update mesh all
	 */
	void update_mesh_all();

	Alembic::AbcGeom::P3fArraySamplePtr vertex() { return vertex_; }
	Alembic::AbcGeom::Int32ArraySamplePtr vertex_index() { return vertex_index_; }
	Alembic::AbcGeom::Int32ArraySamplePtr face_count() { return face_count_; }
	Alembic::AbcGeom::IN3fGeomParam::Sample& normal() { return normal_; }
	Alembic::AbcGeom::IV2fGeomParam::Sample& uv() { return uv_; }
	IndexList& triangle_index() { return triangle_index_; }
	std::vector<Imath::V3f>& normals() { return original_normal_; }

	/**
	 * get material list
	 */
	const umdraw::UMMaterialList& material_list() const { return material_list_; }
	
	/**
	 * get material list
	 */
	umdraw::UMMaterialList& mutable_material_list() { return material_list_; }

	/**
	 * get material from face index
	 */
	umdraw::UMMaterialPtr material_from_face_index(int face_index) const;

	/**
	 * get faceset name list
	 */
	const std::vector<umstring>& faceset_name_list() const { return faceset_name_list_; }

	/**
	 * get faceset polycount list
	 */
	const std::vector<int>& faceset_polycount_list() const { return faceset_polycount_list_; }

	/**
	 * set cw or not
	 */
	void set_cw(bool is_cw) { is_cw_ = is_cw; }

protected:
	UMAbcMesh(Alembic::AbcGeom::IPolyMesh poly_mesh)
		: UMAbcObject(poly_mesh)
		, poly_mesh_(poly_mesh)
		, is_cw_(true)
	{}
	
	virtual UMAbcObjectPtr self_reference()
	{
		return self_reference_.lock();
	}

private:
	/** 
	 * update normal
	 */
	void update_normal();
	
	/**
	 * update uv
	 */
	void update_uv();

	/**
	 * update vertex
	 */
	void update_vertex(Alembic::AbcGeom::IPolyMeshSchema::Sample& sample);

	/**
	 * update vertex index
	 */
	void update_vertex_index(Alembic::AbcGeom::IPolyMeshSchema::Sample& sample);
	
	/**
	 * update vertex index
	 */
	void update_vertex_index_by_faceset(Alembic::AbcGeom::IPolyMeshSchema::Sample& sample);

	/**
	 * update material (face set)
	 */
	void update_material();

	Alembic::AbcGeom::IPolyMesh poly_mesh_;
	Alembic::AbcGeom::IPolyMeshSchema::Sample initial_sample_;
	Alembic::AbcGeom::IBox3dProperty initial_bounds_prop_;

	Alembic::AbcGeom::P3fArraySamplePtr vertex_;
	Alembic::AbcGeom::Int32ArraySamplePtr vertex_index_;
	Alembic::AbcGeom::Int32ArraySamplePtr face_count_;
	
	Alembic::AbcGeom::IN3fGeomParam::Sample normal_;
	Alembic::AbcGeom::IV2fGeomParam::Sample uv_;

	std::vector<Imath::V3f> original_normal_;
	std::map<std::string , Alembic::AbcGeom::IFaceSetSchema::Sample> faceset_;

	IndexList triangle_index_;
	
	UMAbcMeshWeakPtr self_reference_;
	UMOpenGLAbcMeshPtr opengl_mesh_;
	UMDirectX11AbcMeshPtr directx_mesh_;
	umdraw::UMMaterialList material_list_;
	std::vector<umstring> faceset_name_list_;
	std::vector<std::string> faceset_names_;
	std::vector<int> faceset_polycount_list_;
	std::vector<int> faceset_original_polycount_list_;
	bool is_cw_;
};

} // umabc
