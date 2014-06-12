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

namespace umgui
{
	
/**
 * constructor
 */
UMOpenGLGUIBoard::UMOpenGLGUIBoard(UMGUIBoardPtr board)
	: board_(board)
{

}

/** 
 * initialize
 */
bool UMOpenGLGUIBoard::init()
{
	if (UMGUIBoardPtr board = board_.lock())
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
 * draw
 */
void UMOpenGLGUIBoard::draw(umdraw::UMOpenGLDrawParameterPtr parameter) const
{
	if (gl_mesh_)
	{
		gl_mesh_->draw(parameter);
	}
}

} // umgui
