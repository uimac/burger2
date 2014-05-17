/**
 * @file UMTriangle.h
 * a triangle
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
#include "UMMesh.h"
#include "UMMathTypes.h"
#include "UMBox.h"
#include "UMPrimitive.h"
#include "UMRay.h"
#include "UMShaderParameter.h"

namespace umabc
{
	class UMAbcMesh;
	typedef std::shared_ptr<UMAbcMesh> UMAbcMeshPtr;
	typedef std::weak_ptr<UMAbcMesh> UMAbcMeshWeakPtr;
} // umabc

namespace umrt
{

class UMTriangle;
typedef std::shared_ptr<UMTriangle> UMTrianglePtr;
typedef std::weak_ptr<UMTriangle> UMTriangleWeakPtr;
typedef std::vector<UMTrianglePtr> UMTriangleList;

/**
 * a triangle
 */
class UMTriangle : public UMPrimitive
{
	DISALLOW_COPY_AND_ASSIGN(UMTriangle);
public:
	
	/// create
	static UMTrianglePtr create(umdraw::UMMeshPtr mesh, const UMVec3i& vertex_index, int face_index);

	/// create
	static UMTrianglePtr create_from_abc_mesh(umabc::UMAbcMeshPtr mesh, const UMVec3i& vertex_index, int face_index);

	UMTriangle() :
		vertex_index_(0),
		face_index_(0)
		{}

	~UMTriangle() {}
	
	/**
	 * ray triangle intersection static version
	 * @param [in] v1 vertex 1
	 * @param [in] v2 vertex 2
	 * @param [in] v3 vertex 3
	 * @param [in] ray a ray
	 * @param [in,out] parameter shading parameters
	 * @param [out] barycentric barycentric coordinate value
	 */
	static bool intersects(
		const UMVec3d& v1,
		const UMVec3d& v2,
		const UMVec3d& v3,
		const UMRay& ray, 
		UMShaderParameter& parameter);

	/**
	 * ray triangle intersection static version
	 * @param [in] v1 vertex 1
	 * @param [in] v2 vertex 2
	 * @param [in] v3 vertex 3
	 * @param [in] ray a ray
	 */
	static bool intersects(
		const UMVec3d& v1,
		const UMVec3d& v2,
		const UMVec3d& v3,
		const UMRay& ray);

	/**
	 * ray triangle intersection
	 * @param [in] ray a ray
	 * @param [in,out] parameter shading parameters
	 */
	virtual bool intersects(const UMRay& ray, UMShaderParameter& parameter) const;

	/**
	 * ray triangle intersection
	 * @param [in] ray a ray
	 */
	virtual bool intersects(const UMRay& ray) const;
	
	/**
	 * get box
	 */
	virtual const umbase::UMBox& box() const { return box_; }
	
	/**
	 * update AABB
	 */
	virtual void update_box();
	
	/**
	 * get index
	 */
	const UMVec3i& vertex_index() const { return vertex_index_; }
	
	/**
	 * set index
	 * @param [in] index source index
	 */
	void set_vertex_index(const UMVec3i& index) { vertex_index_ = index; }

	/**
	 * get face index
	 */
	int face_index() const { return face_index_; }

	/**
	 * set face index
	 */
	void set_face_index(const int face_index) { face_index_ = face_index; }

	///**
	// * get normal
	// */
	//const UMVec3d& normal() const { return normal_; }
	//
	///**
	// * set normal
	// * @param [in] normal source normal
	// */
	//void set_normal(const UMVec3d& normal) { normal_ = normal;
	
private:
	/**
	 * get mesh
	 */ 
	umdraw::UMMeshPtr mesh() const { return mesh_.lock(); }

	/**
	 * get abc mesh
	 */
	umabc::UMAbcMeshPtr abc_mesh() const { return abc_mesh_.lock(); }

	umdraw::UMMeshPtr mesh() { return mesh_.lock(); }
	umdraw::UMMeshWeakPtr mesh_;
	
	umabc::UMAbcMeshPtr abc_mesh() { return abc_mesh_.lock(); }
	umabc::UMAbcMeshWeakPtr abc_mesh_;

	UMTriangleWeakPtr self_;
	
	int face_index_;
	UMVec3i vertex_index_;
	//UMVec3d normal_;
	
	umbase::UMBox box_;
};

} // umrt
