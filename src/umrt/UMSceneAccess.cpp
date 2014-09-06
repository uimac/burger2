/**
 * @file UMSceneAccess.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#include "UMSceneAccess.h"
#include <string>
#include <assert.h>
#include "UMMesh.h"
#include "UMMeshGroup.h"
#include "UMMaterial.h"
#include "UMImage.h"
#include "UMLight.h"
#include "UMVertexParameter.h"
#include "UMBvh.h"
#include "UMPrimitive.h"
#include "UMTriangle.h"
#include "UMBvh.h"
#include "UMSubdivision.h"

#ifdef WITH_ALEMBIC
	#include "UMAbcScene.h"
	#include "UMAbcObject.h"
	#include "UMAbcMesh.h"
	#include "UMAbcIO.h"
#endif //WITH_ALEMBIC

namespace
{
	using namespace umabc;
	using namespace umdraw;
	using namespace umrt;

	void create_triangle_and_vertex(
		UMPrimitiveList& primitive_list, 
		UMVertexParameterList& vertex_parameter_list,
		UMMeshPtr mesh)
	{
		const size_t vertex_count = mesh->vertex_list().size();
		const size_t vparam_start_index = vertex_parameter_list.size();
		vertex_parameter_list.resize(vparam_start_index + vertex_count);
		for (size_t i = 0; i < vertex_count; ++i)
		{
			UMVertexParameterPtr vp(std::make_shared<UMVertexParameter>(mesh));
			vertex_parameter_list.at(vparam_start_index + i) = vp;
		}
		
		const size_t start_index = primitive_list.size();
		if (mesh->face_list().empty())
		{
			const int face_count = static_cast<int>(vertex_count / 3);
			primitive_list.resize(start_index + face_count);
			for (int i = 0; i < face_count; ++i)
			{
				UMVec3i face(i * 3 + 0, i * 3 + 1, i * 3 + 2);
				UMTrianglePtr triangle(UMTriangle::create(mesh, face, i));
				primitive_list.at(start_index + i) = triangle;

				for (int k = 0; k < 3; ++k)
				{
					int index = face[k];
					UMVertexParameterPtr vp = vertex_parameter_list.at(vparam_start_index + index);
					vp->mutable_triangle_index_list().push_back(i);
				}
			}
		}
		else
		{
			const int face_count = static_cast<int>(mesh->face_list().size());
			primitive_list.resize(start_index + face_count);
			for (int i = 0; i < face_count; ++i)
			{
				const UMVec3i& face = mesh->face_list().at(i);
				UMTrianglePtr triangle(UMTriangle::create(mesh, face, i));
				primitive_list.at(start_index + i) = triangle;

				for (int k = 0; k < 3; ++k)
				{
					int index = face[k];
					UMVertexParameterPtr vp = vertex_parameter_list.at(vparam_start_index + index);
					vp->mutable_triangle_index_list().push_back(i);
				}
			}
		}
	}
	
#ifdef WITH_ALEMBIC
	void create_triangle_and_vertex_from_abc_mesh(
		UMPrimitiveList& primitive_list, 
		UMVertexParameterList& vertex_parameter_list,
		UMAbcMeshPtr mesh)
	{
		const size_t vertex_count = mesh->vertex()->size();
		const size_t vparam_start_index = vertex_parameter_list.size();
		vertex_parameter_list.resize(vparam_start_index + vertex_count);
		for (size_t i = 0; i < vertex_count; ++i)
		{
			UMVertexParameterPtr vp(std::make_shared<UMVertexParameter>(mesh));
			vertex_parameter_list.at(vparam_start_index + i) = vp;
		}
		
		const size_t start_index = primitive_list.size();
		if (mesh->triangle_index().empty())
		{
			const int face_count = static_cast<int>(vertex_count / 3);
			primitive_list.resize(start_index + face_count);
			for (int i = 0; i < face_count; ++i)
			{
				const UMVec3i face(i * 3 + 0, i * 3 + 2, i * 3 + 1);
				const UMVec3i iface(face.x, face.y, face.z);
				UMTrianglePtr triangle(UMTriangle::create_from_abc_mesh(mesh, iface, i));
				primitive_list.at(start_index + i) = triangle;

				for (int k = 0; k < 3; ++k)
				{
					int index = iface[k];
					UMVertexParameterPtr vp = vertex_parameter_list.at(vparam_start_index + index);
					vp->mutable_triangle_index_list().push_back(i);
				}
			}
		}
		else
		{
			const int face_count = static_cast<int>(mesh->triangle_index().size());
			primitive_list.resize(start_index + face_count);
			for (int i = 0; i < face_count; ++i)
			{
				const UMVec3ui& face = mesh->triangle_index().at(i);
				const UMVec3i iface(face.x, face.z, face.y);
				UMTrianglePtr triangle(UMTriangle::create_from_abc_mesh(mesh, iface, i));
				primitive_list.at(start_index + i) = triangle;

				for (int k = 0; k < 3; ++k)
				{
					int index = iface[k];
					UMVertexParameterPtr vp = vertex_parameter_list.at(vparam_start_index + index);
					vp->mutable_triangle_index_list().push_back(i);
				}
			}
		}
	}
	
	void create_triangle_and_vertex_from_abc(
		umabc::UMAbcMeshList& dst_abc_mesh_list, 
		UMPrimitiveList& primitive_list, 
		UMVertexParameterList& vertex_parameter_list,
		UMAbcObjectPtr object)
	{
		if (UMAbcMeshPtr mesh = std::dynamic_pointer_cast<UMAbcMesh>(object))
		{
			//if (umdraw::UMMeshPtr draw_mesh = umabc::UMAbcIO::convert_abc_mesh_to_mesh(mesh))
			{
				create_triangle_and_vertex_from_abc_mesh(primitive_list, vertex_parameter_list, mesh);
				dst_abc_mesh_list.push_back(mesh);
			}
		}
		UMAbcObjectList::const_iterator it = object->children().begin();
		for (; it != object->children().end(); ++it)
		{
			create_triangle_and_vertex_from_abc(
				dst_abc_mesh_list,
				primitive_list, 
				vertex_parameter_list,
				*it);
		}
	}
#endif // WITH_ALEMBIC

	bool subdivide_mesh(umdraw::UMMeshPtr mesh)
	{
		return false;
	}
}

namespace umrt
{
	using namespace umdraw;

/**
 * constructor
 */
