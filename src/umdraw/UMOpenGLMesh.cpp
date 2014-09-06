/**
 * @file UMOpenGL.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_OPENGL

#include "UMOpenGLMesh.h"
#include "UMOpenGLShaderManager.h"
#include "UMOpenGLLight.h"
#include "UMOpenGLCamera.h"
#include "UMOpenGLTexture.h"
#include "UMOpenGLMaterial.h"
#include "UMOpenGLDrawParameter.h"
#include "UMCamera.h"

#include <GL/glew.h>

namespace umdraw
{

class UMOpenGLMesh::Impl
{
	DISALLOW_COPY_AND_ASSIGN(Impl);
public:

	Impl()
	: vertex_index_vbo_(0)
	, vertex_vbo_(0)
	, normal_vbo_(0)
	, uv_vbo_(0)
	, is_valid_vertex_vbo_(false)
	, is_valid_normal_vbo_(false)
	, is_valid_uv_vbo_(false)
	, is_valid_vertex_index_vbo_(false)
	, view_projection_location_(-1)
	, light_position_location_(-1)
	, light_color_location_(-1)
	, light_ambient_color_location_(-1)
	, mat_diffuse_location_(-1)
	, mat_specular_location_(-1)
	, mat_ambient_location_(-1)
	, mat_flags_location_(-1)
	, sampler_location_(-1)
	, resolution_location_(-1)
	, position_attr_(-1)
	, normal_attr_(-1)
	, uv_attr_(-1)
	, is_frag_color_binded_(false)
	{}

	~Impl() 
	{
		if (is_valid_vertex_vbo_) { 
			glDeleteBuffers(1, &vertex_vbo_);
		}
		if (is_valid_normal_vbo_) { 
			glDeleteBuffers(1, &normal_vbo_);
		}
		if (is_valid_uv_vbo_) { 
			glDeleteBuffers(1, &uv_vbo_);
		}
		if (is_valid_vertex_index_vbo_) { 
			glDeleteBuffers(1, &vertex_index_vbo_);
		}
	}

	bool init()
	{
		UMOpenGLMaterialList::const_iterator mt = material_list().begin();
		for (; mt != material_list().end(); ++mt)
		{
			UMOpenGLMaterialPtr material = *mt;
			if (!material) continue;
			material->init();
		}
		return true;
	}

	bool update();

	void draw(UMOpenGLDrawParameterPtr parameter);
	
	bool is_valid_vertex_vbo() const { return is_valid_vertex_vbo_; };
	
	bool is_valid_vertex_index_vbo() const { return is_valid_vertex_index_vbo_; };
	
	bool is_valid_normal_vbo() const { return is_valid_normal_vbo_; };
	
	bool is_valid_uv_vbo() const { return is_valid_uv_vbo_; };

	unsigned int  vertex_index_vbo() { return vertex_index_vbo_; };

	unsigned int vertex_vbo() { return vertex_vbo_; };

	unsigned int normal_vbo() { return normal_vbo_; };

	unsigned int uv_vbo() { return uv_vbo_; };
	
	void set_vertex_index_vbo(unsigned int vbo) { vertex_index_vbo_ = vbo;  is_valid_vertex_index_vbo_ = true; }

	void set_vertex_vbo(unsigned int vbo) { vertex_vbo_ = vbo; is_valid_vertex_vbo_ = true; }

	void set_normal_vbo(unsigned int vbo) { normal_vbo_ = vbo; is_valid_normal_vbo_ = true; }

	void set_uv_vbo(unsigned int vbo) { uv_vbo_ = vbo;  is_valid_uv_vbo_ = true; }

	const UMOpenGLMaterialList& material_list() const { return material_list_; }
	
	UMOpenGLMaterialList& mutable_material_list() { return material_list_; }
	
	void set_draw_parameter(umdraw::UMOpenGLDrawParameterPtr parameter) { draw_parameter_ = parameter; }

private:
	bool is_valid_vertex_vbo_;
	bool is_valid_vertex_index_vbo_;
	bool is_valid_normal_vbo_;
	bool is_valid_uv_vbo_;
	
	unsigned int vertex_index_vbo_;
	unsigned int vertex_vbo_;
	unsigned int normal_vbo_;
	unsigned int uv_vbo_;

	int position_attr_;
	int normal_attr_;
	int uv_attr_;
	int view_projection_location_;
	int light_position_location_;
	int light_color_location_;
	int light_ambient_color_location_;
	int mat_diffuse_location_;
	int mat_specular_location_;
	int mat_ambient_location_;
	int mat_flags_location_;
	int sampler_location_;
	
	int resolution_location_;
	bool is_frag_color_binded_;

	UMOpenGLMaterialList material_list_;
	UMOpenGLDrawParameterPtr draw_parameter_;
	
	typedef std::map<UMOpenGLMaterialPtr, unsigned int> VAOMap;
	VAOMap& mutable_vao_map() { return vao_map_; }
	VAOMap vao_map_;
	bool init_vao(UMOpenGLShaderPtr shader);
	bool init_vbo(UMOpenGLShaderPtr shader);
};

/** 
 * init vbo
 */
