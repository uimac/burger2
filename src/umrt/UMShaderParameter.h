/**
 * @file UMShaderParameter.h
 * shading parameters
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include "UMMacro.h"
#include "UMMaterial.h"

namespace umrt
{

/**
 * shading parameters
 */
class UMShaderParameter
{
public:
	UMShaderParameter() : emissive(0), bounce(-1), depth(32), max_depth(32) {}
	~UMShaderParameter() {}
	
	/**
	 * material
	 */
	umdraw::UMMaterialPtr material;

	/**
	 * color
	 */
	UMVec3d color;

	/**
	 * emissive
	 */
	UMVec3d emissive;

	/**
	 * distance
	 */
	double distance;

	/**
	 * normal
	 */
	UMVec3d normal;

	/**
	 * intersect point
	 */
	UMVec3d intersect_point;

	/**
	 * triangle bycentic parameter
	 */
	UMVec3d uvw;

	/**
	 * uv coordinate
	 */
	UMVec2d uv;

	/**
	 * bounce
	 */
	int bounce;
	
	/**
	 * max depth
	 */
	int depth;

	/**
	 * max depth
	 */
	int max_depth;

	/**
	 * face index
	 */
	int face_index;
};

} // umrt
