/**
 * @file UMOpenGLGUIBoard.h
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <vector>
#include <memory>
#include "UMMacro.h"
#include "UMMesh.h"

namespace umdraw
{
	class UMOpenGLMesh;
	typedef std::shared_ptr<UMOpenGLMesh> UMOpenGLMeshPtr;
	
	class UMOpenGLDrawParameter;
	typedef std::shared_ptr<UMOpenGLDrawParameter> UMOpenGLDrawParameterPtr;

} // umdraw

namespace umgui
{
	
class UMGUIBoard;
typedef std::shared_ptr<UMGUIBoard> UMGUIBoardPtr;
typedef std::weak_ptr<UMGUIBoard> UMGUIBoardWeakPtr;

class UMOpenGLGUIBoard;
typedef std::shared_ptr<UMOpenGLGUIBoard> UMOpenGLGUIBoardPtr;
typedef std::vector<UMOpenGLGUIBoardPtr> UMOpenGLGUIBoardList;

/**
 * a board
 */
class UMOpenGLGUIBoard
{
	DISALLOW_COPY_AND_ASSIGN(UMOpenGLGUIBoard);
public:
	UMOpenGLGUIBoard() {}
	UMOpenGLGUIBoard(UMGUIBoardPtr board);

	~UMOpenGLGUIBoard() {}

	/** 
	 * initialize
	 */
	bool init();

	/**
	 * draw
	 */
	void draw(umdraw::UMOpenGLDrawParameterPtr parameter) const;

private:
	UMGUIBoardWeakPtr board_;
	umdraw::UMOpenGLMeshPtr gl_mesh_;
};

} // umgui
