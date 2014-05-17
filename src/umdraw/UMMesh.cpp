/**
 * @file UMMesh.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#include "UMMesh.h"
#include "UMMacro.h"
#include "UMVector.h"

#include <vector>

namespace
{
	using namespace umdraw;
	bool create_smooth_normals(UMMesh& mesh)
	{
		return false;
	}

	bool create_flat_normals(UMMesh& mesh)
	{
		return false;
	}
}

namespace umdraw
{
/** 
 * create normals
 * @param [in] is_smooth smooth or flat
 */
bool UMMesh::create_normals(bool is_smooth)
{
	if (vertex_list().empty()) return false;
	
	const int face_size = static_cast<int>(face_list().size());
	const int vertex_size = static_cast<int>(vertex_list().size());
	mutable_normal_list().resize(vertex_size);
	
	for (int i = 0; i < vertex_size; ++i)
	{
		mutable_normal_list().at(i) = UMVec3d(0);
	}

	if (face_size > 0)
	{
		for (int i = 0; i < face_size; ++i)
		{
			UMVec3i face = face_list().at(i);
			const UMVec3d& v0 = vertex_list().at(face.x);
			const UMVec3d& v1 = vertex_list().at(face.y);
			const UMVec3d& v2 = vertex_list().at(face.z);
			UMVec3d normal = (v0-v1).cross(v1-v2);

			mutable_normal_list().at(face.x) += normal;
			mutable_normal_list().at(face.y) += normal;
			mutable_normal_list().at(face.z) += normal;
		}
	}
	else
	{
		for (int i = 0, size = vertex_size / 3; i < size; ++i)
		{
			int vi0 = i * 3 + 0;
			int vi1 = i * 3 + 1;
			int vi2 = i * 3 + 2;
			const UMVec3d& v0 = vertex_list().at(vi0);
			const UMVec3d& v1 = vertex_list().at(vi1);
			const UMVec3d& v2 = vertex_list().at(vi2);
			UMVec3d normal = (v0-v1).cross(v1-v2);

			mutable_normal_list().at(vi0) += normal;
			mutable_normal_list().at(vi1) += normal;
			mutable_normal_list().at(vi2) += normal;
		}
	}
	
	for (int i = 0; i < vertex_size; ++i)
	{
		mutable_normal_list().at(i) = normal_list().at(i).normalized();
	}
	return false;
}

/**
 * update AABB
 */
void UMMesh::update_box()
{
	box_.set_minimum(UMVec3d(std::numeric_limits<double>::infinity()));
	box_.set_maximum(UMVec3d(-std::numeric_limits<double>::infinity()));
	
	const int vertex_size = static_cast<int>(vertex_list().size());
	for (int i = 0; i < vertex_size; ++i)
	{
		box_.extend(vertex_list().at(i));
	}
}

/** 
 * get material from vertex index
 */
UMMaterialPtr UMMesh::material_from_face_index(int face_index) const
{
	int pos = 0;
	UMMaterialList::const_iterator it = material_list_.begin();
	for (; it != material_list_.end(); ++it)
	{
		const int polygon_count = (*it)->polygon_count();
		if (face_index >= pos && face_index < (pos+polygon_count)) {
			return *it;
		}
		pos += polygon_count;
	}
	return UMMaterialPtr();
}

} //umdraw