/**
 * @file UMOpenGLShader.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_OPENGL

#include "UMOpenGLShader.h"

#include <map>
#include <fstream>
#include <string>
#include <iterator>
#include <iostream>
#include <tchar.h>
#include <GL/glew.h>

#include "UMMacro.h"
#include "UMPath.h"
#include "UMStringUtil.h"

namespace
{
	GLuint load_shader( GLenum type, const char *src )
	{
		GLuint	shader;
		shader = glCreateShader( type );
		glShaderSource( shader, 1, &src, NULL );
		glCompileShader( shader );
		return shader;
	}

	GLuint load_shader_from_resource( GLenum type, const std::u16string& file_path)
	{
		std::ifstream ifs( file_path.c_str());
		if (!ifs.good()) return 0;
		std::string str( (std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		const char* cstr = str.c_str();
		
		GLuint	shader;
		shader = glCreateShader( type );
		glShaderSource( shader, 1, &cstr, NULL );
		glCompileShader( shader );
		return shader;
	}

	void initialize( 
		unsigned int& program_object,
		unsigned int& vertex_shader,
		unsigned int& fragment_shader,
		const std::u16string& vertex_shader_path,
		const std::u16string& fragment_shader_path)
	{
		vertex_shader = load_shader_from_resource( GL_VERTEX_SHADER, vertex_shader_path );
		fragment_shader = load_shader_from_resource( GL_FRAGMENT_SHADER, fragment_shader_path );

		if (program_object == 0)
		{
			program_object = glCreateProgram();
		}
		glAttachShader(program_object, vertex_shader );
		glAttachShader(program_object, fragment_shader );

		glLinkProgram( program_object );
		glUseProgram( program_object );
		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			error = error;
		}
	}
	
	void release(
		unsigned int& program_object,
		unsigned int& vertex_shader,
		unsigned int& fragment_shader,
		unsigned int& texture_id)
	{
		glDetachShader(program_object, vertex_shader);
		glDetachShader(program_object, fragment_shader);

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		glDeleteTextures(1, &texture_id);
		glDeleteProgram(program_object);
	}

} // anonymouse namespace

namespace umdraw
{
	
	
class UMOpenGLShader::Impl
{
	DISALLOW_COPY_AND_ASSIGN(Impl);
public:

	Impl()
		: program_object_(0)
		, vertex_shader_(0)
		, fragment_shader_(0)
		, texture_id_(0)
	{}

	~Impl() 
	{
		release(program_object_,
			vertex_shader_,
			fragment_shader_,
			texture_id_
			);
	}
	
	bool create_shader_from_memory(
		const std::string& vertex_shader, 
		const std::string& fragment_shader)
	{
		vertex_shader_ = load_shader(GL_VERTEX_SHADER, vertex_shader.c_str());
		fragment_shader_ = load_shader(GL_FRAGMENT_SHADER, fragment_shader.c_str());
	
		program_object_ = glCreateProgram();
		glAttachShader( program_object_, vertex_shader_ );
		glAttachShader( program_object_, fragment_shader_ );
		glLinkProgram( program_object_ );
		glUseProgram( program_object_ );
		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			error = error;
		}
		return true;
	}

	bool create_shader_from_file(
		const std::u16string& veretx_shader_path, 
		const std::u16string& fragment_shader_path)
	{
		initialize(
			program_object_,
			vertex_shader_, 
			fragment_shader_,
			veretx_shader_path,
			fragment_shader_path);
	
		return true;
	}
	
	unsigned int program_object() const
	{
		return program_object_;
	}

	unsigned int vertex_shader() const 
	{
		return vertex_shader_;
	}

	unsigned int fragment_shader() const 
	{
		return fragment_shader_;
	}

private:
	unsigned int program_object_;
	unsigned int vertex_shader_;
	unsigned int fragment_shader_;
	unsigned int texture_id_;
};


/// constructor
UMOpenGLShader::UMOpenGLShader()
	: impl_(new UMOpenGLShader::Impl())
{}

/// destructor
UMOpenGLShader::~UMOpenGLShader()
{
}

/**
 * create shader from string
 */
bool UMOpenGLShader::create_shader_from_memory(
	const std::string& vertex_shader, 
	const std::string& fragment_shader)
{
	return impl_->create_shader_from_memory(vertex_shader, fragment_shader);
}

/**
 * create shader from file
 */
bool UMOpenGLShader::create_shader_from_file(
	const std::u16string& veretx_shader_path, 
	const std::u16string& fragment_shader_path)
{
	return impl_->create_shader_from_file(veretx_shader_path, fragment_shader_path);
}

/**
 * get program object
 */
unsigned int UMOpenGLShader::program_object() const
{
	return impl_->program_object();
}

/**
 * get vertex shader
 */
unsigned int UMOpenGLShader::vertex_shader() const 
{
	return impl_->vertex_shader();
}

/**
 * get pixel shader
 */
unsigned int UMOpenGLShader::fragment_shader() const 
{
	return impl_->fragment_shader();
}


} // umdraw

#endif // WITH_OPENGL
