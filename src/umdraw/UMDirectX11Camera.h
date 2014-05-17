/**
 * @file UMDirectX11Camera.h
 * a camera
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <dxgi.h>
#include <d3d11.h>
#include <vector>

#include "UMMacro.h"
#include "UMMathTypes.h"
#include "UMMath.h"
#include "UMCamera.h"

namespace umdraw
{

class UMDirectX11Camera;
typedef std::shared_ptr<UMDirectX11Camera> UMDirectX11CameraPtr;
typedef std::vector<UMDirectX11CameraPtr> UMDirectX11CameraList;

/**
 * camera
 */
class UMDirectX11Camera
{
	DISALLOW_COPY_AND_ASSIGN(UMDirectX11Camera);

public:

	/**
	 * constructor.
	 */
	explicit UMDirectX11Camera(UMCameraPtr umcamera) :
		umcamera_(umcamera)
	{}

	/**
	 * destructor
	 */
	~UMDirectX11Camera();

	/** 
	 * initialize
	 */
	bool init(ID3D11Device *device_pointer);
	
	/**
	 * update
	 */
	bool update(ID3D11Device* device_pointer);

	/**
	 * draw
	 */
	void draw(ID3D11Device* device_pointer);
	
	/**
	 * get view projection matrix by float
	 */
	umbase::UMMat44f view_projection_matrix() const;

	/**
	 * get view matrix by float
	 */
	umbase::UMMat44f view_matrix() const;

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
	UMMat44f view_projection_transposed_;
	UMMat44f view_transposed_;
	UMVec4f position_;
	
	UMCameraWeakPtr umcamera_;
};

} // umdraw
