/**
 * @file UMSoftwareIO.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#include <string>
#include <assert.h>

#include "UMSoftwareIO.h"
#include "UMStringUtil.h"
#include "UMPath.h"
#include "UMMesh.h"
#include "UMMeshGroup.h"
#include "UMMaterial.h"
#include "UMImage.h"
#include "UMLight.h"

namespace
{
	using namespace umdraw;

	double import_scale_for_debug = 1.0;
	
	UMVec2d to_um(const umio::UMVec2d& v) { return UMVec2d(v.x, v.y); }

	UMVec3d to_um(const umio::UMVec3d& v) { return UMVec3d(v.x, v.y, v.z); }

	UMVec4d to_um(const umio::UMVec4d& v) { return UMVec4d(v.x, v.y, v.z, v.w); }

	UMVec4d to_um_material_value(const umio::UMVec4d& v) { return UMVec4d(v.x, v.y, v.z, 1.0); }
	
	UMVec2i to_um(const umio::UMVec2i& v) { return UMVec2i(v.x, v.y); }

	UMVec3i to_um(const umio::UMVec3i& v) { return UMVec3i(v.x, v.y, v.z); }

	UMVec4i to_um(const umio::UMVec4i& v) { return UMVec4i(v.x, v.y, v.z, v.w); }
	
	//----------------------------------------------------------------------------

	/**
	 * load vertex index from umio to umdraw
	 */
	void load_vertex_index(UMMeshPtr mesh, const umio::UMMesh& ummesh)
	{
		const int size = static_cast<int>(ummesh.vertex_index_list().size());
		mesh->mutable_face_list().resize(size);
		for (int i = 0; i < size; ++i)
		{
			const umio::IntList& index = ummesh.vertex_index_list().at(i);
			UMVec3i umface( index.at(0), index.at(1), index.at(2) );
			mesh->mutable_face_list().at(i) = umface;
		}
		mesh->mutable_vertex_index_list().resize(size);
		for (int i = 0; i < size; ++i)
		{
			const umio::IntList& index = ummesh.vertex_index_list().at(i);
			mesh->mutable_vertex_index_list().at(i) = index;
		}
	}

	/**
	 * load vertex from umio to umdraw
	 */
	void load_vertex(UMMeshPtr mesh, const umio::UMMesh& ummesh)
	{
		const int size = static_cast<int>(ummesh.vertex_list().size());
		mesh->mutable_vertex_list().resize(size);
		for (int i = 0; i < size; ++i)
		{
			const umio::DoubleList& vertex = ummesh.vertex_list().at(i);
			UMVec3d umvertex( vertex.at(0), vertex.at(1), vertex.at(2) );
			mesh->mutable_vertex_list().at(i) = umvertex * import_scale_for_debug;
		}
	}
	
	/**
	 * load normal from umio to umdraw
	 */
	void load_normal(UMMeshPtr mesh, const umio::UMMesh& ummesh)
	{
		const int size = static_cast<int>(ummesh.normal_list().size());
		mesh->mutable_normal_list().resize(size);
		for (int i = 0; i < size; ++i)
		{
			const umio::DoubleList& normal = ummesh.normal_list().at(i);
			UMVec3d umnormal( normal.at(0), normal.at(1), normal.at(2) );
			mesh->mutable_normal_list().at(i) = umnormal;
		}
	}
	
	/**
	 * load uv  from umio to umdraw
	 */
	void load_uv(UMMeshPtr mesh, const umio::UMMesh& ummesh)
	{
		const int size = static_cast<int>(ummesh.uv_list().size());
		mesh->mutable_uv_list().resize(size);
		for (int i = 0; i < size; ++i)
		{
			const umio::DoubleList& uv = ummesh.uv_list().at(i);
			UMVec2d umuv( uv.at(0), uv.at(1) );
			mesh->mutable_uv_list().at(i) = umuv;
		}
	}
	
	/**
	 * load material from umio to umdraw
	 */
	void load_material(const std::u16string& absolute_file_path, UMMeshPtr mesh, const umio::UMMesh& ummesh)
	{
		const int size = static_cast<int>(ummesh.material_list().size());
		mesh->mutable_material_list().resize(size);

		for (int i = 0; i < size; ++i)
		{
			const umio::UMMaterial& material = ummesh.material_list().at(i);
			UMMaterialPtr ummaterial = std::make_shared<UMMaterial>();
			ummaterial->set_ambient(to_um_material_value(material.ambient()));
			ummaterial->set_diffuse(to_um_material_value(material.diffuse()));
			ummaterial->set_specular(to_um_material_value(material.specular()));
			ummaterial->set_emissive(to_um_material_value(material.emissive()));
			ummaterial->set_refrection(to_um_material_value(material.refrection()));
			ummaterial->set_transparent(to_um_material_value(material.transparent()));
			ummaterial->set_shininess(material.shininess());
			ummaterial->set_transparency_factor(material.transparency_factor());
			ummaterial->set_reflection_factor(material.reflection_factor());
			ummaterial->set_diffuse_factor(material.diffuse_factor());
			ummaterial->set_specular_factor(material.specular_factor());
			ummaterial->set_emissive_factor(material.emissive_factor());
			ummaterial->set_ambient_factor(material.ambient_factor());
			
			// load texture path
			const int texture_count = static_cast<int>(material.texture_list().size());
			for (int k = 0; k < texture_count; ++k)
			{
				const umio::UMTexture& texture = material.texture_list().at(k);
				const std::string& absolute_or_relative_file_name = texture.file_name();
				std::u16string str = umbase::UMStringUtil::utf8_to_utf16(absolute_or_relative_file_name);
				std::u16string file_name = umbase::UMPath::get_file_name(str);
				std::u16string path = umbase::UMPath::get_absolute_path(absolute_file_path, file_name);
				ummaterial->mutable_texture_path_list().push_back(path);
			}

			const int polygon_count = static_cast<int>(std::count(
				ummesh.material_index_list().begin(), 
				ummesh.material_index_list().end(),
				i));

			ummaterial->set_polygon_count(polygon_count);
			mesh->mutable_material_list().at(i) = ummaterial;
		}

		// default material
		if (size == 0)
		{
			mesh->mutable_material_list().resize(1);
			UMMaterialPtr ummaterial = UMMaterial::default_material();
			const int polygon_count = static_cast<int>(ummesh.vertex_index_list().size());
			ummaterial->set_polygon_count(polygon_count);
			mesh->mutable_material_list().at(0) = ummaterial;
		}
	}

	//----------------------------------------------------------------------------
	
	typedef std::pair<int,int> IndexPair;

	/**
	 * sort umio mesh elements by material
	 */
	void sort_by_material(umio::UMMesh& src_mesh)
	{
		// sort material index with index
		size_t index_size = src_mesh.material_index_list().size();
		std::vector< IndexPair > index_pair_list;
		if (index_size > 0)
		{
			// has material index
			index_pair_list.resize(index_size);
			for (size_t i = 0; i < index_size; ++i)
			{
				index_pair_list[i] = IndexPair(src_mesh.material_index_list().at(i), static_cast<int>(i));
			}
			std::sort(index_pair_list.begin(), index_pair_list.end());
		}
		else
		{
			// one material and no material index
			index_size = src_mesh.vertex_index_list().size();
			index_pair_list.resize(index_size);
			for (size_t i = 0; i < index_size; ++i)
			{
				index_pair_list[i] = IndexPair(0, static_cast<int>(i));
			}
		}

		bool is_vertex_sized_normal = src_mesh.vertex_list().size() == src_mesh.normal_list().size();
		
		// store vertex index and material index by material order
		umio::IntListVec sorted_vertex_index;
		umio::IntList sorted_material_index;
		umio::DoubleListVec sorted_uv;
		umio::DoubleListVec sorted_normal;
		sorted_vertex_index.resize(index_size);
		sorted_material_index.resize(index_size);
		if (!src_mesh.uv_list().empty())
		{
			sorted_uv.resize(index_size * 3);
		}
		if (!is_vertex_sized_normal)
		{
			sorted_normal.resize(index_size * 3);
		}

		for (size_t i = 0; i < index_size; ++i)
		{
			IndexPair& pair = index_pair_list[i];
			sorted_material_index[i] = pair.first;
			sorted_vertex_index[i] = src_mesh.vertex_index_list().at(pair.second);
			if (!src_mesh.uv_list().empty())
			{
				for (int k = 0; k < 3; ++k)
				{
					sorted_uv[i * 3 + k] = src_mesh.uv_list().at(pair.second * 3 + k);
				}
			}
			if (!is_vertex_sized_normal && !src_mesh.normal_list().empty())
			{
				for (int k = 0; k < 3; ++k)
				{
					sorted_normal[i * 3 + k] = src_mesh.normal_list().at(pair.second * 3 + k);
				}
			}
		}
		src_mesh.mutable_vertex_index_list().swap(sorted_vertex_index);
		src_mesh.mutable_material_index().swap(sorted_material_index);
		if (!src_mesh.uv_list().empty())
		{
			src_mesh.mutable_uv_list().swap(sorted_uv);
		}
		if (!is_vertex_sized_normal)
		{
			src_mesh.mutable_normal_list().swap(sorted_normal);
		}
	}

	/**
	 * convert vertices to triangle list
	 */
	void convert_to_triangle_list(umio::UMMesh& src_mesh)
	{
		const int vertex_index_size = static_cast<int>(src_mesh.vertex_index_list().size());
		umio::DoubleListVec triangle_list;
		triangle_list.resize(vertex_index_size * 3);
		
		umio::DoubleListVec normal_list;
		normal_list.resize(vertex_index_size * 3);
		
		for (int i = 0; i < vertex_index_size; ++i)
		{
			const umio::IntList& vertex_index = src_mesh.vertex_index_list().at(i);
			for (int k = 0; k < 3; ++k)
			{
				triangle_list[i * 3 + k] = src_mesh.vertex_list().at(vertex_index[k]);
				normal_list[i * 3 + k] = src_mesh.normal_list().at(vertex_index[k]);
			}
		}
		src_mesh.mutable_vertex_list().swap(triangle_list);
		src_mesh.mutable_normal_list().swap(normal_list);
	}

} // anonymouse namespace


namespace umdraw
{

/** 
 * import umdraw mesh list
 */
bool UMSoftwareIO::import_mesh_list(UMMeshList& dst, const umio::UMObjectPtr src, const std::u16string& absolute_file_path)
{
	if (!src) return false;

	bool result = false;
	umio::UMMesh::IDToMeshMap::iterator it = src->mutable_mesh_map().begin();
	for (; it != src->mutable_mesh_map().end(); ++it)
	{
		umio::UMMesh& ummesh = (*it).second;

		UMMeshPtr mesh(std::make_shared<UMMesh>());
		dst.push_back(mesh);
		result = true;
		
		sort_by_material(ummesh);
		load_material(absolute_file_path, mesh, ummesh);
		load_vertex_index(mesh, ummesh);
		load_vertex(mesh, ummesh);
		load_normal(mesh, ummesh);
		load_uv(mesh, ummesh);
		mesh->update_box();
	}
	return result;
}

} // umdraw
