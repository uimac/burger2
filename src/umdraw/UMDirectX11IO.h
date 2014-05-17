/**
 * @file UMDirectX11IO.h
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include "UMMacro.h"

#include "UMDirectX11Scene.h"

namespace umio
{
	class UMObject;
	typedef std::shared_ptr<UMObject> UMObjectPtr;

} // umio

namespace umdraw
{

class UMDirectX11Line;
typedef std::shared_ptr<UMDirectX11Line> UMDirectX11LinePtr;


/**
 * model in/out
 */
class UMDirectX11IO
{
	DISALLOW_COPY_AND_ASSIGN(UMDirectX11IO);

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
		const std::u16string& absolute_file_path);
	
	/**
	 * convert umdraw mesh to directx11 mesh
	 * @param [in] device directx11 device pointer
	 * @param [in] src source umdraw mesh
	 * @retval UMDirectX11MeshPtr converted dx11 mesh or none
	 */
	static UMDirectX11MeshPtr convert_mesh_to_dx11_mesh(
		ID3D11Device *device_pointer,
		UMMeshPtr src);
	
	/**
	 * convert umdraw mesh group to directx11 mesh
	 * @param [in] device directx11 device pointer
	 * @param [in] src source umdraw mesh group
	 * @retval UMDirectX11MeshGroupPtr converted dx11 mesh group or none
	 */
	static UMDirectX11MeshGroupPtr convert_mesh_group_to_dx11_mesh_group(
		ID3D11Device *device_pointer,
		UMMeshGroupPtr src);
	
	/**
	 * convert umdraw line to directx11 line
	 * @param [in] device directx11 device pointer
	 * @param [in] src source umdraw mesh group
	 * @retval UMDirectX11LinePtr converted dx11 line or none
	 */
	static UMDirectX11LinePtr convert_line_to_dx11_line(
		ID3D11Device *device_pointer,
		UMLinePtr src);
	
	/**
	 * convert umdraw light to directx11 light
	 * @param [in] device directx11 device pointer
	 * @param [in] src source umdraw light
	 * @retval UMDirectX11LightPtr converted dx11 light or none
	 */
	static UMDirectX11LightPtr convert_light_to_dx11_light(
		ID3D11Device *device_pointer,
		UMLightPtr src);
	
	/**
	 * convert umdraw material to directx11 material
	 * @param [in] device directx11 device pointer
	 * @param [in] src source umdraw material
	 * @retval UMDirectX11MaterialPtr converted dx11 material or none
	 */
	static UMDirectX11MaterialPtr convert_material_to_dx11_material(
		ID3D11Device *device_pointer,
		UMMaterialPtr src);
	
	/**
	 * convert umdraw camera to directx11 camera
	 * @param [in] device directx11 device pointer
	 * @param [in] src source umdraw camera
	 * @retval UMCameraPtr converted dx11 camera or none
	 */
	static UMDirectX11CameraPtr convert_camera_to_dx11_camera(
		ID3D11Device *device_pointer,
		UMCameraPtr src);
};

} // umdraw