bool UMOpenGLMesh::Impl::init_vbo(UMOpenGLShaderPtr shader)
{
	// bind vbo to vao
	if (is_valid_vertex_vbo())
	{
		unsigned int offset = 0;
		glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo());
		GLuint position_attr = glGetAttribLocation(shader->program_object(), "a_position" );
		if (static_cast<int>(position_attr) >= 0)
		{
			glEnableVertexAttribArray(position_attr);
			glVertexAttribPointer(position_attr, 3, GL_FLOAT, GL_FALSE, 0, (const void*)offset);
		}
	}
	else
	{
		GLuint position_attr = glGetAttribLocation(shader->program_object(), "a_position" );
		glDisableVertexAttribArray(position_attr);
	}

	if (is_valid_normal_vbo())
	{
		unsigned int offset = 0;
		glBindBuffer(GL_ARRAY_BUFFER, normal_vbo());
		GLuint normal_attr = glGetAttribLocation(shader->program_object(), "a_normal" );
		if (static_cast<int>(normal_attr ) >= 0)
		{
			glEnableVertexAttribArray(normal_attr);
			glVertexAttribPointer(normal_attr, 3, GL_FLOAT, GL_FALSE, 0, (const void*)offset);
		}
	}
	else
	{
		GLuint normal_attr = glGetAttribLocation(shader->program_object(), "a_normal" );
		glDisableVertexAttribArray(normal_attr);
	}

	if (is_valid_uv_vbo())
	{
		unsigned int offset = 0;
		glBindBuffer(GL_ARRAY_BUFFER, uv_vbo());
		GLuint uv_attr = glGetAttribLocation(shader->program_object(), "a_uv" );
		if (static_cast<int>(uv_attr) >= 0)
		{
			glEnableVertexAttribArray(uv_attr);
			glVertexAttribPointer(uv_attr, 2, GL_FLOAT, GL_FALSE, 0, (const void*)offset);
		}
	}
	else
	{
		GLuint uv_attr = glGetAttribLocation(shader->program_object(), "a_uv" );
		glDisableVertexAttribArray(uv_attr);
	}
	return true;
}

/** 
 * init vao
 */
