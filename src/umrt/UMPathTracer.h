/**
 * @file UMPathTracer.h
 * a pathtracer
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
#include "UMMacro.h"
#include "UMRenderer.h"
#include "UMVector.h"
#include "UMRay.h"
#include "UMShaderParameter.h"
//#include "UMSceneAccess.h"
#include "UMImage.h"
//#include "UMEvent.h"

namespace umrt
{

class UMPathTracer;
typedef std::shared_ptr<UMPathTracer> UMPathTracerPtr;

class UMSceneAccess;
typedef std::shared_ptr<UMSceneAccess> UMSceneAccessPtr;

class UMScene;
class UMRenderParameter;
class UMIntersection;

/**
 * a pathtracer
 */
class UMPathTracer : public UMRenderer
{
	DISALLOW_COPY_AND_ASSIGN(UMPathTracer);
public:
	UMPathTracer();

	~UMPathTracer() {}
	
	/**
	 * initialize
	 * @note needs a context
	 */
	virtual bool init() {
		current_sample_count_ = 0;
		return true;
	}

	/**
	 * get renderer type
	 */
	virtual RendererType type() const { return ePathTracer; }
	
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
	/**
	 * trace
	 */
	UMVec3d trace(
		const UMRay& ray, 
		UMSceneAccessPtr scene_access, 
		UMShaderParameter& parameter, 
		std::mt19937& mt);

	/**
	 * direct lighting
	 */
	UMVec3d illuminate_direct(
		const UMRay& ray, 
		UMSceneAccessPtr scene_access, 
		const UMIntersection& intersection, 
		UMShaderParameter& parameter,
		std::mt19937& mt);

	/**
	 * indirect lighting
	 */
	UMVec3d illuminate_indirect(
		const UMRay& ray, 
		UMSceneAccessPtr scene_access, 
		const UMIntersection& intersection, 
		UMShaderParameter& parameter, 
		std::mt19937& mt);
	

	// for progress render
	int current_sample_count_;
	int current_subpixel_x_;
	int current_subpixel_y_;
	int max_sample_count_;
	//UMRandomSampler sampler_;
	UMImage temporary_image_;
	//UMEventPtr sample_event_;
};

} // umrt
