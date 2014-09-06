/**
 * @file UMOpenGLNode.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_OPENGL

#include "UMOpenGLNode.h"
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

class UMOpenGLNode::Impl
{
	DISALLOW_COPY_AND_ASSIGN(Impl);
public:

	Impl()
		: is_valid_vertex_vbo_(false)
		, shader_manager_(std::make_shared<UMOpenGLShaderManager>())
		, vertex_vbo_(0) 
		, vertex_count_(0)
		, view_projection_location_(-1)
		, mat_diffuse_location_(-1)
		, mat_flags_location_(-1)
	{}

	~Impl() 
	{
		if (!is_valid_vertex_vbo_) {
			glDeleteBuffers(1, &vertex_vbo_);
		}
	}

	bool init()
	{
		// init shader manager
		shader_manager_->init(UMOpenGLShaderManager::eModel);
		return true;
	}
	
	/** 
	 * init vbo
	 */
	bool init_vbo(UMOpenGLShaderPtr shader)
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
	bool init_vao(UMOpenGLShaderPtr shader)
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
			unsigned int index_count = material->polygon_count() * 3;
	
	#if !defined(WITH_EMSCRIPTEN)
			unsigned int& vao = mutable_vao_map()[material];
			if (vao == 0) continue;
			glBindVertexArray(vao);
	#endif
			glUniform4f(mat_diffuse_location_, 0.0f, 0.0f, 0.0f, 1.0f);
			glDrawArrays(GL_LINE_STRIP, index_offset, index_count);

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

			glDrawArrays(GL_TRIANGLES, index_offset, index_count);
		
			index_offset += index_count;
		}
	
	#if !defined(WITH_EMSCRIPTEN)
		glBindVertexArray(0);
	#endif
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
	/**
	 * get material list
	 */
	const UMOpenGLMaterialList& material_list() const { return material_list_; }
	
	/**
	 * get material list
	 */
	UMOpenGLMaterialList& mutable_material_list() { return material_list_; }
	
	/**
	 * get shader manager
	 */
	UMOpenGLShaderManagerPtr shader_manager() const { return shader_manager_; }
private:
	
	/**
	 * get shader manager
	 */
	UMOpenGLShaderManager& mutable_shader_manager() { return *shader_manager_; }
	
	typedef std::map<UMOpenGLMaterialPtr, unsigned int> VAOMap;
	VAOMap& mutable_vao_map() { return vao_map_; }
	VAOMap vao_map_;
	UMOpenGLShaderManagerPtr shader_manager_;
	bool is_valid_vertex_vbo_;
	unsigned int vertex_vbo_;
	unsigned int vertex_count_;
	int view_projection_location_;
	UMOpenGLDrawParameterPtr draw_parameter_;
	int mat_diffuse_location_;
	int mat_flags_location_;
	UMOpenGLMaterialList material_list_;
};

/// constructor
UMOpenGLNode::UMOpenGLNode() 
	: impl_(new UMOpenGLNode::Impl)
{}

/// destructor
UMOpenGLNode::~UMOpenGLNode()
{
}

/** 
 * initialize
 */
bool UMOpenGLNode::init()
{
	return impl_->init();
}

/**
 * draw
 */
void UMOpenGLNode::draw(UMOpenGLDrawParameterPtr parameter)
{
	impl_->draw(parameter);
}

/**
 * get index buffer id id
 */
bool UMOpenGLNode::is_valid_vertex_vbo() const 
{
	return impl_->is_valid_vertex_vbo();
}

/**
 * get vertex buffer id
 */
unsigned int UMOpenGLNode::vertex_vbo()
{
	return impl_->vertex_vbo();
}
	
/**
 * set vertex buffer id
 */
void UMOpenGLNode::set_vertex_vbo(unsigned int vbo)
{
	return impl_->set_vertex_vbo(vbo);
}
	
/**
 * set vertex count
 */
void UMOpenGLNode::set_vertex_count(unsigned int size)
{
	return impl_->set_vertex_count(size);
}

/**
 * get veretx count
 */
unsigned int UMOpenGLNode::vertex_count() const
{
	return impl_->vertex_count();
}
	
/**
 * set draw parameter
 */
void UMOpenGLNode::set_draw_parameter(umdraw::UMOpenGLDrawParameterPtr parameter)
{
	return impl_->set_draw_parameter(parameter);
}

/**
 * get material list
 */
const UMOpenGLMaterialList& UMOpenGLNode::material_list() const
{
	return impl_->material_list();
}
	
/**
	* get material list
	*/
UMOpenGLMaterialList& UMOpenGLNode::mutable_material_list()
{
	return impl_->mutable_material_list();
}

/**
 * get shader manager
 */
UMOpenGLShaderManagerPtr UMOpenGLNode::shader_manager() const
{
	return impl_->shader_manager();
}

} // umdraw

#endif // WITH_OPENGL
