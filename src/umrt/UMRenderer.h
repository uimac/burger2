/**
 * @file UMRenderer.h
 * an interface of renderer
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <memory>
#include "UMMacro.h"
//#include "UMEvent.h"
//#include "UMListenerConnector.h"

namespace OSL
{
	class RendererServices;
} // OSL

namespace umrt
{

class UMRenderer;
typedef std::shared_ptr<UMRenderer> UMRendererPtr;

class UMSceneAccess;
typedef std::shared_ptr<UMSceneAccess> UMSceneAccessPtr;

class UMRenderParameter;

/**
 * an interface of renderer
 */
class UMRenderer //: public UMListenerConnector
{
	DISALLOW_COPY_AND_ASSIGN(UMRenderer);
public:
	/**
	 * renderer types
	 */
	enum RendererType {
		eSimpleRayTracer,
		ePathTracer,
		eToonRender,
	};
	
	UMRenderer() : 
		width_(0), 
		height_(0) {}
	
	virtual ~UMRenderer() {}

	/**
	 * create renderer
	 */
	static UMRendererPtr create(RendererType type);
	
	/**
	 * initialize
	 * @note needs a context
	 */
	virtual bool init() = 0;

	/**
	 * get renderer type
	 */
	virtual RendererType type() const = 0;
	
	/**
	 * render
	 * @param [in] scene target scene
	 * @param [in,out] parameter parameters for rendering
	 * @retval success or failed
	 */
	virtual bool render(UMSceneAccessPtr scene, UMRenderParameter& parameter) = 0;
	
	/**
	 * progressive render
	 * @param [in] scene target scene
	 * @param [in,out] parameter parameters for rendering
	 * @retval true still render
	 * @retval false render finished or failed
	 */
	virtual bool progress_render(UMSceneAccessPtr scene, UMRenderParameter& parameter){ return false; }

	/**
	 * OpenShadingLanguage render service
	 */
	virtual OSL::RendererServices* render_service() { return NULL; }

	/** 
	 * set client width
	 */
	virtual void set_width(int width) { width_ = width; }

	/** 
	 * set client height
	 */
	virtual void set_height(int height) { height_ = height; }

	/** 
	 * get client width
	 */
	virtual int width() const { return width_; }

	/** 
	 * get client height
	 */
	virtual int height() const { return height_; }

protected:
	int width_;
	int height_;
};

} // umrt
