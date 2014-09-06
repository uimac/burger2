/**
 * @file UMToonRender.h
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

class UMToonRender;
typedef std::shared_ptr<UMToonRender> UMToonRenderPtr;

class UMScene;
class UMRenderParameter;

/**
 * a raytracer
 */
class UMToonRender : public UMRenderer
{
	DISALLOW_COPY_AND_ASSIGN(UMToonRender);
public:
	UMToonRender();

	~UMToonRender();
	
	/**
	 * initialize
	 * @note needs a context
	 */
	virtual bool init();

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
	 * OpenShadingLanguage render service
	 */
	virtual OSL::RendererServices* render_service();

	/**
	 * progressive render
	 * @param [in] scene_access target scene access
	 * @param [in,out] parameter parameters for rendering
	 * @retval true still render
	 * @retval false render finished or failed
	 */
	virtual bool progress_render(UMSceneAccessPtr scene_access, UMRenderParameter& parameter);

	/** 
	 * set client width
	 */
	virtual void set_width(int width);

	/** 
	 * set client height
	 */
	virtual void set_height(int height);


private:
	class Impl;
	typedef std::unique_ptr<Impl> ImplPtr;
	ImplPtr impl_;
};

} // umrt
