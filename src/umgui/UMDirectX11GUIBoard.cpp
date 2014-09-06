/**
 * @file UMDirectX11GUIBoard.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_DIRECTX

#include "UMDirectX11GUIBoard.h"
#include "UMGUIBoard.h"
#include "UMDirectX11IO.h"
#include "UMDirectX11Mesh.h"
#include "UMMesh.h"
#include "UMGUIEventType.h"

namespace umgui
{

/**
 * constructor
 */
UMDirectX11GUIBoard::UMDirectX11GUIBoard(UMGUIObjectPtr board)
	: board_(board)
{
}

/**
 * initialize
 */
bool UMDirectX11GUIBoard::init(ID3D11Device* device_pointer)
{
	if (UMGUIObjectPtr board = board_.lock())
	{
		if (board->is_root()) return true;
		dx_mesh_ = umdraw::UMDirectX11IO::convert_mesh_to_dx11_mesh(device_pointer, board->mesh());
		if (dx_mesh_->init(device_pointer))
		{
			return true;
		}
	}
	return false;
}

/**
 * update
 */
bool UMDirectX11GUIBoard::update(ID3D11Device* device_pointer)
{
	if (UMGUIObjectPtr board = board_.lock())
	{
		if (board->is_root()) return true;
		umdraw::UMMeshPtr mesh = board->mesh();
		if (mesh->local_transform() != pre_local_transform_)
		{
			mesh->update();
			if (dx_mesh_)
			{
				umdraw::UMOpenGLIO::deformed_mesh_to_dx_mesh(dx_mesh_, mesh);
			}
			pre_local_transform_ = mesh->local_transform();
		}
		return true;
	}
	return false;
}

/**
 * draw
 */
void UMDirectX11GUIBoard::draw(ID3D11Device* device_pointer, umdraw::UMDirectX11DrawParameterPtr parameter) const
{
	if (UMGUIObjectPtr board = board_.lock())
	{
		if (board->is_visible())
		{
			if (dx_mesh_)
			{
				dx_mesh_->draw(device_pointer, parameter);
			}
		}
	}
}

/**
 * update event
 */
void UMDirectX11GUIBoard::update(umbase::UMEventType event_type, umbase::UMAny& parameter)
{
	if (event_type == eGUIEventObjectUpdated)
	{
		update();
	}
}

} // umgui


#endif // WITH_DIRECTX