bool UMOpenGLMesh::Impl::init_vao(UMOpenGLShaderPtr shader)
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
void UMOpenGLMesh::Impl::draw(UMOpenGLDrawParameterPtr parameter)
{
	unsigned int index_offset = 0;

	UMOpenGLLightPtr light;
	UMOpenGLCameraPtr camera;
	UMOpenGLShaderManagerPtr shader_manager;
	if (parameter)
	{
		light = parameter->light();
		camera = parameter->camera();
		shader_manager = parameter->shader_manager();
	}
	if (draw_parameter_)
	{
		if (draw_parameter_->light()) { light = draw_parameter_->light(); }
		if (draw_parameter_->camera()) { camera = draw_parameter_->camera(); }
		if (draw_parameter_->shader_manager()) { shader_manager = draw_parameter_->shader_manager(); }
	}
	
	if (!shader_manager) return;
	// shader list
	const UMOpenGLShaderManager::ShaderList& shaders = shader_manager->shader_list();
	if (shaders.empty()) return;
	UMOpenGLShaderPtr shader = shaders[0];

	// get locations
	if (view_projection_location_  == -1)
	{
		view_projection_location_ = glGetUniformLocation(shader->program_object(), "view_projection_matrix");
		if (view_projection_location_ < 0) { view_projection_location_ = -2; }
	}
	if (light_position_location_ == -1)
	{
		light_position_location_  = glGetUniformLocation(shader->program_object(), "light_position");
		if (light_position_location_ < 0) { light_position_location_ = -2; }
	}
	if (light_color_location_ == -1)
	{
		light_color_location_ = glGetUniformLocation(shader->program_object(), "light_color");
		if (light_color_location_ < 0) { light_color_location_ = -2; }
	}
	if (light_ambient_color_location_ == -1)
	{
		light_ambient_color_location_  = glGetUniformLocation(shader->program_object(), "light_ambient_color");
		if (light_ambient_color_location_ < 0) { light_ambient_color_location_ = -2; }
	}
	if (mat_diffuse_location_ == -1)
	{
		mat_diffuse_location_  = glGetUniformLocation(shader->program_object(), "mat_diffuse");
		if (mat_diffuse_location_ < 0) { mat_diffuse_location_ = -2; }
	}
	if (mat_specular_location_ == -1)
	{
		mat_specular_location_  = glGetUniformLocation(shader->program_object(), "mat_specular");
		if (mat_specular_location_ < 0) { mat_specular_location_ = -2; }
	}
	if (mat_ambient_location_ == -1)
	{
		mat_ambient_location_  = glGetUniformLocation(shader->program_object(), "mat_ambient");
		if (mat_ambient_location_ < 0) { mat_ambient_location_ = -2; }
	}
	if (mat_flags_location_ == -1)
	{
		mat_flags_location_  = glGetUniformLocation(shader->program_object(), "mat_flags");
		if (mat_flags_location_ < 0) { mat_flags_location_ = -2; }
	}
	if (sampler_location_  == -1)
	{
		sampler_location_ = glGetUniformLocation(shader->program_object(), "s_texture");
		if (sampler_location_ < 0) { sampler_location_ = -2; }
	}
	if (resolution_location_ == -1)
	{
		resolution_location_ = glGetUniformLocation(shader->program_object(), "resolution");
		if (resolution_location_ < 0) { resolution_location_ = -2; }
	}
	//if (!is_frag_color_binded_)
	//{
	//	glBindFragDataLocation(shader->program_object(), 0, "frag_color");
	//	is_frag_color_binded_ = true;
	//}

	glUseProgram(shader->program_object());
	init_vao(shader);

	if (resolution_location_ >= 0)
	{
		UMVec2f resolution(
			static_cast<float>(draw_parameter_->resolution().x), 
			static_cast<float>(draw_parameter_->resolution().y));
		glUniform2f(resolution_location_, resolution.x, resolution.y);
	}

	// put camera to glsl
	// don't use ubo because intel/amd issue
	if (camera)
	{
		if (UMCameraPtr umcamera = camera->umcamera())
		{
			if (umcamera->is_ortho())
			{
				UMMat44f view_projection = camera->view_matrix();
				glUniformMatrix4fv(view_projection_location_, 1, GL_FALSE, view_projection.m[0]);
			}
			else
			{
				UMMat44f view_projection = camera->view_projection_matrix();
				glUniformMatrix4fv(view_projection_location_, 1, GL_FALSE, view_projection.m[0]);
			}
		}
	}

	// put light to glsl
	// don't use ubo because intel/amd issue
	if (light)
	{
		if (light_position_location_ >= 0)
		{
			const UMVec4f& pos = light->position();
			glUniform4f(light_position_location_, pos.x, pos.y, pos.z, 0.0f);
		}
		
		if (light_color_location_ >= 0)
		{
			const UMVec4f& color = light->color();
			glUniform4f(light_color_location_, color.x, color.y, color.z, 1.0f);
		}
		
		if (light_ambient_color_location_ >= 0)
		{
			const UMVec4f& ambient_color = light->ambient_color();
			glUniform4f(light_ambient_color_location_, ambient_color.x, ambient_color.y, ambient_color.z, 0.0f);
		}
	}

	UMOpenGLMaterialList::const_iterator mt = material_list().begin();
	for (; mt != material_list().end(); ++mt)
	{
		UMOpenGLMaterialPtr material = *mt;
		if (!material) continue;
		
#if !defined(WITH_EMSCRIPTEN)
		unsigned int& vao = mutable_vao_map()[material];
		if (vao == 0) continue;
#endif
	
		// put material to glsl
		// don't use ubo because intel/amd issue
		{
			if (mat_diffuse_location_ >= 0)
			{
				const UMVec4f& diffuse = material->diffuse();
				glUniform4f(mat_diffuse_location_, diffuse.x, diffuse.y, diffuse.z, diffuse.w);
			}
				
			if (mat_specular_location_ >= 0)
			{
				const UMVec4f& specular = material->specular();
				glUniform4f(mat_specular_location_, specular.x, specular.y, specular.z, specular.w);
			}
			
			if (mat_ambient_location_ >= 0)
			{
				const UMVec4f& ambient = material->ambient();
				glUniform4f(mat_ambient_location_, ambient.x, ambient.y, ambient.z, ambient.w);
			}
			
			if (mat_flags_location_ >= 0)
			{
				const UMVec4f& shader_flags = material->shader_flags();
				glUniform4f(mat_flags_location_, shader_flags.x, shader_flags.y, shader_flags.z, shader_flags.w);
			}
				
			// has textue ?
			if (umdraw::UMOpenGLTexturePtr texture = material->diffuse_texture())
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture->texture_id());
				if (sampler_location_ >= 0)
				{
					glUniform1i(sampler_location_ , 0);
				}
			}
		}

		unsigned int index_count = material->polygon_count() * 3;
		
