/**
 * @file UMOpenGLMeshGroup.h
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <memory>
#include <vector>
#include "UMMacro.h"

namespace umdraw
{

class UMOpenGLMesh;
typedef std::shared_ptr<UMOpenGLMesh> UMOpenGLMeshPtr;
typedef std::vector<UMOpenGLMeshPtr> UMOpenGLMeshList;

class UMOpenGLMeshGroup;
typedef std::shared_ptr<UMOpenGLMeshGroup> UMOpenGLMeshGroupPtr;
typedef std::vector<UMOpenGLMeshGroupPtr> UMOpenGLMeshGroupList;

class UMOpenGLShaderManager;
typedef std::shared_ptr<UMOpenGLShaderManager> UMOpenGLShaderManagerPtr;

class UMMeshGroup;
typedef std::shared_ptr<UMMeshGroup> UMMeshGroupPtr;
typedef std::weak_ptr<UMMeshGroup> UMMeshGroupWeakPtr;

class UMOpenGLDrawParameter;
typedef std::shared_ptr<UMOpenGLDrawParameter> UMOpenGLDrawParameterPtr;

/**
 * opengl mesh group
 */
class UMOpenGLMeshGroup
{
	DISALLOW_COPY_AND_ASSIGN(UMOpenGLMeshGroup);

public:
	UMOpenGLMeshGroup();
	
	~UMOpenGLMeshGroup();

	/**
	 * init mesh group
	 */
	bool init();
	
	/**
	 * update mesh group
	 */
	bool update();

	/**
	 * draw mesh group
	 */
	void draw(UMOpenGLDrawParameterPtr parameter);

	/**
	 * get OpenGL mesh list
	 */
	const UMOpenGLMeshList& gl_mesh_list() const;
	
	/**
	 * get OpenGL mesh list
	 */
	UMOpenGLMeshList& mutable_gl_mesh_list();
	
	/**
	 * set ummesh group
	 */
	void set_ummesh_group(UMMeshGroupPtr ummesh_group);

	/**
	 * get ummesh group
	 */
	UMMeshGroupPtr ummesh_group();

private:
	class Impl;
	typedef std::unique_ptr<Impl> ImplPtr;
	ImplPtr impl_;
};

} // umdraw
