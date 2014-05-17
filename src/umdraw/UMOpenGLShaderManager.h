/**
 * @file UMOpenGLShaderManager.h
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <vector>
#include <memory>
#include "UMMacro.h"
#include "UMOpenGLShader.h"

namespace umdraw
{

class UMOpenGLShaderManager;
typedef std::shared_ptr<UMOpenGLShaderManager> UMOpenGLShaderManagerPtr;
typedef std::weak_ptr<UMOpenGLShaderManager> UMOpenGLShaderManagerWeakPtr;

/**
 * opengl shader manager
 */
class UMOpenGLShaderManager
{
	DISALLOW_COPY_AND_ASSIGN(UMOpenGLShaderManager);

public:
	typedef std::vector<UMOpenGLShaderPtr> ShaderList;
	typedef std::vector<unsigned int> BufferList;

	enum ShaderType {
		eConstants,
		eBoard,
		eModel,
		ePoint,
		eOriginal,
		eBoardForDeferred,
		eModelDeferredGeo,
	};

	UMOpenGLShaderManager();

	~UMOpenGLShaderManager();
	
	/**
	 * initialize
	 * @param [in] type shader type
	 */
	bool init(ShaderType type);

	/**
	 * get constant buffer list
	 */
	const BufferList& uniform_buffer_list() const;
	
	/**
	 * get shader list
	 */
	const ShaderList& shader_list() const;

	/**
	 * get shader list
	 */
	ShaderList& mutable_shader_list();

	/**
	 * is deferred lighting
	 */
	bool is_deferred() const;

private:
	class Impl;
	typedef std::unique_ptr<Impl> ImplPtr;
	ImplPtr impl_;
};

} // umdraw
