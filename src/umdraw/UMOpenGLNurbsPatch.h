/**
 * @file UMOpenGLNurbsPatch.h
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <memory>
#include <string>
#include "UMMacro.h"

namespace umdraw
{

class UMOpenGLNurbsPatch;
typedef std::shared_ptr<UMOpenGLNurbsPatch> UMOpenGLNurbsPatchPtr;

/**
 * a shader
 */
class UMOpenGLNurbsPatch
{
	DISALLOW_COPY_AND_ASSIGN(UMOpenGLNurbsPatch);

public:

	UMOpenGLNurbsPatch();

	~UMOpenGLNurbsPatch();
	
	/** 
	 * initialize
	 */
	bool init() { return false; }
	
	/** 
	 * update
	 */
	bool update() { return false; }

	/**
	 * draw
	 */
	void draw() {}

private:
};

} // umdraw
