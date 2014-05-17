/**
 * @file UMSceneAccess.h
 * accelerated scene access
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include "UMMacro.h"
#include "UMVector.h"
#include "UMMathTypes.h"
#include "UMBox.h"
#include "UMScene.h"
#include "UMPrimitive.h"
#include "UMVertexParameter.h"

namespace umdraw
{

class UMScene;
typedef std::shared_ptr<UMScene> UMScenePtr;

class UMOpenGLMaterial;
typedef std::shared_ptr<UMOpenGLMaterial> UMOpenGLMaterialPtr;

} //umdraw

namespace umabc
{
	class UMAbcScene;
	typedef std::shared_ptr<UMAbcScene> UMAbcScenePtr;
	
	class UMAbcMesh;
	typedef std::shared_ptr<UMAbcMesh> UMAbcMeshPtr;
	typedef std::vector<UMAbcMeshPtr> UMAbcMeshList;
} // umabc

namespace umrt
{

class UMSceneAccess;
typedef std::shared_ptr<UMSceneAccess> UMSceneAccessPtr;
typedef std::vector<UMSceneAccessPtr> UMSceneAccessList;

class UMBvh;
typedef std::shared_ptr<UMBvh> UMBvhPtr;

class UMSubdivision;
typedef std::shared_ptr<UMSubdivision> UMSubdivisionPtr;

/**
 * accelerated scene access
 */
class UMSceneAccess
{
	DISALLOW_COPY_AND_ASSIGN(UMSceneAccess);
public:
	UMSceneAccess();

	/**
	 * init
	 */
	bool init();

	/**
	 * add scene 
	 */
	void add_scene(umdraw::UMScenePtr scene);
	
	/**
	 * add abc scene 
	 */
	void add_abc_scene(umabc::UMAbcScenePtr abc_scene);

	/**
	 * subdivide mesh
	 */
	bool subdivide(unsigned int id, unsigned int level);
	
	/**
	 * get primitive list
	 */
	const UMPrimitiveList& primitive_list() const { return primitive_list_; }

	/**
	 * get primitive list
	 */
	UMPrimitiveList& mutable_primitive_list() { return primitive_list_; }
	
	/**
	 * get primitive list
	 */
	const UMPrimitiveList& render_primitive_list() const { return render_primitive_list_; }

	/**
	 * get primitive list
	 */
	UMPrimitiveList& mutable_render_primitive_list() { return render_primitive_list_; }
	
	/**
	 * get vertex paramter list
	 */
	const UMVertexParameterList& vertex_parameter_list() const { return vertex_parameter_list_; }
	
	/**
	 * get vertex paramter list
	 */
	UMVertexParameterList& mutable_vertex_parameter_list() { return vertex_parameter_list_; }

	/**
	 * get background color
	 */
	UMVec3d background_color() { return scene_ ? scene_->background_color() : UMVec3d(0); }
	
	/**
	 * get scene
	 */
	umdraw::UMScenePtr scene() { return scene_; }

	/**
	 * get bvh
	 */
	UMBvhPtr bvh() { return bvh_; }

	/**
	 * update bvh
	 */
	bool update_bvh();
	
	
	/** 
	 * generate a camera ray
	 * @param [out] ray generated ray
	 * @param [in] sample_point a sample point on pixel in imageplane
	 */
	void generate_ray(UMRay& ray, const UMVec2d& sample_point) const;

private:
	umdraw::UMScenePtr scene_;
	umabc::UMAbcScenePtr abc_scene_;
	umabc::UMAbcMeshList abc_mesh_list_;

	UMPrimitiveList render_primitive_list_;
	UMPrimitiveList primitive_list_;
	UMVertexParameterList vertex_parameter_list_;
	UMBvhPtr bvh_;
};

} // umrt
