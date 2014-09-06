/**
 * @file UMDirectX11Node.h
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

class UMDirectX11Node;
typedef std::shared_ptr<UMDirectX11Node> UMDirectX11NodePtr;

class UMDirectX11DrawParameter;
typedef std::shared_ptr<UMDirectX11DrawParameter> UMDirectX11DrawParameterPtr;

/**
 * a shader
 */
class UMDirectX11Node
{
	DISALLOW_COPY_AND_ASSIGN(UMDirectX11Node);

public:

	UMDirectX11Node();

	~UMDirectX11Node();
	
	/** 
	 * initialize
	 */
	bool init() { return false; }
	
	/** 
	 * update
	 */
	bool update() { return false; }

	/**
	 * draw
	 */
	void draw(UMDirectX11DrawParameterPtr parameter);
	
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
	void set_draw_parameter(umdraw::UMDirectX11DrawParameterPtr parameter);

private:
	class Impl;
	typedef std::unique_ptr<Impl> ImplPtr;
	ImplPtr impl_;
};

} // umdraw
