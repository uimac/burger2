/**
 * @file UMOpenGLAbcMesh.cpp
 * any
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_OPENGL

#include "UMOpenGLAbcMesh.h"

#include "UMAbcMesh.h"
#include "UMAbcConvert.h"
#include "UMOpenGLMesh.h"
#include <GL/glew.h>

namespace umabc
{
	
	using namespace Alembic::Abc;
	using namespace Alembic::AbcGeom;

/**
 * constructor
 */
UMOpenGLAbcMesh::UMOpenGLAbcMesh(UMAbcMeshPtr abc_mesh)
	: abc_mesh_(abc_mesh)
	, gl_mesh_(std::make_shared<umdraw::UMOpenGLMesh>())
	, vertex_size_(0)
	, triagnle_size_(0)
{}

/**
 * destructor
 */
UMOpenGLAbcMesh::~UMOpenGLAbcMesh()
{
}

/** 
 * update normal
 */
void UMOpenGLAbcMesh::update_normal(
	const std::vector<UMVec3ui >& triangle_index, 
	const std::vector<Imath::V3f>& normal, 
	VaryingType type,
	bool is_cw)
{
	if (!gl_mesh_) return;
	if (!gl_mesh_->is_valid_normal_vbo())
	{
		unsigned int normal_vbo = 0;
		glGenBuffers(1, &normal_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, normal_vbo);
		gl_mesh_->set_normal_vbo(normal_vbo);
	}

	glBindBuffer(GL_ARRAY_BUFFER, gl_mesh_->normal_vbo());
	
	const size_t index_size = static_cast<int>(triangle_index.size());

	// vertex varying
	if (type == eVertexVarying)
	{
		const size_t normal_size = normal.size();
		if (triangle_index.empty())
		{
			// pure vertex varying
			const size_t face_size = normal_size / 3;
			if (is_cw)
			{
				for (size_t i = 0; i < face_size; ++i)
				{
					vertex_varying_temporary_v3f_[i * 3 + 0] = normal[i * 3 + 0];
					vertex_varying_temporary_v3f_[i * 3 + 1] = normal[i * 3 + 2];
					vertex_varying_temporary_v3f_[i * 3 + 2] = normal[i * 3 + 1];
				}
			}
			else
			{
				for (size_t i = 0; i < face_size; ++i)
				{
					vertex_varying_temporary_v3f_[i * 3 + 0] = normal[i * 3 + 0];
					vertex_varying_temporary_v3f_[i * 3 + 1] = normal[i * 3 + 1];
					vertex_varying_temporary_v3f_[i * 3 + 2] = normal[i * 3 + 2];
				}
			}
			glBufferData(GL_ARRAY_BUFFER,
				sizeof (V2f) * normal_size,
				reinterpret_cast<const GLvoid*>( &(*vertex_varying_temporary_v3f_.begin()) ), 
				GL_DYNAMIC_DRAW );
			return;
		}
		else if (index_size * 3 == normal.size())
		{
			// vertex varying but "vertex" is using vertexindex
			vertex_varying_temporary_v3f_.resize(normal_size);
			if (is_cw)
			{
				for (size_t i = 0; i < index_size; ++i)
				{
					vertex_varying_temporary_v3f_[i * 3 + 0] = normal[i * 3 + 0];
					vertex_varying_temporary_v3f_[i * 3 + 1] = normal[i * 3 + 2];
					vertex_varying_temporary_v3f_[i * 3 + 2] = normal[i * 3 + 1];
				}
			}
			else
			{
				for (size_t i = 0; i < index_size; ++i)
				{
					vertex_varying_temporary_v3f_[i * 3 + 0] = normal[i * 3 + 0];
					vertex_varying_temporary_v3f_[i * 3 + 1] = normal[i * 3 + 1];
					vertex_varying_temporary_v3f_[i * 3 + 2] = normal[i * 3 + 2];
				}
			}
			glBufferData(GL_ARRAY_BUFFER,
				sizeof (V3f) * normal_size,
				reinterpret_cast<const GLvoid*>( &(*vertex_varying_temporary_v3f_.begin()) ), 
				GL_DYNAMIC_DRAW );
			return;
		}
		// else try face varying
	}

	// face varying
	{
		const size_t normal_size = index_size * 3;
		vertex_varying_temporary_v3f_.resize(normal_size);
		for (size_t i = 0; i < index_size; ++i)
		{
			const int index0 = triangle_index.at(i)[0];
			const int index1 = triangle_index.at(i)[1];
			const int index2 = triangle_index.at(i)[2];
			vertex_varying_temporary_v3f_[i * 3 + 0] = normal[index0];
			vertex_varying_temporary_v3f_[i * 3 + 1] = normal[index2];
			vertex_varying_temporary_v3f_[i * 3 + 2] = normal[index1];
		}
		glBufferData(GL_ARRAY_BUFFER,
			sizeof (V3f) * normal_size,
			reinterpret_cast<const GLvoid*>( &(*vertex_varying_temporary_v3f_.begin()) ), 
			GL_DYNAMIC_DRAW );
	}
}

