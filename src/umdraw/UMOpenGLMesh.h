/**
 * @file UMOpenGLMesh.h
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>
#include "UMMacro.h"

namespace umdraw
{

class UMOpenGLMesh;
typedef std::shared_ptr<UMOpenGLMesh> UMOpenGLMeshPtr;
typedef std::vector<UMOpenGLMeshPtr> UMOpenGLMeshList;

class UMOpenGLMaterial;
typedef std::shared_ptr<UMOpenGLMaterial> UMOpenGLMaterialPtr;
typedef std::vector<UMOpenGLMaterialPtr> UMOpenGLMaterialList;

class UMOpenGLDrawParameter;
typedef std::shared_ptr<UMOpenGLDrawParameter> UMOpenGLDrawParameterPtr;

class UMOpenGLShader;
typedef std::shared_ptr<UMOpenGLShader> UMOpenGLShaderPtr;

/**
 * opengl mesh
 */
class UMOpenGLMesh
{
	DISALLOW_COPY_AND_ASSIGN(UMOpenGLMesh);

public:
	UMOpenGLMesh();
	
	~UMOpenGLMesh();
	
	/** 
	 * initialize
	 */
	bool init();
	
	/** 
	 * update
	 */
	bool update() { return true; }

	/**
	 * draw
	 */
	void draw(UMOpenGLDrawParameterPtr parameter);

	/**
	 * get index buffer id id
	 */
	bool is_valid_vertex_vbo() const;
	
	/**
	 * get vertex buffer id id
	 */
	bool is_valid_vertex_index_vbo() const;
	
	/**
	 * get normal buffer id id
	 */
	bool is_valid_normal_vbo() const;
	
	/**
	 * get uv buffer id id
	 */
	bool is_valid_uv_vbo() const;

	/**
	 * get index buffer id
	 */
	unsigned int  vertex_index_vbo();

	/**
	 * get vertex buffer id
	 */
	unsigned int vertex_vbo();

	/**
	 * get normal buffer id
	 */
	unsigned int normal_vbo();

	/**
	 * get uv buffer id
	 */
	unsigned int uv_vbo();
	
	/**
	 * set index buffer id
	 */
	void set_vertex_index_vbo(unsigned int vbo);

	/**
	 * set vertex buffer id
	 */
	void set_vertex_vbo(unsigned int vbo);

	/**
	 * set normal buffer id
	 */
	void set_normal_vbo(unsigned int vbo);

	/**
	 * set uv buffer id
	 */
	void set_uv_vbo(unsigned int vbo);

	/**
	 * get material list
	 */
	const UMOpenGLMaterialList& material_list() const;
	
	/**
	 * get material list
	 */
	UMOpenGLMaterialList& mutable_material_list();
	
	/**
	 * set draw parameter
	 */
	void set_draw_parameter(umdraw::UMOpenGLDrawParameterPtr parameter);

private:
	class Impl;
	typedef std::unique_ptr<Impl> ImplPtr;
	ImplPtr impl_;
};

} // umdraw
