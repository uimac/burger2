/**
 * @file UMOpenGLAbcMesh.h
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
#include "UMAbcCallback.h"
#include "UMMathTypes.h"

namespace umdraw
{
	class UMOpenGLMaterial;
	typedef std::shared_ptr<UMOpenGLMaterial> UMOpenGLMaterialPtr;
	typedef std::vector<UMOpenGLMaterialPtr> UMOpenGLMaterialList;
	
	class UMOpenGLMesh;
	typedef std::shared_ptr<UMOpenGLMesh> UMOpenGLMeshPtr;
	typedef std::weak_ptr<UMOpenGLMesh> UMOpenGLMeshWeakPtr;

	class UMOpenGLDrawParameter;
	typedef std::shared_ptr<UMOpenGLDrawParameter> UMOpenGLDrawParameterPtr;
} // umdraw

namespace umabc
{
class UMAbcMesh;
typedef std::shared_ptr<UMAbcMesh> UMAbcMeshPtr;
typedef std::weak_ptr<UMAbcMesh> UMAbcMeshWeakPtr;

class UMOpenGLAbcMesh;
typedef std::shared_ptr<UMOpenGLAbcMesh> UMOpenGLAbcMeshPtr;

class UMOpenGLAbcMesh
{
	DISALLOW_COPY_AND_ASSIGN(UMOpenGLAbcMesh);
public:
	UMOpenGLAbcMesh(UMAbcMeshPtr abc_mesh);

	~UMOpenGLAbcMesh();

	enum VaryingType {
		eVertexVarying,
		eFaceVarying
	};

	/** 
	 * update normal
	 */
	void update_normal(
		const std::vector<UMVec3ui >& triangle_index,
		const std::vector<Imath::V3f>& normal, VaryingType type,
		bool is_cw);
	
	/**
	 * update uv
	 */
	void update_uv(
		const std::vector<UMVec3ui >& triangle_index,
		const Alembic::AbcGeom::IV2fGeomParam::Sample& uv,
		VaryingType type,
		bool is_cw);

	/**
	 * update vertex
	 */
	void update_vertex(
	const std::vector<UMVec3ui >& triangle_index, 
	const Alembic::AbcGeom::P3fArraySamplePtr& vertex,
	bool is_cw);

	/**
	 * draw
	 */
	void draw(umdraw::UMOpenGLDrawParameterPtr parameter) const;
	
	/**
	 * get name
	 */
	const std::string& name();
	
	/**
	 * get opengl mesh
	 */
	umdraw::UMOpenGLMeshPtr opengl_mesh() { return gl_mesh_; }

private:
	UMAbcMeshPtr abc_mesh() const { return abc_mesh_.lock(); }
	UMAbcMeshWeakPtr abc_mesh_;

	umdraw::UMOpenGLMeshPtr gl_mesh_;

	unsigned int triagnle_size_;
	unsigned int vertex_size_;

	std::vector<Imath::V3f> vertex_varying_temporary_v3f_;
	std::vector<Imath::V2f> vertex_varying_temporary_v2f_;
};

} // umabc
