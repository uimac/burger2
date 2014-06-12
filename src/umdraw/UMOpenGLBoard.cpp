/**
 * @file UMOpenGLBoard.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_OPENGL

#include "UMOpenGLBoard.h"

#include "UMOpenGLTexture.h"
#include "UMOpenGLShaderManager.h"
#include "UMOpenGLDrawParameter.h"
#include "UMOpenGLLight.h"

#include <GL/glew.h>

namespace umdraw
{
	
class UMOpenGLBoard::Impl
{
	DISALLOW_COPY_AND_ASSIGN(Impl);
public:

	Impl(UMVec2f left_top,
		UMVec2f right_bottom,
		float z)
		: vertex_vbo_(0)
		, is_valid_vertex_vbo_(false)
		, uv_vbo_(0)
		, is_valid_uv_vbo_(false)
		, is_frag_color_binded_(false)
		, sampler_location_(-1)
		, position_sampler_location_(-1)
		, diffuse_sampler_location_(-1)
		, normal_sampler_location_(-1)
		, uv_sampler_location_(-1)
		, light_position_location_(-1)
		, vao_(0)
	{
		// culling is CCW
		verts_.push_back(UMVec3f(right_bottom.x, left_top.y, z));
		uvs_.push_back(UMVec2f(1.0f, 1.0f));

		verts_.push_back(UMVec3f(left_top.x, left_top.y, z));
		uvs_.push_back(UMVec2f(0.0f, 1.0f));

		verts_.push_back(UMVec3f(right_bottom.x, right_bottom.y, z));
		uvs_.push_back(UMVec2f(1.0f, 0.0f));

		verts_.push_back(UMVec3f(left_top.x, right_bottom.y, z));
		uvs_.push_back(UMVec2f(0.0f, 0.0f));
	
		shader_manager_ = std::make_shared<UMOpenGLShaderManager>();
	}
	~Impl() 
	{

	}
	
	bool init()
	{
		// init shader manager
		shader_manager_->init(UMOpenGLShaderManager::eBoard);

		return true;
	}
	
	bool init_vbo(UMOpenGLShaderPtr shader)
	{
		// create vertex buffer
		{
			unsigned int vertex_vbo = vertex_vbo_;
			if (!is_valid_vertex_vbo_)
			{
				glGenBuffers(1, &vertex_vbo);
				glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo);
			}
			if (vertex_vbo == 0) return false;
			vertex_vbo_ = vertex_vbo;
			is_valid_vertex_vbo_ = true;
			glBindBuffer(GL_ARRAY_BUFFER, vertex_vbo_);
		
			size_t vertex_size = verts_.size();
			glBufferData(GL_ARRAY_BUFFER,
				sizeof (UMVec3f) * vertex_size,
				reinterpret_cast<const GLvoid*>( &(*verts_.begin()) ), 
				GL_STATIC_DRAW );
			
			unsigned int offset = 0;
			GLuint position_attr = glGetAttribLocation(shader->program_object(), "a_position" );
			glEnableVertexAttribArray(position_attr);
			glVertexAttribPointer(position_attr, 3, GL_FLOAT, GL_FALSE, 0, (const void*)offset);
		}
		
		// create uv buffer
		{
			unsigned int uv_vbo = uv_vbo_;
			if (!is_valid_uv_vbo_)
			{
				glGenBuffers(1, &uv_vbo);
				glBindBuffer(GL_ARRAY_BUFFER, uv_vbo);
			}
			if (uv_vbo == 0) return false;
			uv_vbo_ = uv_vbo;
			is_valid_uv_vbo_ = true;
			glBindBuffer(GL_ARRAY_BUFFER, uv_vbo_);
			
			const size_t uv_size = uvs_.size();
			glBufferData(GL_ARRAY_BUFFER,
				sizeof (UMVec2f) * uv_size,
				reinterpret_cast<const GLvoid*>( &(*uvs_.begin()) ), 
				GL_STATIC_DRAW );
			
			unsigned int offset = 0;
			GLuint uv_attr = glGetAttribLocation(shader->program_object(), "a_uv" );
			glEnableVertexAttribArray(uv_attr);
			glVertexAttribPointer(uv_attr, 2, GL_FLOAT, GL_FALSE, 0, (const void*)offset);
		}
		return true;
	}

	bool init_vao(UMOpenGLShaderPtr shader)
	{
#if !defined(WITH_EMSCRIPTEN)
		if (vao_ <= 0)
		{
			// create vao
			glGenVertexArrays(1, &vao_);
			glBindVertexArray(vao_);
			init_vbo(shader);
			glBindVertexArray(0);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
#else
		init_vbo(shader);
#endif
		return true;
	}

	
	bool update()
	{
		return true;
	}

	void draw(UMOpenGLDrawParameterPtr parameter)
	{
		UMOpenGLLightPtr light;
		UMOpenGLCameraPtr camera;
		UMOpenGLShaderManagerPtr shader_manager = shader_manager_;
		if (parameter)
		{
			light = parameter->light();
			shader_manager = parameter->shader_manager();
		}

		// shader list
		const UMOpenGLShaderManager::ShaderList& shaders = shader_manager->shader_list();
		if (shaders.empty()) return;
	
		UMOpenGLShaderPtr shader = shaders[0];

		//if (!is_frag_color_binded_)
		//{
		//	glBindFragDataLocation(shader->program_object(), 0, "frag_color");
		//	is_frag_color_binded_ = true;
		//}
		if (sampler_location_  == -1)
		{
			sampler_location_ = glGetUniformLocation(shader->program_object(), "s_texture");
			if (sampler_location_ < 0) { sampler_location_ = -2; }
		}
		if (position_sampler_location_ == -1)
		{
			position_sampler_location_ = glGetUniformLocation(shader->program_object(), "position_texture");
			if (position_sampler_location_ < 0) { position_sampler_location_ = -2; }
		}
		if (diffuse_sampler_location_ == -1)
		{
			diffuse_sampler_location_ = glGetUniformLocation(shader->program_object(), "diffuse_texture");
			if (diffuse_sampler_location_ < 0) { diffuse_sampler_location_ = -2; }
		}
		if (normal_sampler_location_ == -1)
		{
			normal_sampler_location_ = glGetUniformLocation(shader->program_object(), "normal_texture");
			if (normal_sampler_location_ < 0) { normal_sampler_location_ = -2; }
		}
		if (uv_sampler_location_ == -1)
		{
			uv_sampler_location_ = glGetUniformLocation(shader->program_object(), "uv_texture");
			if (uv_sampler_location_ < 0) { uv_sampler_location_ = -2; }
		}
		if (light_position_location_ == -1)
		{
			light_position_location_  = glGetUniformLocation(shader->program_object(), "light_position");
			if (light_position_location_ < 0) { light_position_location_ = -2; }
		}
		
		glUseProgram(shader->program_object());
		init_vao(shader);
		
		if (light)
		{
			if (light_position_location_ >= 0)
			{
				const UMVec4f& pos = light->position();
				glUniform4f(light_position_location_, pos.x, pos.y, pos.z, 0.0f);
			}
		}

		// has textue ?
		if (texture_)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture_->texture_id());
			if (sampler_location_ >= 0)
			{
				glUniform1i(sampler_location_ , 0);
			}
		}
		if (!attachments_.empty())
		{
			const int size = static_cast<int>(attachments_.size());
			if (size > 0)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, attachments_.at(0)->texture_id());
				glUniform1i(position_sampler_location_ , 0);
			}
			if (size > 1)
			{
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, attachments_.at(1)->texture_id());
				glUniform1i(diffuse_sampler_location_ , 1);
			}
			if (size > 2)
			{
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, attachments_.at(2)->texture_id());
				glUniform1i(normal_sampler_location_ , 2);
			}
			if (size > 3)
			{
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, attachments_.at(3)->texture_id());
				glUniform1i(uv_sampler_location_ , 3);
			}
		}
		
#if !defined(WITH_EMSCRIPTEN)
		glBindVertexArray(vao_);
#endif
		glDrawArrays(
			GL_TRIANGLE_STRIP,
			0,
			4);
#if !defined(WITH_EMSCRIPTEN)
		glBindVertexArray(0);
#endif
		glUseProgram(0);
	}
	
	void set_texture(UMOpenGLTexturePtr texture)
	{
		texture_ = texture;
	}

	void set_color_attachments(const UMOpenGLTextureList& attachments)
	{
		attachments_ = attachments;
	}
	
	UMOpenGLTexturePtr texture() const 
	{
		return texture_;
	}

private:

	std::vector<UMVec3f> verts_;
	std::vector<UMVec2f> uvs_;

	bool is_valid_vertex_vbo_;
	unsigned int vertex_vbo_;
	
	bool is_valid_uv_vbo_;
	unsigned int uv_vbo_;

	unsigned int vao_;

	bool is_frag_color_binded_;
	int sampler_location_;
	int position_sampler_location_;
	int diffuse_sampler_location_;
	int normal_sampler_location_;
	int uv_sampler_location_;
	int light_position_location_;

	UMOpenGLShaderManagerPtr shader_manager_;
	UMOpenGLTexturePtr texture_;
	UMOpenGLTextureList attachments_;
};


/**
 * constructor
 */
UMOpenGLBoard::UMOpenGLBoard(
	UMVec2f left_top,
	UMVec2f right_bottom,
	float z)
	: impl_(new UMOpenGLBoard::Impl(left_top, right_bottom, z))
{
}

/**
 * destructor
 */
UMOpenGLBoard::~UMOpenGLBoard()
{
}

/** 
 * initialize
 */
bool UMOpenGLBoard::init()
{
	return impl_->init();
}

/**
 * update
 */
bool UMOpenGLBoard::update()
{
	return impl_->update();
}

/**
 * draw board
 */
void UMOpenGLBoard::draw(UMOpenGLDrawParameterPtr parameter)
{
	impl_->draw(parameter);
}

/**
 * set texture
 */
void UMOpenGLBoard::set_texture(UMOpenGLTexturePtr texture)
{
	impl_->set_texture(texture);
}

/**
 * set color attachments for deferred rendering
 */
void UMOpenGLBoard::set_color_attachments(const UMOpenGLTextureList& attachments)
{
	impl_->set_color_attachments(attachments);
}

/**
 * get texture
 */
UMOpenGLTexturePtr UMOpenGLBoard::texture() const
{
	return impl_->texture();
}

} // umdraw

#endif // WITH_OPENGL
