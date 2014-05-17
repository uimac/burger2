/**
 * @file UMOpenGLDrawParameter.h
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

namespace umdraw
{

class UMOpenGLDrawParameter;
typedef std::shared_ptr<UMOpenGLDrawParameter> UMOpenGLDrawParameterPtr;

class UMOpenGLLight;
typedef std::shared_ptr<UMOpenGLLight> UMOpenGLLightPtr;

class UMOpenGLCamera;
typedef std::shared_ptr<UMOpenGLCamera> UMOpenGLCameraPtr;

class UMOpenGLShaderManager;
typedef std::shared_ptr<UMOpenGLShaderManager> UMOpenGLShaderManagerPtr;

/**
 * draw parameter
 */
class UMOpenGLDrawParameter
{
	DISALLOW_COPY_AND_ASSIGN(UMOpenGLDrawParameter);

public:
	UMOpenGLDrawParameter() {}

	/// get camera
	UMOpenGLCameraPtr camera() const { return camera_; }

	/// set camera
	void set_camera(UMOpenGLCameraPtr camera) { camera_ = camera; }
	
	/// get light
	UMOpenGLLightPtr light() const { return light_; }

	/// get resolution
	const UMVec2d& resolution() const { return resolution_; }
	
	/// set light
	void set_light(UMOpenGLLightPtr light) { light_ = light; }
	
	/// get shader manager
	UMOpenGLShaderManagerPtr shader_manager() { return shader_manager_; }

	/// set shader manager
	void set_shader_manager(UMOpenGLShaderManagerPtr shader_manager) { shader_manager_ = shader_manager; }

	void set_resolution(const UMVec2d& resolution) { resolution_ = resolution; }

private:
	UMOpenGLCameraPtr camera_;
	UMOpenGLLightPtr light_;
	UMOpenGLShaderManagerPtr shader_manager_;
	UMVec2d resolution_;
};

} // umdraw