/** 
 * update uv
 */
void UMOpenGLAbcMesh::update_uv(
	const std::vector<UMVec3ui >& triangle_index, 
	const Alembic::AbcGeom::IV2fGeomParam::Sample& uv, 
	VaryingType type,
	bool is_cw)
{
	if (!gl_mesh_) return;
	if (!gl_mesh_->is_valid_uv_vbo())
	{
		unsigned int uv_vbo = 0;
		glGenBuffers(1, &uv_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, uv_vbo);
		gl_mesh_->set_uv_vbo(uv_vbo);
	}

	glBindBuffer(GL_ARRAY_BUFFER, gl_mesh_->uv_vbo());
	
	const V2f *uvs =  uv.getVals()->get();
	const size_t index_size = static_cast<int>(triangle_index.size());
	
	// vertex varying
	if (type == eVertexVarying)
	{
		const size_t uv_size = uv.getVals()->size();
		if (triangle_index.empty())
		{
			// pure vertex varying
			const size_t face_size = uv_size / 3;
			if (is_cw)
			{
				for (size_t i = 0; i < face_size; ++i)
				{
					vertex_varying_temporary_v2f_[i * 3 + 0] = uvs[i * 3 + 0];
					vertex_varying_temporary_v2f_[i * 3 + 1] = uvs[i * 3 + 2];
					vertex_varying_temporary_v2f_[i * 3 + 2] = uvs[i * 3 + 1];
				}
			}
			else
			{
				for (size_t i = 0; i < face_size; ++i)
				{
					vertex_varying_temporary_v2f_[i * 3 + 0] = uvs[i * 3 + 0];
					vertex_varying_temporary_v2f_[i * 3 + 1] = uvs[i * 3 + 1];
					vertex_varying_temporary_v2f_[i * 3 + 2] = uvs[i * 3 + 2];
				}
			}
			glBufferData(GL_ARRAY_BUFFER,
				sizeof (V2f) * uv_size,
				reinterpret_cast<const GLvoid*>( &(*vertex_varying_temporary_v2f_.begin()) ), 
				GL_DYNAMIC_DRAW );
			return;
		}
		else if (index_size * 3 == uv_size)
		{
			// vertex varying but "vertex" is using vertexindex
			vertex_varying_temporary_v2f_.resize(uv_size);
			if (is_cw)
			{
				for (size_t i = 0; i < index_size; ++i)
				{
					vertex_varying_temporary_v2f_[i * 3 + 0] = uvs[i * 3 + 0];
					vertex_varying_temporary_v2f_[i * 3 + 1] = uvs[i * 3 + 2];
					vertex_varying_temporary_v2f_[i * 3 + 2] = uvs[i * 3 + 1];
				}
			}
			else
			{
				for (size_t i = 0; i < index_size; ++i)
				{
					vertex_varying_temporary_v2f_[i * 3 + 0] = uvs[i * 3 + 0];
					vertex_varying_temporary_v2f_[i * 3 + 1] = uvs[i * 3 + 1];
					vertex_varying_temporary_v2f_[i * 3 + 2] = uvs[i * 3 + 2];
				}
			}
			glBufferData(GL_ARRAY_BUFFER,
				sizeof (V2f) * uv_size,
				reinterpret_cast<const GLvoid*>( &(*vertex_varying_temporary_v2f_.begin()) ), 
				GL_DYNAMIC_DRAW );
			return;
		}
		// else try face varying
	}
	
	// face varying
	{
		const size_t uv_size = index_size * 3;
		vertex_varying_temporary_v2f_.resize(uv_size);
		for (size_t i = 0; i < index_size; ++i)
		{
			const UMVec3ui& in = triangle_index.at(i);
			vertex_varying_temporary_v2f_[i * 3 + 0] = uvs[in.x];
			vertex_varying_temporary_v2f_[i * 3 + 1] = uvs[in.y];
			vertex_varying_temporary_v2f_[i * 3 + 2] = uvs[in.z];
		}
		glBufferData(GL_ARRAY_BUFFER,
			sizeof (V2f) * uv_size,
			reinterpret_cast<const GLvoid*>( &(*vertex_varying_temporary_v2f_.begin()) ), 
			GL_DYNAMIC_DRAW );
	}
}

