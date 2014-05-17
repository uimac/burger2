/**
 * @file UMOpenGLLight.h
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <vector>
#include <memory>
#include "UMMacro.h"
#include "UMMathTypes.h"
#include "UMVector.h"

namespace umdraw
{
	
class UMLight;
typedef std::shared_ptr<UMLight> UMLightPtr;
typedef std::weak_ptr<UMLight> UMLightWeakPtr;

class UMOpenGLLight;
typedef std::shared_ptr<UMOpenGLLight> UMOpenGLLightPtr;
typedef std::weak_ptr<UMOpenGLLight> UMOpenGLLightWeakPtr;
typedef std::vector<UMOpenGLLightPtr> UMOpenGLLightList;

/**
 * opengl light
 */
class UMOpenGLLight
{
	DISALLOW_COPY_AND_ASSIGN(UMOpenGLLight);

public:
	
	/**
	 * constructor.
	 */
	explicit UMOpenGLLight(UMLightPtr umlight) :
		umlight_(umlight)
	{}

	/**
	 * destructor
	 */
	~UMOpenGLLight();

	/** 
	 * initialize
	 */
	bool init();
	
	/** 
	 * update
	 */
	bool update();

	/**
	 * draw light
	 */
	void draw();

	/**
	 * get position
	 */
	const UMVec4f& position() const { return position_; }
	
	/**
	 * get color
	 */
	const UMVec4f& color() const { return color_; }
	
	/**
	 * get ambient_color
	 */
	const UMVec4f& ambient_color() const { return ambient_color_; }

	/**
	 * set umlight
	 */
	void set_umlight(UMLightPtr umlight) { umlight_ = umlight; }

	/**
	 * get umlight
	 */
	UMLightPtr umlight() { return umlight_.lock(); }

private:
	UMVec4f position_;
	UMVec4f color_;
	UMVec4f ambient_color_;
	
	UMLightWeakPtr umlight_;
};

} // umdraw
