/**
 * @file UMOpenGLLine.h
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
#include <map>
#include "UMMacro.h"

namespace umdraw
{

class UMOpenGLLine;
typedef std::shared_ptr<UMOpenGLLine> UMOpenGLLinePtr;
typedef std::vector<UMOpenGLLinePtr> UMOpenGLLineList;

class UMOpenGLDrawParameter;
typedef std::shared_ptr<UMOpenGLDrawParameter> UMOpenGLDrawParameterPtr;

class UMOpenGLShaderManager;
typedef std::shared_ptr<UMOpenGLShaderManager> UMOpenGLShaderManagerPtr;

class UMOpenGLMaterial;
typedef std::shared_ptr<UMOpenGLMaterial> UMOpenGLMaterialPtr;
typedef std::vector<UMOpenGLMaterialPtr> UMOpenGLMaterialList;

class UMOpenGLShader;
typedef std::shared_ptr<UMOpenGLShader> UMOpenGLShaderPtr;

/**
 * opengl line
 */
class UMOpenGLLine
{
	DISALLOW_COPY_AND_ASSIGN(UMOpenGLLine);

public:

	UMOpenGLLine();

	~UMOpenGLLine();
	
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
	bool is_valid_vertex_vbo() const { return is_valid_vertex_vbo_; };

	/**
	 * get vertex buffer id
	 */
	unsigned int vertex_vbo() { return vertex_vbo_; };
	
	/**
	 * set vertex buffer id
	 */
	void set_vertex_vbo(unsigned int vbo) { vertex_vbo_ = vbo; is_valid_vertex_vbo_ = true; }
	
	/**
	 * set vertex count
	 */
	void set_vertex_count(unsigned int size) { vertex_count_ = size; }

	/**
	 * get veretx count
	 */
	unsigned int vertex_count() const { return vertex_count_; }
	
	/**
	 * set draw parameter
	 */
	void set_draw_parameter(umdraw::UMOpenGLDrawParameterPtr parameter) { draw_parameter_ = parameter; }

	/**
	 * get shader manager
	 */
	UMOpenGLShaderManagerPtr shader_manager() const { return shader_manager_; }
	
	/**
	 * get material list
	 */
	const UMOpenGLMaterialList& material_list() const { return material_list_; }
	
	/**
	 * get material list
	 */
	UMOpenGLMaterialList& mutable_material_list() { return material_list_; }

private:
	bool is_valid_vertex_vbo_;

	unsigned int vertex_vbo_;
	unsigned int vertex_count_;

	/**
	 * get shader manager
	 */
	UMOpenGLShaderManager& mutable_shader_manager() { return *shader_manager_; }

	UMOpenGLShaderManagerPtr shader_manager_;
	UMOpenGLDrawParameterPtr draw_parameter_;

	UMOpenGLMaterialList material_list_;

	typedef std::map<UMOpenGLMaterialPtr, unsigned int> VAOMap;
	VAOMap& mutable_vao_map() { return vao_map_; }
	VAOMap vao_map_;
	bool init_vao(UMOpenGLShaderPtr shader);
	bool init_vbo(UMOpenGLShaderPtr shader);

	int view_projection_location_;
	int mat_diffuse_location_;
	int mat_flags_location_;
};

} // umdraw
