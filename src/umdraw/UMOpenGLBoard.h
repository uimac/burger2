/**
 * @file UMOpenGLBoard.h
 * a textured rectangle
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <vector>
#include "UMMacro.h"
#include "UMMathTypes.h"
#include "UMVector.h"

namespace umdraw
{

class UMOpenGLBoard;
typedef std::shared_ptr<UMOpenGLBoard> UMOpenGLBoardPtr;
typedef std::vector<UMOpenGLBoardPtr> UMOpenGLBoardList;

class UMOpenGLDrawParameter;
typedef std::shared_ptr<UMOpenGLDrawParameter> UMOpenGLDrawParameterPtr;

class UMOpenGLTexture;
typedef std::shared_ptr<UMOpenGLTexture> UMOpenGLTexturePtr;
typedef std::vector<UMOpenGLTexturePtr> UMOpenGLTextureList;

/**
 * a textured rectangle for frame buffer
 */
class UMOpenGLBoard
{
	DISALLOW_COPY_AND_ASSIGN(UMOpenGLBoard);

public:

	/**
	 * constructor. create board on x-y plane
	 * @param [in] left_top left top position
	 * @param [in] right_bottom right bottom position
	 * @param [in] z z position
	 */
	UMOpenGLBoard(
		UMVec2f left_top,
		UMVec2f right_bottom,
		float z);

	/**
	 * destructor
	 */
	~UMOpenGLBoard();

	/** 
	 * initialize
	 */
	bool init();
	
	/**
	 * update
	 */
	bool update();

	/**
	 * draw board
	 */
	void draw(UMOpenGLDrawParameterPtr parameter);

	/**
	 * set texture
	 */
	void set_texture(UMOpenGLTexturePtr texture);

	/**
	 * set color attachments for deferred rendering
	 */
	void set_color_attachments(const UMOpenGLTextureList& attachments);

	/**
	 * get texture
	 */
	UMOpenGLTexturePtr texture() const;

private:
	class Impl;
	typedef std::unique_ptr<Impl> ImplPtr;
	ImplPtr impl_;
};

} // umdraw
