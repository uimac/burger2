/**
 * @file UMOpenGLCamera.h
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <vector>

#include "UMMacro.h"
#include "UMMathTypes.h"
#include "UMMath.h"

namespace umdraw
{
	
class UMOpenGLCamera;
typedef std::shared_ptr<UMOpenGLCamera> UMOpenGLCameraPtr;
typedef std::weak_ptr<UMOpenGLCamera> UMOpenGLCameraWeakPtr;
typedef std::vector<UMOpenGLCameraPtr> UMOpenGLCameraList;

class UMCamera;
typedef std::shared_ptr<UMCamera> UMCameraPtr;
typedef std::weak_ptr<UMCamera> UMCameraWeakPtr;

/**
 * a shader
 */
class UMOpenGLCamera
{
	DISALLOW_COPY_AND_ASSIGN(UMOpenGLCamera);

public:
	/**
	 * constructor.
	 */
	explicit UMOpenGLCamera(UMCameraPtr umcamera) :
		umcamera_(umcamera)
	{}

	/**
	 * destructor
	 */
	~UMOpenGLCamera();

	/** 
	 * initialize
	 */
	bool init();
	
	/** 
	 * update
	 */
	bool update();

	/**
	 * draw
	 */
	void draw();
	
	/**
	 * get view projection matrix by float
	 */
	UMMat44f view_projection_matrix() const;

	/**
	 * get view matrix by float
	 */
	UMMat44f view_matrix() const;

	/**
	 * set umcamera
	 */
	void set_umcamera(UMCameraPtr umcamera) { umcamera_ = umcamera; }

	/**
	 * get umlight
	 */
	UMCameraPtr umcamera() { return umcamera_.lock(); }
	
	/**
	 * get umlight
	 */
	UMCameraPtr umcamera() const { return umcamera_.lock(); }

private:
	UMVec4f position_;
	
	UMCameraWeakPtr umcamera_;
};

} // umdraw
