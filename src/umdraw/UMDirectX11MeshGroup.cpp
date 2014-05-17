/**
 * @file UMDirectX11MeshGroup.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_DIRECTX

#include "UMDirectX11MeshGroup.h"
#include "UMDirectX11DrawParameter.h"

#include "UMStringUtil.h"
#include "UMTime.h"
namespace umdraw
{
	
/**
 * init meseh group
 */
bool UMDirectX11MeshGroup::init(ID3D11Device *device_pointer)
{
	if (!device_pointer) return false;
	
	// init shader manager
	UMDirectX11ShaderManagerPtr shader_manager = std::make_shared<UMDirectX11ShaderManager>();
	shader_manager->init(device_pointer, UMDirectX11ShaderManager::eModel);

	draw_parameter_ = std::make_shared<UMDirectX11DrawParameter>();
	draw_parameter_->set_shader_manager(shader_manager);

	return true;
}

/**
 * update mesh group
 */
bool UMDirectX11MeshGroup::update(ID3D11Device *device_pointer)
{
	if (!device_pointer) return false;
	
	UMDirectX11MeshList::iterator it = dx11_mesh_list_.begin();
	for (; it != dx11_mesh_list_.end(); ++it)
	{
		UMDirectX11MeshPtr mesh = *it;
		mesh->set_draw_parameter(draw_parameter_);
	}

	return true;
}

/**
 * draw mesh group
 */
void UMDirectX11MeshGroup::draw(ID3D11Device* device_pointer, UMDirectX11DrawParameterPtr parameter)
{
	if (!device_pointer) return;

	UMDirectX11MeshList::iterator it = dx11_mesh_list_.begin();
	for (; it != dx11_mesh_list_.end(); ++it)
	{
		UMDirectX11MeshPtr mesh = *it;
		if (!mesh->ummesh()) continue;
		
		mesh->draw(device_pointer, parameter);
	}
}

} // umdraw

#endif // WITH_DIRECTX
