/**
 * @file UMOpenGL.h
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include "UMDraw.h"
#include "UMScene.h"

namespace umdraw
{

class UMOpenGL;
typedef std::shared_ptr<UMOpenGL> UMOpenGLPtr;

class UMOpenGLImpl;
typedef std::shared_ptr<UMOpenGLImpl> UMOpenGLImplPtr;

/**
 * opengl drawer 
 */
class UMOpenGL : public UMDraw
{
	DISALLOW_COPY_AND_ASSIGN(UMOpenGL);

public:
	static UMOpenGLPtr create();

	virtual ~UMOpenGL();

	/**
	 * initialize
	 */
	virtual bool init(void* hwnd, UMScenePtr scene);
	
	/**
	 * prep for next drawing
	 */
	virtual bool update();

	/**
	 * draw frame
	 */
	virtual bool draw();
	
	/**
	 * create view/depth
	 */
	virtual bool clear();
	
	/**
	 * resize
	 */
	virtual void resize(int width, int height);

	/**
	 * get umdraw scene
	 */
	virtual UMScenePtr scene() const;

private:
	UMOpenGL();

	UMOpenGLImplPtr impl_;
};

} // umdraw
