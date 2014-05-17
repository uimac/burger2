/**
 * @file UMAbcCamera.cpp
 * any
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */

#include "UMAbcCamera.h"
#include "UMOpenGLCamera.h"
#include "UMDirectX11Camera.h"
#include "UMCamera.h"

namespace umabc
{
	using namespace Alembic::Abc;
	using namespace Alembic::AbcGeom;
	
/**
 * create
 */
UMAbcCameraPtr UMAbcCamera::create(Alembic::AbcGeom::ICamera camera)
{
	UMAbcCameraPtr instance = UMAbcCameraPtr(new UMAbcCamera(camera));
	instance->self_reference_ = instance;
	return instance;
}

/**
 * construtor
 */
UMAbcCamera::UMAbcCamera(Alembic::AbcGeom::ICamera camera)
	: UMAbcObject(camera)
	, camera_(camera)
{
	umcamera_ = std::make_shared<umdraw::UMCamera>(false, 1280, 720);
}

/**
 * destructor
 */
UMAbcCamera::~UMAbcCamera()
{
}

/**
 * initialize
 */
bool UMAbcCamera::init(bool recursive)
{
	if (!is_valid()) return false;

	ICameraSchema &schema = camera_.getSchema();

	const size_t num_samples = schema.getNumSamples();
	if (num_samples > 0)
	{
		// get sample
		schema.get(sample_);
		
		// if not consistant, we get time
		if (!schema.isConstant())
		{
			TimeSamplingPtr time = schema.getTimeSampling();
		}
	}

#ifdef WITH_OPENGL
	if (umcamera_)
	{
		opengl_camera_ = std::make_shared<umdraw::UMOpenGLCamera>(umcamera_);
	}
#endif // WITH_OPENGL

#ifdef WITH_DIRECTX
	if (umcamera_)
	{
		directx_camera_ = std::make_shared<umdraw::UMDirectX11Camera>(umcamera_);
	}
#endif // WITH_DIRECTX

	return UMAbcObject::init(recursive);
}

/**
 * set current time
 */
void UMAbcCamera::set_current_time(unsigned long time, bool recursive)
{
	if (!is_valid()) {
		mutable_local_transform().identity();
		return;
	}
	
	if (umcamera_)
	{
		UMAbcObjectPtr p = parent();
		UMMat44d mat = p->local_transform();
		UMMat44d gmat = p->global_transform();

		umcamera_->mutable_global_transform() = mat;
		umcamera_->update_from_node();
		//UMVec3d pos(mat.m[3][0], mat.m[3][1], mat.m[3][2]);
		//printf("l %f, %f, %f\n", pos.x, pos.y, pos.z);
	}
	
	UMAbcObject::set_current_time(time, recursive);
}

/**
 * update box
 */
void UMAbcCamera::update_box(bool recursive)
{
}

/**
 * refresh
 */
void UMAbcCamera::draw(bool recursive, UMAbc::DrawType type)
{
}


}