UMSceneAccess::UMSceneAccess()
{
	bvh_ = UMBvh::create();
}

/**
 * init
 */
bool UMSceneAccess::init()
{
	mutable_render_primitive_list().clear();
	mutable_vertex_parameter_list().clear();
	mutable_primitive_list().clear();
	return true;
}

/**
 * add scene
 */
void UMSceneAccess::add_scene(umdraw::UMScenePtr scene)
{
	if (!scene) return;
	scene_ = scene;
	const UMMeshGroupList& group_list = scene_->mesh_group_list();
	for (UMMeshGroupList::const_iterator it = group_list.begin();
		it != group_list.end();
		++it)
	{
		UMMeshGroupPtr group = *it;
		const UMMeshList& mesh_list = group->mesh_list();
		for (UMMeshList::const_iterator mt = mesh_list.begin();
			mt != mesh_list.end();
			++mt)
		{
			UMMeshPtr mesh = *mt;
			create_triangle_and_vertex(
				mutable_primitive_list(), 
				mutable_vertex_parameter_list(),
				mesh);
		}
	}
	bool added = true;
}

/**
 * add abc scene
 */
void UMSceneAccess::add_abc_scene(umabc::UMAbcScenePtr scene)
{
#ifdef WITH_ALEMBIC
	if (!scene) return;
	if (UMAbcObjectPtr root = scene->root_object())
	{
		create_triangle_and_vertex_from_abc(
			abc_mesh_list_,
			mutable_primitive_list(), 
			mutable_vertex_parameter_list(),
			root);
	}
	abc_scene_ = scene;
	bool added = true;
#endif
}

/**
 * subdivide mesh
 */
bool UMSceneAccess::subdivide(unsigned int id, unsigned int level)
{
	if (id == 0) return false;
	if (level == 0) return false;

	if (scene_)
	{
		umdraw::UMMeshGroupList::iterator it = scene_->mutable_mesh_group_list().begin();
		for (; it != scene_->mutable_mesh_group_list().end(); ++it)
		{
			umdraw::UMMeshGroupPtr group = *it;
			umdraw::UMMeshList::iterator mt = group->mutable_mesh_list().begin();
			for (; mt != group->mutable_mesh_list().end(); ++mt)
			{
				umdraw::UMMeshPtr mesh = *mt;
				if (mesh->id() == id)
				{
					UMSubdivision subdiv(mesh);
					if (umdraw::UMMeshPtr divided_mesh = subdiv.subdivided_mesh(level))
					{
						(*mt) = divided_mesh;
						return true;
					}
				}
			}
		}
	}
	return false;
}

/** 
 * update bvh
 */
bool UMSceneAccess::update_bvh()
{
	if (!bvh_) return false;
	if (!scene_) return false;

	UMPrimitiveList::iterator it = mutable_primitive_list().begin();
	for (; it != mutable_primitive_list().end(); ++it)
	{
		(*it)->update_box();
	}

	if (bvh_->build(mutable_primitive_list()))
	{
		mutable_render_primitive_list().clear();
		mutable_render_primitive_list().push_back(bvh_);
		return true;
	}
	return false;
}

	
/** 
 * generate a camera ray
 */
void UMSceneAccess::generate_ray(UMRay& ray, const UMVec2d& sample_point) const
{
	if (!scene_) return;
	UMCameraPtr camera = scene_->camera();
	if (!camera) return;

	const UMMat44d& view_projection = camera->view_projection_matrix();
	UMVec3d right (view_projection.m[0][0], view_projection.m[1][0], view_projection.m[2][0]);
	UMVec3d up (view_projection.m[0][1], view_projection.m[1][1], view_projection.m[2][1]);
	UMVec3d direction (view_projection.m[0][2], view_projection.m[1][2], view_projection.m[2][2]);
	
	const double inv_yscale = tan(umbase::um_to_radian(camera->fov_y() * 0.5));
	const double inv_xscale = camera->aspect() * inv_yscale;
	right *= inv_xscale;
	up *= inv_yscale;
	const UMVec3d generate_ray_x_scale = right * camera->aspect();
	const UMVec3d generate_ray_y_scale = up;
	const UMVec3d generate_ray_adder = direction / inv_yscale;
	
	const double inverted_width = 1.0 / static_cast<double>(scene_->width());
	const double inverted_height = 1.0 / static_cast<double>(scene_->height());
	const double xx = sample_point.x * inverted_width * 2 - 1;
	const double yy = sample_point.y * inverted_height * 2 - 1;
	UMVec3d dir = generate_ray_x_scale * xx + generate_ray_y_scale * yy + generate_ray_adder;

	ray.set_origin(camera->position());
	ray.set_direction(dir.normalized());
}

} // umrt
