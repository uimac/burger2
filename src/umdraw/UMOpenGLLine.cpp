/**
 * @file UMOpenGLLine.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_OPENGL

#include "UMOpenGLLine.h"
#include "UMOpenGLCamera.h"
#include "UMOpenGLDrawParameter.h"
#include "UMOpenGLShaderManager.h"
#include "UMOpenGLMaterial.h"

#include <iterator>
#include <GL/glew.h>

#include "UMMacro.h"
#include "UMPath.h"
#include "UMStringUtil.h"

namespace umdraw
{

/// constructor
UMOpenGLLine::UMOpenGLLine() 
	: is_valid_vertex_vbo_(false)
	, vertex_vbo_(0) 
	, vertex_count_(0)
	, shader_manager_(std::make_shared<UMOpenGLShaderManager>())
	, view_projection_location_(-1)
	, mat_diffuse_location_(-1)
	, mat_flags_location_(-1)
{}

/// destructor
UMOpenGLLine::~UMOpenGLLine()
{
	if (!is_valid_vertex_vbo_) {
		glDeleteBuffers(1, &vertex_vbo_);
	}
}

/** 
 * initialize
 */
bool UMOpenGLLine::init()
{
	// init shader manager
	if (!shader_manager_->init(UMOpenGLShaderManager::eModel))
	{
		return false;
	}
	return true;
}

/** 
 * init vbo
 */
bool UMOpenGLLine::init_vbo(UMOpenGLShaderPtr shader)
{
	if (is_valid_vertex_vbo())
	{
		unsigned int offset = 0;//sizeof(UMVec3f) * index_offset;

		glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo());

		GLuint position_attr = glGetAttribLocation(shader->program_object(), "a_position" );
		glEnableVertexAttribArray(position_attr);
		glVertexAttribPointer(position_attr, 3, GL_FLOAT, GL_FALSE, 0, (const void*)offset);
	}

	GLuint normal_attr = glGetAttribLocation(shader->program_object(), "a_normal" );
	glDisableVertexAttribArray(normal_attr);
	GLuint uv_attr = glGetAttribLocation(shader->program_object(), "a_uv" );
	glDisableVertexAttribArray(uv_attr);
	return true;
}

/** 
 * initialize vao
 */
bool UMOpenGLLine::init_vao(UMOpenGLShaderPtr shader)
{
	int count = 0;
	UMOpenGLMaterialList::const_iterator mt = material_list().begin();
	for (; mt != material_list().end(); ++mt, ++count)
	{
		UMOpenGLMaterialPtr material = *mt;
		if (!material) continue;
		
#if !defined(WITH_EMSCRIPTEN)
		unsigned int vao = mutable_vao_map()[material];
		if (vao > 0) continue;
		
		// create vao
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		init_vbo(shader);
		glBindVertexArray(0);
		mutable_vao_map()[material] = vao;
#endif
	}
#if !defined(WITH_EMSCRIPTEN)
	glBindBuffer(GL_ARRAY_BUFFER, 0);
#else
	init_vbo(shader);
#endif
	return true;
}

/**
 * draw
 */
void UMOpenGLLine::draw(UMOpenGLDrawParameterPtr parameter)
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
	UMOpenGLShaderPtr shader = shaders[0];
	
	// get locations
	if (view_projection_location_ == -1)
	{
		view_projection_location_ = glGetUniformLocation(shader->program_object(), "view_projection_matrix");
		if (view_projection_location_ < 0) { view_projection_location_ = -2; }
	}
	if (mat_diffuse_location_ == -1)
	{
		mat_diffuse_location_ = glGetUniformLocation(shader->program_object(), "constant_color");
		if (mat_diffuse_location_ < 0) { mat_diffuse_location_ = -2; }
	}
	if (mat_flags_location_ == -1)
	{
		mat_flags_location_ = glGetUniformLocation(shader->program_object(), "mat_flags");
		if (mat_flags_location_ < 0) { mat_flags_location_ = -2; }
	}

	glUseProgram(shader->program_object());
	init_vao(shader);

	// put camera to glsl
	// don't use ubo because intel/amd issue
	if (camera)
	{
		UMMat44f view_projection = camera->view_projection_matrix();
		glUniformMatrix4fv(view_projection_location_, 1, GL_FALSE, view_projection.m[0]);
	}

	unsigned int index_offset = 0;
	
	UMOpenGLMaterialList::const_iterator mt = material_list().begin();
	for (; mt != material_list().end(); ++mt)
	{
		UMOpenGLMaterialPtr material = *mt;
		if (!material) continue;
		unsigned int index_count = material->polygon_count() * 2;
	
#if !defined(WITH_EMSCRIPTEN)
		unsigned int& vao = mutable_vao_map()[material];
		if (vao == 0) continue;
		glBindVertexArray(vao);
#endif
		// put material to glsl
		// don't use ubo because intel/amd issue
		{
			const UMVec4f& diffuse = material->diffuse();
			glUniform4f(mat_diffuse_location_, diffuse.x, diffuse.y, diffuse.z, diffuse.w);

			//GLuint mat_specular = glGetUniformLocation(shader->program_object(), "mat_specular");
			//const UMVec4f& specular = material->specular();
			//glUniform4f(mat_specular, specular.x, specular.y, specular.z, specular.w);

			//GLuint mat_ambient = glGetUniformLocation(shader->program_object(), "mat_ambient");
			//const UMVec4f& ambient = material->ambient();
			//glUniform4f(mat_ambient, ambient.x, ambient.y, ambient.z, ambient.w);

			const UMVec4f& shader_flags = material->shader_flags();
			glUniform4f(mat_flags_location_, shader_flags.x, shader_flags.y, shader_flags.z, shader_flags.w);

		}

		glDrawArrays(GL_LINES, index_offset, index_count);
		
		index_offset += index_count;
	}
	
#if !defined(WITH_EMSCRIPTEN)
	glBindVertexArray(0);
#endif
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}

} // umdraw

#endif // WITH_OPENGL
