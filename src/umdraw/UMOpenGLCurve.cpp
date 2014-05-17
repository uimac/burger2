/**
 * @file UMOpenGLCurve.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_OPENGL

#include "UMOpenGLCurve.h"

#include <iterator>
#include <GL/glew.h>

#include "UMMacro.h"
#include "UMPath.h"
#include "UMStringUtil.h"

namespace umdraw
{

/// constructor
UMOpenGLCurve::UMOpenGLCurve()
{}

/// destructor
UMOpenGLCurve::~UMOpenGLCurve()
{
}

} // umdraw

#endif // WITH_OPENGL
