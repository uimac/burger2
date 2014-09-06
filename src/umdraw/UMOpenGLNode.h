/**
 * @file UMOpenGLNode.h
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
#include <vector>
#include "UMMacro.h"

namespace umdraw
{

class UMOpenGLNode;
typedef std::shared_ptr<UMOpenGLNode> UMOpenGLNodePtr;
typedef std::vector<UMOpenGLNodePtr> UMOpenGLNodeList;

class UMOpenGLDrawParameter;
typedef std::shared_ptr<UMOpenGLDrawParameter> UMOpenGLDrawParameterPtr;

class UMOpenGLMaterial;
typedef std::shared_ptr<UMOpenGLMaterial> UMOpenGLMaterialPtr;
typedef std::vector<UMOpenGLMaterialPtr> UMOpenGLMaterialList;

class UMOpenGLShaderManager;
typedef std::shared_ptr<UMOpenGLShaderManager> UMOpenGLShaderManagerPtr;

/**
 * a shader
 */
class UMOpenGLNode
{
	DISALLOW_COPY_AND_ASSIGN(UMOpenGLNode);

public:

	UMOpenGLNode();

	~UMOpenGLNode();
	
	/** 
	 * initialize
	 */
	bool init();
	
	/** 
	 * update
	 */
	bool update() { return false; }

	/**
	 * draw
	 */
	void draw(UMOpenGLDrawParameterPtr parameter);
	
	/**
	 * get index buffer id id
	 */
	bool is_valid_vertex_vbo() const;

	/**
	 * get vertex buffer id
	 */
	unsigned int vertex_vbo();
	
	/**
	 * set vertex buffer id
	 */
	void set_vertex_vbo(unsigned int vbo);
	
	/**
	 * set vertex count
	 */
	void set_vertex_count(unsigned int size);

	/**
	 * get veretx count
	 */
	unsigned int vertex_count() const;
	
	/**
	 * set draw parameter
	 */
	void set_draw_parameter(umdraw::UMOpenGLDrawParameterPtr parameter);
	
	/**
	 * get shader manager
	 */
	UMOpenGLShaderManagerPtr shader_manager() const;
	
	/**
	 * get material list
	 */
	const UMOpenGLMaterialList& material_list() const;
	
	/**
	 * get material list
	 */
	UMOpenGLMaterialList& mutable_material_list();

private:
	class Impl;
	typedef std::unique_ptr<Impl> ImplPtr;
	ImplPtr impl_;
};

} // umdraw
