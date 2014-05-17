/**
 * @file UMOpenGLNurbsPatch.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_OPENGL

#include "UMOpenGLNurbsPatch.h"

#include <iterator>
#include <GL/glew.h>

#include "UMMacro.h"
#include "UMPath.h"
#include "UMStringUtil.h"

namespace umdraw
{

/// constructor
UMOpenGLNurbsPatch::UMOpenGLNurbsPatch()
{}

/// destructor
UMOpenGLNurbsPatch::~UMOpenGLNurbsPatch()
{
}

} // umdraw

#endif // WITH_OPENGL
