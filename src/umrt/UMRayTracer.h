/**
 * @file UMRayTracer.h
 * a raytracer
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <memory>
#include <vector>
#include <random>
#include <utility>
#include "UMMacro.h"
#include "UMRenderer.h"
#include "UMVector.h"
#include "UMRay.h"
#include "UMBvh.h"
#include "UMShaderParameter.h"
#include "UMSceneAccess.h"

namespace umrt
{

class UMRayTracer;
typedef std::shared_ptr<UMRayTracer> UMRayTracerPtr;

class UMScene;
class UMRenderParameter;

/**
 * a raytracer
 */
class UMRayTracer : public UMRenderer
{
	DISALLOW_COPY_AND_ASSIGN(UMRayTracer);
public:
	UMRayTracer() : 
		current_x_(0),
		current_y_(0),
		ray_(UMVec3d(0, 0, 500), UMVec3d(0))
	{}

	~UMRayTracer() {}
	
	/**
	 * initialize
	 * @note needs a context
	 */
	virtual bool init() {
		current_x_ = 0;
		current_y_ = 0;
		ray_.set_origin( UMVec3d(0, 0, 500) );
		ray_.set_direction( UMVec3d(0) );
		return true;
	}

	/**
	 * get renderer type
	 */
	virtual RendererType type() const { return eSimpleRayTracer; }
	
	/**
	 * render
	 * @param [in] scene_access target scene access
	 * @param [in,out] parameter parameters for rendering
	 * @retval success or failed
	 */
	virtual bool render(UMSceneAccessPtr scene_access, UMRenderParameter& parameter);
	
	/**
	 * progressive render
	 * @param [in] scene_access target scene access
	 * @param [in,out] parameter parameters for rendering
	 * @retval true still render
	 * @retval false render finished or failed
	 */
	virtual bool progress_render(UMSceneAccessPtr scene_access, UMRenderParameter& parameter);

private:
	// for progress render
	int current_x_;
	int current_y_;
	UMRay ray_;
	UMShaderParameter shader_param_;
	//UMRandomSampler sampler_;
};

} // umrt
