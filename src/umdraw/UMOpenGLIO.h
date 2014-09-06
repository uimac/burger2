/**
 * @file UMOpenGLIO.h
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <memory>
#include "UMMacro.h"

#include "UMOpenGLScene.h"

namespace umio
{
	class UMObject;
	typedef std::shared_ptr<UMObject> UMObjectPtr;
} // umio

namespace umdraw
{

class UMOpenGLMaterial;
typedef std::shared_ptr<UMOpenGLMaterial> UMOpenGLMaterialPtr;

class UMLine;
typedef std::shared_ptr<UMLine> UMLinePtr;
class UMOpenGLLine;
typedef std::shared_ptr<UMOpenGLLine> UMOpenGLLinePtr;

class UMOpenGLNode;
typedef std::shared_ptr<UMOpenGLNode> UMOpenGLNodePtr;

/**
 * model in/out
 */
class UMOpenGLIO
{
	DISALLOW_COPY_AND_ASSIGN(UMOpenGLIO);

public:
	
	/** 
	 * import umdraw mesh list
	 * @param [out] dst distination mesh list
	 * @param [in] src source object
	 * @param [in] absolute_file_path file path
	 */
	static bool import_mesh_list(
		UMMeshList& dst, 
		const umio::UMObjectPtr src,
		const umstring& absolute_file_path);
	
	/**
	 * convert umdraw mesh to OpenGL mesh
	 * @param [in] src source umdraw mesh
	 * @retval UMOpenGLMeshPtr converted gl mesh or none
	 */
	static UMOpenGLMeshPtr convert_mesh_to_gl_mesh(
		UMMeshPtr src);
	
	/**
	 * convert umdraw mesh to OpenGL mesh
	 */
	static bool UMOpenGLIO::deformed_mesh_to_gl_mesh(
		UMOpenGLMeshPtr deform_mesh,
		UMMeshPtr src);

	/**
	 * convert umdraw mesh group to OpenGL mesh
	 * @param [in] src source umdraw mesh group
	 * @retval UMMeshGroupPtr converted gl mesh group or none
	 */
	static UMOpenGLMeshGroupPtr convert_mesh_group_to_gl_mesh_group(
		UMMeshGroupPtr src);
	
	/**
	 * convert umdraw node to OpenGL node
	 */
	static UMOpenGLNodePtr UMOpenGLIO::convert_node_to_gl_node(
		UMNodePtr src);

	/**
	 * convert umdraw node to OpenGL node
	 */
	static bool UMOpenGLIO::deformed_node_to_gl_node(
		UMOpenGLNodePtr deform_node,
		UMNodePtr src);

	/**
	 * convert umdraw line to OpenGL line
	 * @param [in] src source umdraw line
	 * @retval UMOpenGLLinePtr converted gl line or none
	 */
	static UMOpenGLLinePtr convert_line_to_gl_line(
		UMLinePtr src);
	
	/**
	 * convert umdraw light to OpenGL light
	 * @param [in] src source umdraw light
	 * @retval UMOpenGLLightPtr converted gl light or none
	 */
	static UMOpenGLLightPtr convert_light_to_gl_light(
		UMLightPtr src);
	
	/**
	 * convert umdraw material to OpenGL material
	 * @param [in] src source umdraw material
	 * @retval UMOpenGLMaterialPtr converted gl material or none
	 */
	static UMOpenGLMaterialPtr convert_material_to_gl_material(
		UMMaterialPtr src);
	
	/**
	 * convert umdraw camera to OpenGL camera
	 * @param [in] src source umdraw camera
	 * @retval UMCameraPtr converted gl camera or none
	 */
	static UMOpenGLCameraPtr convert_camera_to_gl_camera(
		UMCameraPtr src);
};

} // umdraw
