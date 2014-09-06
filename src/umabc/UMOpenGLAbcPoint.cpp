/**
 * @file UMOpenGLAbcPoint.cpp
 * any
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_OPENGL

#include "UMOpenGLAbcPoint.h"

#include "UMAbcConvert.h"
#include "UMOpenGLPoint.h"
#include <GL/glew.h>

namespace umabc
{
	using namespace Alembic::Abc;
	using namespace Alembic::AbcGeom;
	
UMOpenGLAbcPoint::UMOpenGLAbcPoint(UMAbcPointPtr abc_point)
	: abc_point_(abc_point)
	, gl_point_(std::make_shared<umdraw::UMOpenGLPoint>())
	, vertex_size_(0)
	{}

/**
	* update vertex
	*/
void UMOpenGLAbcPoint::update_vertex(const Alembic::AbcGeom::P3fArraySamplePtr& vertex)
{
	if (!gl_point_) return;
	gl_point_->set_vertex_count(static_cast<unsigned int>(vertex->size()));

	if (!gl_point_->is_valid_vertex_vbo())
	{
		unsigned int vertex_vbo = 0;
		glGenBuffers(1, &vertex_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
		gl_point_->set_vertex_vbo(vertex_vbo);
	}

	glBindBuffer(GL_ARRAY_BUFFER, gl_point_->vertex_vbo());

	const V3f *points = vertex->get();
	glBufferData(GL_ARRAY_BUFFER,
		sizeof (V3f) * gl_point_->vertex_count(),
		reinterpret_cast<const GLvoid*>(points), 
		GL_DYNAMIC_DRAW );
}

/**
 * draw
 */
void UMOpenGLAbcPoint::draw() const
{
	if (!gl_point_) return;
	gl_point_->draw(umdraw::UMOpenGLDrawParameterPtr());
}

} // umabc

#endif // WITH_OPENGL
