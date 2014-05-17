/**
 * @file UMOpenGLCurve.h
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

class UMOpenGLCurve;
typedef std::shared_ptr<UMOpenGLCurve> UMOpenGLCurvePtr;

/**
 * a shader
 */
class UMOpenGLCurve
{
	DISALLOW_COPY_AND_ASSIGN(UMOpenGLCurve);

public:

	UMOpenGLCurve();

	~UMOpenGLCurve();
	
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
