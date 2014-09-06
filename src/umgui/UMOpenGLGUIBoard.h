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
#include "UMListener.h"
#include "UMGUIObject.h"

namespace umdraw
{
	class UMOpenGLMesh;
	typedef std::shared_ptr<UMOpenGLMesh> UMOpenGLMeshPtr;
	
	class UMOpenGLDrawParameter;
	typedef std::shared_ptr<UMOpenGLDrawParameter> UMOpenGLDrawParameterPtr;

} // umdraw

namespace umgui
{

class UMOpenGLGUIBoard;
typedef std::shared_ptr<UMOpenGLGUIBoard> UMOpenGLGUIBoardPtr;
typedef std::vector<UMOpenGLGUIBoardPtr> UMOpenGLGUIBoardList;

/**
 * a board
 */
class UMOpenGLGUIBoard : public umbase::UMListener
{
	DISALLOW_COPY_AND_ASSIGN(UMOpenGLGUIBoard);
public:
	UMOpenGLGUIBoard() {}
	UMOpenGLGUIBoard(UMGUIObjectPtr board);

	~UMOpenGLGUIBoard() {}

	/** 
	 * initialize
	 */
	bool init();

	/**
	 * update
	 */
	bool update();

	/**
	 * draw
	 */
	void draw(umdraw::UMOpenGLDrawParameterPtr parameter) const;
	
	/**
	 * update event
	 */
	virtual void update(umbase::UMEventType event_type, umbase::UMAny& parameter);

private:
	UMGUIObjectWeakPtr board_;
	umdraw::UMOpenGLMeshPtr gl_mesh_;
	umbase::UMMat44d pre_local_transform_;
};

} // umgui
