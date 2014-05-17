/**
 * @file UMDirectX11GUIBoard.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */

#include "UMDirectX11GUIBoard.h"
#include "UMGUIBoard.h"
#include "UMDirectX11IO.h"
#include "UMDirectX11Mesh.h"
#include "UMMesh.h"

namespace umgui
{

/**
 * constructor
 */
UMDirectX11GUIBoard::UMDirectX11GUIBoard(UMGUIBoardPtr board)
	: board_(board)
{
}

/**
 * initialize
 */
bool UMDirectX11GUIBoard::init(ID3D11Device* device_pointer)
{
	if (UMGUIBoardPtr board = board_.lock())
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
 * draw
 */
void UMDirectX11GUIBoard::draw(ID3D11Device* device_pointer, umdraw::UMDirectX11DrawParameterPtr parameter) const
{
	if (dx_mesh_)
	{
		dx_mesh_->draw(device_pointer, parameter);
	}
}

} // umgui