#if !defined(WITH_EMSCRIPTEN)
		glBindVertexArray(vao);
#endif
		if (is_valid_vertex_index_vbo())
		{
			//unsigned int offset = sizeof(GLuint) * index_offset;
			unsigned int offset = index_offset;
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_index_vbo());
			glDrawElements(
				GL_TRIANGLES,
				(GLsizei)index_count,
				GL_UNSIGNED_INT,
				(const void*)offset);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
		else
		{
			glDrawArrays(
				GL_TRIANGLES,
				index_offset,
				index_count);
		}
		index_offset += index_count;
		
		glBindTexture(GL_TEXTURE_2D, 0);
	}
#if !defined(WITH_EMSCRIPTEN)
	glBindVertexArray(0);
#endif
	glUseProgram(0);
}

/**
 * update
 */
bool UMOpenGLMesh::Impl::update()
{
	return false;
}


UMOpenGLMesh::UMOpenGLMesh()
	: impl_(new UMOpenGLMesh::Impl)
{}

UMOpenGLMesh::~UMOpenGLMesh()
{
}

/** 
 * initialize
 */
bool UMOpenGLMesh::init()
{
	return impl_->init();
}

/** 
 * draw
 */
void UMOpenGLMesh::draw(UMOpenGLDrawParameterPtr parameter)
{
	impl_->draw(parameter);
}

/**
 * update
 */
bool UMOpenGLMesh::update()
{
	return impl_->update();
}

/**
 * get index buffer id id
 */
bool UMOpenGLMesh::is_valid_vertex_vbo() const 
{
	return impl_->is_valid_vertex_vbo();
}

/**
 * get vertex buffer id id
 */
bool UMOpenGLMesh::is_valid_vertex_index_vbo() const
{
	return impl_->is_valid_vertex_index_vbo();
}
	
/**
 * get normal buffer id id
 */
bool UMOpenGLMesh::is_valid_normal_vbo() const
{
	return impl_->is_valid_normal_vbo();
}
	
/**
 * get uv buffer id id
 */
bool UMOpenGLMesh::is_valid_uv_vbo() const
{
	return impl_->is_valid_uv_vbo();
}

/**
 * get index buffer id
 */
unsigned int  UMOpenGLMesh::vertex_index_vbo()
{
	return impl_->vertex_index_vbo();
}

/**
 * get vertex buffer id
 */
unsigned int UMOpenGLMesh::vertex_vbo()
{
	return impl_->vertex_vbo();
}

/**
 * get normal buffer id
 */
unsigned int UMOpenGLMesh::normal_vbo()
{
	return impl_->normal_vbo();
}


/**
 * get uv buffer id
 */
unsigned int UMOpenGLMesh::uv_vbo()
{
	return impl_->uv_vbo();
}
	
/**
 * set index buffer id
 */
void UMOpenGLMesh::set_vertex_index_vbo(unsigned int vbo)
{
	impl_->set_vertex_index_vbo(vbo);
}

/**
 * set vertex buffer id
 */
void UMOpenGLMesh::set_vertex_vbo(unsigned int vbo) 
{
	impl_->set_vertex_vbo(vbo);
}

/**
 * set normal buffer id
 */
void UMOpenGLMesh::set_normal_vbo(unsigned int vbo)
{
	impl_->set_normal_vbo(vbo);
}

/**
 * set uv buffer id
 */
void UMOpenGLMesh::set_uv_vbo(unsigned int vbo)
{
	impl_->set_uv_vbo(vbo);
}

/**
 * get material list
 */
const UMOpenGLMaterialList& UMOpenGLMesh::material_list() const
{
	return impl_->material_list();
}
	
/**
 * get material list
 */
UMOpenGLMaterialList& UMOpenGLMesh::mutable_material_list()
{
	return impl_->mutable_material_list();
}

/**
 * set draw parameter
 */
void UMOpenGLMesh::set_draw_parameter(umdraw::UMOpenGLDrawParameterPtr parameter)
{
	impl_->set_draw_parameter(parameter);
}


} // umdraw

#endif // WITH_OPENGL
