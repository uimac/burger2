/**
 * @file UMOpenGLIO.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_OPENGL

#include <string>
#include <assert.h>
#include <GL/glew.h>

#include "UMIO.h"
#include "UMOpenGLIO.h"
#include "UMStringUtil.h"
#include "UMPath.h"
#include "UMLine.h"

#include "UMSoftwareIO.h"
#include "UMOpenGLMesh.h"
#include "UMOpenGLMeshGroup.h"
#include "UMOpenGLMaterial.h"
#include "UMOpenGLTexture.h"
#include "UMOpenGLLine.h"
#include "UMOpenGLDrawParameter.h"

#include <poly2tri.h>

namespace
{
	using namespace umdraw;

	double import_scale_for_debug = 1.0;

	UMVec2f to_gl(const umio::UMVec2d& v) { return UMVec2f((float)v.x, (float)v.y); }

	UMVec3f to_gl(const umio::UMVec3d& v) { return UMVec3f((float)v.x, (float)v.y, (float)v.z); }

	UMVec4f to_gl(const umio::UMVec4d& v) { return UMVec4f((float)v.x, (float)v.y, (float)v.z, (float)v.w); } 

	//----------------------------------------------------------------------------

	/** 
	 * triangulate index and store as faces
	 */
	bool triangulate_vertex_index(
		UMOpenGLMeshPtr dst_mesh, 
		UMMeshPtr src_mesh)
	{
		//const UMMesh::Vec3iList& face_list = src_mesh->face_list();
		//const size_t vertex_index_size = face_list.size();

		//unsigned int vertex_index_vbo = dst_mesh->vertex_index_vbo();
		//if (!dst_mesh->is_valid_vertex_index_vbo())
		//{
		//	glGenBuffers(1, &vertex_index_vbo);
		//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_index_vbo);
		//}

		//if (vertex_index_vbo == 0) return false;

		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_index_vbo);
		//
		////UMMesh::Vec3iList faces;
		////faces.resize(vertex_index_size);
		////for (size_t i = 0; i < vertex_index_size; ++i) {
		////	const UMVec3i& in = face_list.at(i);
		////	faces[i].x = in.x;
		////	faces[i].y = in.y;
		////	faces[i].z = in.z;
		////}

		//glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		//	sizeof (UMVec3i) * vertex_index_size,
		//	reinterpret_cast<const GLvoid*>( &(*face_list.begin()) ), 
		//	GL_STATIC_DRAW );
		//
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		//dst_mesh->set_vertex_index_vbo(vertex_index_vbo);
		return true;
	}
	
	/**
	 * load vertex from umdraw to gl
	 */
	bool load_vertex(
		UMOpenGLMeshPtr dst_mesh, 
		UMMeshPtr src_mesh)
	{
		const UMMesh::Vec3iList& face_list = src_mesh->face_list();
		const UMMesh::Vec3dList& vertex_list = src_mesh->vertex_list();
		const size_t face_size = face_list.size();

		unsigned int vertex_vbo = dst_mesh->vertex_vbo();
		if (!dst_mesh->is_valid_vertex_vbo())
		{
			glGenBuffers(1, &vertex_vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
		}

		if (vertex_vbo == 0) return false;

		glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);

		std::vector<UMVec3f> verts;
		size_t vertex_size = vertex_list.size();

		if (face_size > 0)
		{
			vertex_size = face_size * 3;
			verts.resize(vertex_size);

			for (size_t i = 0; i < face_size; ++i) {
				const UMVec3i& face = face_list.at(i);
				for (int k = 0; k < 3; ++k)
				{
					const UMVec3d& vertex = vertex_list.at(face[k]);
					verts[i * 3 + k].x = static_cast<float>(vertex.x);
					verts[i * 3 + k].y = static_cast<float>(vertex.y);
					verts[i * 3 + k].z = static_cast<float>(vertex.z);
				}
			}
		}
		else
		{
			verts.resize(vertex_size);

			for (size_t i = 0; i < vertex_size; ++i) {
				const UMVec3d& vertex = vertex_list.at(i);
				verts[i].x = static_cast<float>(vertex.x);
				verts[i].y = static_cast<float>(vertex.y);
				verts[i].z = static_cast<float>(vertex.z);
			}
		}
		glBufferData(GL_ARRAY_BUFFER,
			sizeof (UMVec3f) * vertex_size,
			reinterpret_cast<const GLvoid*>( &(*verts.begin()) ), 
			GL_STATIC_DRAW );
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		dst_mesh->set_vertex_vbo(vertex_vbo);
		return true;
	}

	/**
	 * load normal from umdraw to gl
	 */
	bool load_normal(
		UMOpenGLMeshPtr dst_mesh, 
		UMMeshPtr src_mesh)
	{
		const UMMesh::Vec3iList& face_list = src_mesh->face_list();
		const UMMesh::Vec3dList& normal_list = src_mesh->normal_list();
		const size_t face_size = face_list.size();
		const size_t normal_size = face_size ? face_size * 3 : normal_list.size();
		const bool is_vertex_sized_normal = src_mesh->vertex_list().size() == src_mesh->normal_list().size();

		unsigned int normal_vbo = dst_mesh->normal_vbo();
		if (!dst_mesh->is_valid_normal_vbo())
		{
			glGenBuffers(1, &normal_vbo);
			glBindBuffer(GL_ARRAY_BUFFER, normal_vbo);
		}

		if (normal_vbo == 0) return false;

		glBindBuffer(GL_ARRAY_BUFFER, normal_vbo);

		std::vector<UMVec3f> normals;
		normals.resize(normal_size);
		if (is_vertex_sized_normal)
		{
			if (face_size > 0)
			{
				for (size_t i = 0; i < face_size; ++i) {
					const UMVec3i& face = face_list.at(i);
					for (int k = 0; k < 3; ++k)
					{
						const UMVec3d& normal = normal_list.at(face[k]);
						normals[i * 3 + k].x = static_cast<float>(normal.x);
						normals[i * 3 + k].y = static_cast<float>(normal.y);
						normals[i * 3 + k].z = static_cast<float>(normal.z);
					}
				}
			}
			else
			{
				for (size_t i = 0; i < normal_size; ++i) {
					const UMVec3d& normal = normal_list.at(i);
					normals[i].x = static_cast<float>(normal.x);
					normals[i].y = static_cast<float>(normal.y);
					normals[i].z = static_cast<float>(normal.z);
				}
			}
		}
		else if (normal_list.size() == src_mesh->face_list().size() * 3)
		{
			for (size_t i = 0; i < face_size; ++i) {
				for (int k = 0; k < 3; ++k)
				{
					const UMVec3d& normal = normal_list.at(i * 3 + k);
					normals[i * 3 + k].x = static_cast<float>(normal.x);
					normals[i * 3 + k].y = static_cast<float>(normal.y);
					normals[i * 3 + k].z = static_cast<float>(normal.z);
				}
			}
		}

		glBufferData(GL_ARRAY_BUFFER,
			sizeof (UMVec3f) * normal_size,
			reinterpret_cast<const GLvoid*>( &(*normals.begin()) ), 
			GL_STATIC_DRAW );
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		dst_mesh->set_normal_vbo(normal_vbo);
		return true;
	}
	
	/** 
	 * load uv from umdraw to gl
	 */
	bool load_uv(
		UMOpenGLMeshPtr dst_mesh, 
		UMMeshPtr src_mesh)
	{
		const UMMesh::Vec2dList& uv_list = src_mesh->uv_list();
		const size_t uv_size = uv_list.size();

		unsigned int uv_vbo = dst_mesh->uv_vbo();
		if (!dst_mesh->is_valid_uv_vbo())
		{
			glGenBuffers(1, &uv_vbo);
			glBindBuffer(GL_ARRAY_BUFFER, uv_vbo);
		}
		if (uv_vbo == 0) return false;

		glBindBuffer(GL_ARRAY_BUFFER, uv_vbo);
		
		std::vector<UMVec2f> uvs;
		uvs.resize(uv_size);
		for (size_t i = 0; i < uv_size; ++i)
		{
			const UMVec2d& uv = uv_list.at(i);
			uvs[i].x = static_cast<float>(uv.x);
			uvs[i].y = static_cast<float>(uv.y);
		}

		glBufferData(GL_ARRAY_BUFFER,
			sizeof (UMVec2f) * uv_size,
			reinterpret_cast<const GLvoid*>( &(*uvs.begin()) ), 
			GL_STATIC_DRAW );
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		dst_mesh->set_uv_vbo(uv_vbo);
		return true;
	}
	
	/** 
	 * load line from umdraw to gl
	 */
	bool load_line(
		UMOpenGLLinePtr dst_line,
		UMLinePtr src_line)
	{
		const UMLine::LineList& line_list = src_line->line_list();
		const size_t line_size = line_list.size() * 2;

		unsigned int vertex_vbo = dst_line->vertex_vbo();
		if (!dst_line->is_valid_vertex_vbo())
		{
			glGenBuffers(1, &vertex_vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
		}
		if (vertex_vbo == 0) return false;
		
		glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
		
		std::vector<UMVec3f> lines;
		lines.resize(line_size);
		UMLine::LineList::const_iterator it = line_list.begin();
		for (size_t i = 0, size = line_list.size(); i < size; ++i)
		{
			const UMLine::Line& umline = line_list.at(i);
			lines[i * 2 + 0].x = static_cast<float>(umline.p0.x);
			lines[i * 2 + 0].y = static_cast<float>(umline.p0.y);
			lines[i * 2 + 0].z = static_cast<float>(umline.p0.z);
			lines[i * 2 + 1].x = static_cast<float>(umline.p1.x);
			lines[i * 2 + 1].y = static_cast<float>(umline.p1.y);
			lines[i * 2 + 1].z = static_cast<float>(umline.p1.z);
		}
		
		glBufferData(GL_ARRAY_BUFFER,
			sizeof (UMVec3f) * line_size,
			reinterpret_cast<const GLvoid*>( &(*lines.begin()) ), 
			GL_STATIC_DRAW );
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		dst_line->set_vertex_vbo(vertex_vbo);
		dst_line->set_vertex_count(static_cast<unsigned int>(line_size));
		return true;
	}

} // anonymouse namespace


