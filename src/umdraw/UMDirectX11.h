/**
 * @file UMDirectX11.h
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include "UMDraw.h"
#include "UMScene.h"


namespace umdraw
{
	
typedef void* UMDirectXDevice;

class UMDirectX11;
typedef std::shared_ptr<UMDirectX11> UMDirectX11Ptr;

class UMDirectX11Impl;
typedef std::shared_ptr<UMDirectX11Impl> UMDirectX11ImplPtr;


/**
 * directx 11 viewer
 */
class UMDirectX11 : public UMDraw
{
	DISALLOW_COPY_AND_ASSIGN(UMDirectX11);

public:
	static UMDirectX11Ptr create();

	virtual ~UMDirectX11();

	/**
	 * initialize
	 */
	virtual bool init(void* hWnd, umdraw::UMScenePtr scene);
	
	/**
	 * draw frame
	 */
	virtual bool update();

	/**
	 * draw frame
	 */
	virtual bool draw();

	/**
	 * create view/depth
	 */
	virtual bool clear();
	
	/**
	 * resize
	 */
	virtual void resize(int width, int height);

	/**
	 * get umdraw scene
	 */
	virtual UMScenePtr scene() const;

	/**
	 * get current device context
	 */
	static UMDirectXDevice current_device_pointer() { return UMDirectX11::dx_device_; }

private:
	UMDirectX11();

	UMDirectX11ImplPtr impl_;

	static UMDirectXDevice dx_device_;
};

} // umdraw
