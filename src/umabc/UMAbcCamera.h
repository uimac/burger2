/**
 * @file UMAbcCamera.h
 * any
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <memory>
#include <Alembic/Abc/All.h>
#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreFactory/All.h>

#include "UMMacro.h"
#include "UMAbcObject.h"

namespace umdraw
{
class UMCamera;
typedef std::shared_ptr<UMCamera> UMCameraPtr;

class UMOpenGLCamera;
typedef std::shared_ptr<UMOpenGLCamera> UMOpenGLCameraPtr;

class UMDirectX11Camera;
typedef std::shared_ptr<UMDirectX11Camera> UMDirectX11CameraPtr;
} // umdraw

namespace umabc
{
	
class UMAbcCamera;
typedef std::shared_ptr<UMAbcCamera> UMAbcCameraPtr;
typedef std::weak_ptr<UMAbcCamera> UMAbcCameraWeakPtr;

class UMAbcCamera : public UMAbcObject
{
	DISALLOW_COPY_AND_ASSIGN(UMAbcCamera);
public:

	/**
	 * crate instance
	 */
	static UMAbcCameraPtr create(Alembic::AbcGeom::ICamera camera);

	~UMAbcCamera();

	/**
	 * initialize
	 * @param [in] recursive do children recursively
	 * @retval succsess or fail
	 */
	virtual bool init(bool recursive);
	
	/**
	 * set current time
	 * @param [in] time time
	 * @param [in] recursive do children recursively
	 */
	virtual void set_current_time(unsigned long time, bool recursive);
	
	/**
	 * update box
	 * @param [in] recursive do children recursively
	 */
	virtual void update_box(bool recursive);

	/**
	 * draw
	 * @param [in] recursive do children recursively
	 */
	virtual void draw(bool recursive, UMAbc::DrawType type);

	/**
	 * get umcamera
	 */
	umdraw::UMCameraPtr umcamera() const { return umcamera_; }
	
protected:
	UMAbcCamera(Alembic::AbcGeom::ICamera camera);
	
	virtual UMAbcObjectPtr self_reference()
	{
		return self_reference_.lock();
	}
private:
	Alembic::AbcGeom::ICamera camera_;
	Alembic::AbcGeom::CameraSample sample_;
	UMAbcCameraWeakPtr self_reference_;

	umdraw::UMCameraPtr umcamera_;
	umdraw::UMOpenGLCameraPtr opengl_camera_;
	umdraw::UMDirectX11CameraPtr directx_camera_;
};

}