namespace umdraw
{

/** 
 * import umdraw mesh list
 */
bool UMOpenGLIO::import_mesh_list(UMMeshList& dst, const umio::UMObjectPtr src, const std::u16string& absolute_file_path)
{
	return UMSoftwareIO::import_mesh_list(dst, src, absolute_file_path);
}

/**
 * convert umdraw mesh to OpenGL mesh
 */
UMOpenGLMeshPtr UMOpenGLIO::convert_mesh_to_gl_mesh(
	UMMeshPtr src)
{
	if (!src) return UMOpenGLMeshPtr();

	UMOpenGLMeshPtr mesh(std::make_shared<UMOpenGLMesh>());
	
	// triangulate index
	if (src->vertex_index_list().size() > 0) {
		if (!triangulate_vertex_index(mesh, src)) {
			return UMOpenGLMeshPtr();
		}
	}

	// create vertex buffer
	if (src->vertex_list().size() > 0) {
		// create full triangle verts for uv
		if (!load_vertex(mesh, src)) {
			return UMOpenGLMeshPtr();
		}
	}

	// create normal buffer
	if (src->normal_list().size() > 0) {
		// create full triangle normals for uv
		if (!load_normal(mesh, src)) {
			return UMOpenGLMeshPtr();
		}
	}
		
	// create uv buffer
	if (src->uv_list().size() > 0) {
		if (!load_uv(mesh, src)) {
			return UMOpenGLMeshPtr();
		}
	}

	// convert material
	if (src->material_list().size() > 0) {
		const int material_size = static_cast<int>(src->material_list().size());
		for (int i = 0; i < material_size; ++i)
		{
			UMMaterialPtr material = src->material_list().at(i);
			UMOpenGLMaterialPtr gl_material = UMOpenGLIO::convert_material_to_gl_material(material);
			if (gl_material)
			{
				mesh->mutable_material_list().push_back(gl_material);
			}
		}
	}

	if (src->is_valid_shader_entry()) {
		UMOpenGLDrawParameterPtr parameter(std::make_shared<UMOpenGLDrawParameter>());
		UMOpenGLShaderManagerPtr manager(std::make_shared<UMOpenGLShaderManager>());
		manager->init(UMOpenGLShaderManager::eOriginal);
		UMOpenGLShaderPtr shader(std::make_shared<UMOpenGLShader>());
		shader->create_shader_from_memory(
			src->shader_entry()->gl_vertex_shader(),
			src->shader_entry()->gl_fragment_shader());
		manager->mutable_shader_list().push_back(shader);
		parameter->set_resolution(UMVec2d(src->box().size().x, src->box().size().y));
		parameter->set_shader_manager(manager);
		mesh->set_draw_parameter(parameter);
	}

	//mesh->set_ummesh(src);
	return mesh;
}

/**
 * convert umdraw mesh group to OpenGL mesh
 */
UMOpenGLMeshGroupPtr UMOpenGLIO::convert_mesh_group_to_gl_mesh_group(
	UMMeshGroupPtr src)
{
	if (!src) return UMOpenGLMeshGroupPtr();
	
	UMOpenGLMeshGroupPtr group(std::make_shared<UMOpenGLMeshGroup>());
	group->init();
	group->set_ummesh_group(src);
	UMMeshList::const_iterator it = src->mesh_list().begin();
	for (; it != src->mesh_list().end(); ++it)
	{
		UMMeshPtr ummesh = *it;
		UMOpenGLMeshPtr mesh = UMOpenGLIO::convert_mesh_to_gl_mesh(ummesh);
		if (!mesh) return  UMOpenGLMeshGroupPtr();

		//mesh->set_ummesh(ummesh);
		group->mutable_gl_mesh_list().push_back(mesh);
	}
	return group;
}


/**
 * convert umdraw line to OpenGL line
 * @param [in] src source umdraw line
 * @retval UMOpenGLLinePtr converted gl line or none
 */
UMOpenGLLinePtr UMOpenGLIO::convert_line_to_gl_line(
	UMLinePtr src)
{
	if (!src) return UMOpenGLLinePtr();

	UMOpenGLLinePtr line(std::make_shared<UMOpenGLLine>());
	if (load_line(line, src))
	{
		// convert material
		if (src->material_list().size() > 0) {
			const int material_size = static_cast<int>(src->material_list().size());
			for (int i = 0; i < material_size; ++i)
			{
				UMMaterialPtr material = src->material_list().at(i);
				UMOpenGLMaterialPtr gl_material = UMOpenGLIO::convert_material_to_gl_material(material);
				gl_material->set_shader_flags(UMVec4f(0, 1, 0, 0));
				if (gl_material)
				{
					line->mutable_material_list().push_back(gl_material);
				}
			}
		}
		return line;
	}
	return UMOpenGLLinePtr();
}

/**
 * convert umdraw light to OpenGL light
 */
UMOpenGLLightPtr UMOpenGLIO::convert_light_to_gl_light(
	UMLightPtr src)
{
	if (!src) return UMOpenGLLightPtr();

	UMOpenGLLightPtr light(std::make_shared<UMOpenGLLight>(src));
	light->init();
	return light;
}

/**
 * convert umdraw material to OpenGL material
 */
UMOpenGLMaterialPtr UMOpenGLIO::convert_material_to_gl_material(
	UMMaterialPtr src)
{
	if (!src) return UMOpenGLMaterialPtr();

	UMOpenGLMaterialPtr material(std::make_shared<UMOpenGLMaterial>(src));
	material->init();
	return material;
}

/**
 * convert umdraw camera to OpenGL camera
 */
UMOpenGLCameraPtr UMOpenGLIO::convert_camera_to_gl_camera(
	UMCameraPtr src)
{
	if (!src) return UMOpenGLCameraPtr();

	UMOpenGLCameraPtr camera(std::make_shared<UMOpenGLCamera>(src));
	camera->init();
	return camera;
}

} // umdraw

#endif // WITH_OPENGL
