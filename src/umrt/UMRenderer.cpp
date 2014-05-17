/**
 * @file UMRenderer.cpp
 * an interface of renderer
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include "UMRenderer.h"
#include "UMRayTracer.h"
#include "UMPathTracer.h"

namespace umrt
{

/**
 * create renderer
 */
UMRendererPtr UMRenderer::create(RendererType type)
{
	if (type == eSimpleRayTracer)
	{
		return std::make_shared<UMRayTracer>();
	}
	else if (type == ePathTracer)
	{
		return std::make_shared<UMPathTracer>();
	}
	return UMRendererPtr();
}

} // umrt
