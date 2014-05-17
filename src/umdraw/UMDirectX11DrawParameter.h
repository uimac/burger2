/**
 * @file UMDirectX11DrawParameter.h
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

struct ID3D11SamplerState;

namespace umdraw
{

class UMDirectX11DrawParameter;
typedef std::shared_ptr<UMDirectX11DrawParameter> UMDirectX11DrawParameterPtr;

class UMDirectX11ShaderManager;
typedef std::shared_ptr<UMDirectX11ShaderManager> UMDirectX11ShaderManagerPtr;

class UMDirectX11Light;
typedef std::shared_ptr<UMDirectX11Light> UMDirectX11LightPtr;

class UMDirectX11Camera;
typedef std::shared_ptr<UMDirectX11Camera> UMDirectX11CameraPtr;

/**
 * draw parameter
 */
class UMDirectX11DrawParameter
{
	DISALLOW_COPY_AND_ASSIGN(UMDirectX11DrawParameter);

public:
	UMDirectX11DrawParameter()
		: default_sampler_state_(NULL) {}

	/// get shader manager
	UMDirectX11ShaderManagerPtr shader_manager() { return shader_manager_; }

	/// set shader manager
	void set_shader_manager(UMDirectX11ShaderManagerPtr shader_manager) { shader_manager_ = shader_manager; }
	
	/// get light
	UMDirectX11LightPtr light() { return light_; }

	/// set light
	void set_light(UMDirectX11LightPtr light) { light_ = light; }
	
	/// get camera
	UMDirectX11CameraPtr camera() { return camera_; }

	/// set camera
	void set_camera(UMDirectX11CameraPtr camera) { camera_ = camera; }

	ID3D11SamplerState * default_sampler_state() { return default_sampler_state_; }
	void set_default_sampler_state(ID3D11SamplerState * state) { default_sampler_state_ = state; }

private:
	ID3D11SamplerState* default_sampler_state_;
	UMDirectX11ShaderManagerPtr shader_manager_;
	UMDirectX11LightPtr light_;
	UMDirectX11CameraPtr camera_;
};

} // umdraw