/**
 * update vertex
 */
void UMOpenGLAbcMesh::update_vertex(
	const std::vector<UMVec3ui >& triangle_index, 
	const P3fArraySamplePtr& vertex,
	bool is_cw)
{
	if (!gl_mesh_) return;
	vertex_size_ = static_cast<unsigned int>(vertex->size());

	if (!gl_mesh_->is_valid_vertex_vbo())
	{
		unsigned int vertex_vbo = 0;
		glGenBuffers(1, &vertex_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
		gl_mesh_->set_vertex_vbo(vertex_vbo);
	}

	glBindBuffer(GL_ARRAY_BUFFER, gl_mesh_->vertex_vbo());
	
	const V3f *points = vertex->get();
	if (triangle_index.empty())
	{
		const size_t face_size = vertex_size_ / 3;
		for (size_t i = 0; i < face_size; ++i)
		{
			if (is_cw)
			{
				vertex_varying_temporary_v3f_[i * 3 + 0] = points[i * 3 + 0];
				vertex_varying_temporary_v3f_[i * 3 + 1] = points[i * 3 + 2];
				vertex_varying_temporary_v3f_[i * 3 + 2] = points[i * 3 + 1];
			}
			else
			{
				vertex_varying_temporary_v3f_[i * 3 + 0] = points[i * 3 + 0];
				vertex_varying_temporary_v3f_[i * 3 + 1] = points[i * 3 + 1];
				vertex_varying_temporary_v3f_[i * 3 + 2] = points[i * 3 + 2];
			}
		}
		glBufferData(GL_ARRAY_BUFFER,
			sizeof (V3f) * vertex_size_,
			reinterpret_cast<const GLvoid*>( &(*vertex_varying_temporary_v3f_.begin()) ),
			GL_DYNAMIC_DRAW );
	}
	else
	{
		const size_t index_size = static_cast<int>(triangle_index.size());
		vertex_size_ = static_cast<unsigned int>(index_size * 3);
		vertex_varying_temporary_v3f_.resize(vertex_size_);
		for (size_t i = 0; i < index_size; ++i)
		{
			const UMVec3ui& in = triangle_index.at(i);
			vertex_varying_temporary_v3f_[i * 3 + 0] = points[in.x];
			vertex_varying_temporary_v3f_[i * 3 + 1] = points[in.z];
			vertex_varying_temporary_v3f_[i * 3 + 2] = points[in.y];
		}
		glBufferData(GL_ARRAY_BUFFER,
			sizeof (V3f) * vertex_size_,
			reinterpret_cast<const GLvoid*>( &(*vertex_varying_temporary_v3f_.begin()) ), 
			GL_DYNAMIC_DRAW );
	}
}

///**
// * update vertex index
// */
//void UMOpenGLAbcMesh::update_vertex_index(const std::vector<UMVec3ui >& triangle_index)
//{
//	if (!gl_mesh_) return;
//	triagnle_size_ = static_cast<unsigned int>(triangle_index.size());
//
//	if (!gl_mesh_->is_valid_vertex_index_vbo())
//	{
//		unsigned int vertex_index_vbo = 0;
//		glGenBuffers(1, &vertex_index_vbo);
//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_index_vbo);
//		gl_mesh_->set_vertex_index_vbo(vertex_index_vbo);
//	}
//
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_mesh_->vertex_index_vbo());
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
//		sizeof (V3i) * triagnle_size_,
//		NULL, 
//		GL_DYNAMIC_DRAW );
//
//	V3i* buffer = static_cast<V3i*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));
//	if (buffer)
//	{
//		const std::vector<UMVec3ui >& face_list = triangle_index;
//		for (size_t i = 0, i_size = face_list.size(); i < i_size; ++i) {
//			// Alembic is CW
//			const UMVec3ui& in = triangle_index.at(i);
//			buffer[i].x = in.x;
//			buffer[i].y = in.z;
//			buffer[i].z = in.y;
//		}
//	}
//	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
//}

/**
 * draw
 */
void UMOpenGLAbcMesh::draw(umdraw::UMOpenGLDrawParameterPtr parameter) const
{
	if (!gl_mesh_) return;

	gl_mesh_->draw(parameter);
}

} // umabc

#endif // WITH_OPENGL
