/**
 * @file UMOpenGLShader.h
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <memory>
#include <string>
#include "UMMacro.h"

namespace umdraw
{

class UMOpenGLShader;
typedef std::shared_ptr<UMOpenGLShader> UMOpenGLShaderPtr;

/**
 * a shader
 */
class UMOpenGLShader
{
	DISALLOW_COPY_AND_ASSIGN(UMOpenGLShader);

public:

	UMOpenGLShader();

	~UMOpenGLShader();

	/**
	 * create shader from string
	 */
	bool create_shader_from_memory(
		const std::string& vertex_shader, 
		const std::string& fragment_shader);

	/**
	 * create shader from file
	 */
	bool create_shader_from_file(
		const std::u16string& vertex_shader_file_path,
		const std::u16string& fragment_shader_file_path);

	/**
	 * get program object
	 */
	unsigned int program_object() const;

	/**
	 * get vertex shader
	 */
	unsigned int vertex_shader() const;

	/**
	 * get pixel shader
	 */
	unsigned int fragment_shader() const;

private:
	class Impl;
	typedef std::unique_ptr<Impl> ImplPtr;
	ImplPtr impl_;
};

} // umdraw
