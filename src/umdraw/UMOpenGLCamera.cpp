/**
 * @file UMOpenGLCamera.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_OPENGL

#include "UMOpenGLCamera.h"
#include "UMCamera.h"

namespace umdraw
{

UMOpenGLCamera::~UMOpenGLCamera()
{
}

/** 
 * initialize
 */
bool UMOpenGLCamera::init()
{
	UMCameraPtr camera = umcamera();
	if (!camera) return false;

	return true;
}

/** 
 * update
 */
bool UMOpenGLCamera::update()
{
	UMCameraPtr camera = umcamera();
	if (!camera) return false;
	
	camera->update();

	//view_projection_transposed_ = view_projection_matrix().transposed();
	//view_transposed_ = view_matrix().transposed();

	const UMVec3d& pos = camera->position();
	position_ = UMVec4f((float)pos.x, (float)pos.y, (float)pos.z, 0.0f);
	return true;
}

/**
 * draw
 */
void UMOpenGLCamera::draw()
{
}

/**
 * get view projection matrix
 */
UMMat44f UMOpenGLCamera::view_projection_matrix() const
{
	const UMMat44d& vp = umcamera()->view_projection_matrix();
	UMMat44f dst;
	for (int i = 0; i < 4; ++i) {
		for (int k = 0; k < 4; ++k) {
			dst.m[i][k] = static_cast<float>(vp.m[i][k]);
		}
	}
	return dst;
}

/**
 * get view matrix
 */
UMMat44f UMOpenGLCamera::view_matrix() const
{
	const UMMat44d& vp = umcamera()->view_matrix();
	UMMat44f dst;
	for (int i = 0; i < 4; ++i) {
		for (int k = 0; k < 4; ++k) {
			dst.m[i][k] = static_cast<float>(vp.m[i][k]);
		}
	}
	return dst;
}

} // umdraw

#endif // WITH_OPENGL
