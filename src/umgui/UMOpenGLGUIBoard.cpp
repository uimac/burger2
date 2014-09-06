/**
 * @file UMOpenGLGUIBoard.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */

#include "UMOpenGLGUIBoard.h"
#include "UMGUIBoard.h"
#include "UMOpenGLIO.h"
#include "UMOpenGLMesh.h"
#include "UMGUIEventType.h"

namespace umgui
{
	
/**
 * constructor
 */
UMOpenGLGUIBoard::UMOpenGLGUIBoard(UMGUIObjectPtr board)
	: board_(board)
{

}

/** 
 * initialize
 */
bool UMOpenGLGUIBoard::init()
{
	if (UMGUIObjectPtr board = board_.lock())
	{
		if (board->is_root()) return true;
		gl_mesh_ = umdraw::UMOpenGLIO::convert_mesh_to_gl_mesh(board->mesh());
		if (gl_mesh_->init())
		{
			return true;
		}
	}
	return false;
}

/**
 * update
 */
bool UMOpenGLGUIBoard::update()
{
	if (UMGUIObjectPtr board = board_.lock())
	{
		if (board->is_root()) return true;
		umdraw::UMMeshPtr mesh = board->mesh();
		if (board->local_transform() != pre_local_transform_)
		{
			mesh->mutable_local_transform() = board->local_transform();
			mesh->mutable_global_transform() = board->global_transform();
			mesh->update();
			if (gl_mesh_)
			{
				umdraw::UMOpenGLIO::deformed_mesh_to_gl_mesh(gl_mesh_, mesh);
			}
			pre_local_transform_ = board->local_transform();
		}
		return true;
	}
	return false;
}

/**
 * draw
 */
void UMOpenGLGUIBoard::draw(umdraw::UMOpenGLDrawParameterPtr parameter) const
{
	if (UMGUIObjectPtr board = board_.lock())
	{
		if (board->is_visible())
		{
			if (gl_mesh_)
			{
				gl_mesh_->draw(parameter);
			}
		}
	}
}

/**
 * update event
 */
void UMOpenGLGUIBoard::update(umbase::UMEventType event_type, umbase::UMAny& parameter)
{
	if (event_type == eGUIEventObjectUpdated)
	{
		update();
	}
}

} // umgui
