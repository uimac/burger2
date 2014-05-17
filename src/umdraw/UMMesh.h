/**
 * @file UMMesh.h
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
#include "UMMaterial.h"
#include "UMMathTypes.h"
#include "UMBox.h"
#include "UMNode.h"

#include <vector>

namespace umdraw
{

class UMMesh;
typedef std::shared_ptr<UMMesh> UMMeshPtr;
typedef std::weak_ptr<UMMesh> UMMeshWeakPtr;

typedef std::vector<UMMeshPtr> UMMeshList;

/**
 * a mesh object
 */
class UMMesh : public UMNode
{
	DISALLOW_COPY_AND_ASSIGN(UMMesh);
public:
	UMMesh() {}
	~UMMesh() {}
	
	typedef std::vector<UMVec3d> Vec4dList;
	typedef std::vector<UMVec3d> Vec3dList;
	typedef std::vector<UMVec2d> Vec2dList;
	typedef std::vector<UMVec3i> Vec3iList;
	typedef std::vector<int> IndexList;
	typedef std::vector<IndexList> VertexIndexList;
	
	/**
	 * get face list
	 */
	const Vec3iList& face_list() const { return face_list_; }

	/**
	 * get face list
	 */
	Vec3iList& mutable_face_list() { return face_list_; }

	/**
	 * get vertex index list
	 */
	const VertexIndexList& vertex_index_list() const { return vertex_index_list_; }

	/**
	 * get vertex index list
	 */
	VertexIndexList& mutable_vertex_index_list() { return vertex_index_list_; }

	/** 
	 * get vertex list
	 */
	const Vec3dList& vertex_list() const { return vertex_list_; }

	/**
	 * get vertex list
	 */
	Vec3dList& mutable_vertex_list() { return vertex_list_; }

	/**
	 * get normal list
	 */
	const Vec3dList& normal_list() const { return normal_list_; }

	/**
	 * get normal list
	 */
	Vec3dList& mutable_normal_list() { return normal_list_; }

	/**
	 * get vertex color list
	 */
	const Vec4dList& vertex_color_list() const { return vertex_color_list_; }

	/**
	 * get vertex color list
	 */
	Vec4dList& mutable_vertex_color_list() { return vertex_color_list_; }

	/**
	 * get uv list
	 */
	const Vec2dList& uv_list() const { return uv_list_; }

	/**
	 * get uv list
	 */
	Vec2dList& mutable_uv_list() { return uv_list_; }

	/**
	 * get uv index list
	 */
	const IndexList& uv_index_list() const { return uv_index_list_; }

	/**
	 * get uv index list
	 */
	IndexList& mutable_uv_index_list() { return uv_index_list_; }

	/**
	 * get material list
	 */
	const UMMaterialList& material_list() const { return material_list_; }
	
	/**
	 * get material list
	 */
	UMMaterialList& mutable_material_list() { return material_list_; }

	/** 
	 * create normals
	 * @param [in] is_smooth smooth or flat
	 */
	bool create_normals(bool is_smooth);

	/**
	 * get centroid
	 */
	UMVec3d centroid() const { return box().center(); }
	
	/**
	 * get box
	 */
	const umbase::UMBox& box() const { return box_; }
	
	/**
	 * update AABB
	 */
	void update_box();
	

	/**
	 * get material from face index
	 */ 
	UMMaterialPtr material_from_face_index(int face_index) const;

private:
	Vec3iList face_list_;
	VertexIndexList vertex_index_list_;
	Vec4dList vertex_list_;
	Vec4dList normal_list_;
	Vec4dList vertex_color_list_;
	Vec2dList uv_list_;
	IndexList uv_index_list_;

	umbase::UMBox box_;
	UMMaterialList material_list_;
};

} //umdraw
