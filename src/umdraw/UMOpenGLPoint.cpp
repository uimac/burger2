/**
 * @file UMOpenGLPoint.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_OPENGL

#include "UMOpenGLPoint.h"
#include "UMOpenGLCamera.h"
#include "UMOpenGLDrawParameter.h"
#include "UMOpenGLShaderManager.h"

#include <iterator>
#include <GL/glew.h>

#include "UMMacro.h"
#include "UMPath.h"
#include "UMStringUtil.h"

namespace umdraw
{

class UMOpenGLPoint::Impl
{
	DISALLOW_COPY_AND_ASSIGN(Impl);
public:

	Impl()
		: is_valid_vertex_vbo_(false)
		, vertex_vbo_(0) 
		, vertex_count_(0)
	{}

	~Impl() 
	{
		if (!is_valid_vertex_vbo_) {
			glDeleteBuffers(1, &vertex_vbo_);
		}
	}

	void draw(UMOpenGLDrawParameterPtr parameter)
	{
		if (!is_valid_vertex_vbo_) return;
		if (vertex_count_ == 0) return;
	
		UMOpenGLShaderManagerPtr shader_manager;
		UMOpenGLCameraPtr camera;
		UMOpenGLLightPtr light;
		if (parameter)
		{
			light = parameter->light();
			camera = parameter->camera();
			shader_manager = parameter->shader_manager();
		}
		if (draw_parameter_)
		{
			light = draw_parameter_->light();
			camera = draw_parameter_->camera();
			shader_manager = draw_parameter_->shader_manager();
		}
	
		// shader list
		const UMOpenGLShaderManager::ShaderList& shaders = shader_manager->shader_list();
		if (shaders.empty()) return;
		UMOpenGLShaderPtr shader = shaders[1];
	
		if (view_projection_location_ < 0)
		{
			view_projection_location_ = glGetUniformLocation(shader->program_object(), "view_projection_matrix");
		}

		glUseProgram(shader->program_object());

		glPointSize(3.0);

		// put camera to glsl
		// don't use ubo because intel/amd issue
		if (camera)
		{
			UMMat44f view_projection = camera->view_projection_matrix();
			glUniformMatrix4fv(view_projection_location_, 1, GL_FALSE, view_projection.m[0]);
		}

		if (is_valid_vertex_vbo())
		{
			unsigned int offset = 0;//sizeof(UMVec3f) * index_offset;

			glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo());

			GLuint position_attr = glGetAttribLocation(shader->program_object(), "a_position" );
			glEnableVertexAttribArray(position_attr);
			glVertexAttribPointer(position_attr, 3, GL_FLOAT, GL_FALSE, 0, (const void*)offset);
		}

		glDrawArrays(GL_POINTS, 0, vertex_count_);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glUseProgram(0);
	}
	
	bool is_valid_vertex_vbo() const 
	{
		return is_valid_vertex_vbo_;
	}

	unsigned int vertex_vbo()
	{
		return vertex_vbo_;
	}
	
	void set_vertex_vbo(unsigned int vbo)
	{
		vertex_vbo_ = vbo; is_valid_vertex_vbo_ = true;
	}
	
	void set_vertex_count(unsigned int size)
	{
		vertex_count_ = size;
	}

	unsigned int vertex_count() const
	{
		return vertex_count_;
	}
	
	void set_draw_parameter(umdraw::UMOpenGLDrawParameterPtr parameter)
	{
		draw_parameter_ = parameter;
	}
private:
	bool is_valid_vertex_vbo_;
	unsigned int vertex_vbo_;
	unsigned int vertex_count_;
	int view_projection_location_;
	UMOpenGLDrawParameterPtr draw_parameter_;
};

/// constructor
UMOpenGLPoint::UMOpenGLPoint() 
	: impl_(new UMOpenGLPoint::Impl)
{}

/// destructor
UMOpenGLPoint::~UMOpenGLPoint()
{
}

/**
 * draw
 */
void UMOpenGLPoint::draw(UMOpenGLDrawParameterPtr parameter)
{
	impl_->draw(parameter);
}

/**
 * get index buffer id id
 */
bool UMOpenGLPoint::is_valid_vertex_vbo() const 
{
	return impl_->is_valid_vertex_vbo();
}

/**
 * get vertex buffer id
 */
unsigned int UMOpenGLPoint::vertex_vbo()
{
	return impl_->vertex_vbo();
}
	
/**
 * set vertex buffer id
 */
void UMOpenGLPoint::set_vertex_vbo(unsigned int vbo)
{
	return impl_->set_vertex_vbo(vbo);
}
	
/**
 * set vertex count
 */
void UMOpenGLPoint::set_vertex_count(unsigned int size)
{
	return impl_->set_vertex_count(size);
}

/**
 * get veretx count
 */
unsigned int UMOpenGLPoint::vertex_count() const
{
	return impl_->vertex_count();
}
	
/**
 * set draw parameter
 */
void UMOpenGLPoint::set_draw_parameter(umdraw::UMOpenGLDrawParameterPtr parameter)
{
	return impl_->set_draw_parameter(parameter);
}

} // umdraw

#endif // WITH_OPENGL
