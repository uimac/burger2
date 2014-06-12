/**
 * @file UMDirectX11Material.h
 * a material
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
#include <memory>
#include <vector>

#include "UMMacro.h"
#include "UMMathTypes.h"
#include "UMMath.h"
#include "UMMaterial.h"
#include "UMDirectX11Texture.h"

namespace umdraw
{

class UMDirectX11Material;
typedef std::shared_ptr<UMDirectX11Material> UMDirectX11MaterialPtr;
typedef std::vector<UMDirectX11MaterialPtr> UMDirectX11MaterialList;

/**
 * material
 */
class UMDirectX11Material
{
	DISALLOW_COPY_AND_ASSIGN(UMDirectX11Material);

public:
	typedef std::vector<umstring> TexturePathList;
	
	/**
	 * constructor.
	 */
	explicit UMDirectX11Material(UMMaterialPtr ummaterial) :
		ummaterial_(ummaterial),
		polygon_count_(0),
		diffuse_texture_(std::make_shared<UMDirectX11Texture>(false))
	{}

	/**
	 * destructor
	 */
	~UMDirectX11Material();

	static UMDirectX11MaterialPtr default_material() { 
		UMDirectX11MaterialPtr material(std::make_shared<UMDirectX11Material>(UMMaterialPtr()));
		material->set_diffuse(UMVec4f(0.0f, 0.7f, 0.7f, 1.0f));
		material->set_specular(UMVec4f(0.9f, 0.9f, 0.9f, 1.0f));
		material->set_ambient(UMVec4f(0.3f, 0.3f, 0.3f, 1.0f));
		return material;
	}

	/** 
	 * initialize
	 */
	bool init(ID3D11Device *device_pointer);
	
	/** 
	 * update
	 */
	bool update(ID3D11Device *device_pointer);
	
	/**
	 * draw
	 */
	void draw(ID3D11Device* device_pointer);

	/**
	 * get diffuse
	 */
	const UMVec4f& diffuse() const { return diffuse_; }
	
	/**
	 * set diffuse
	 */
	void set_diffuse(const UMVec4f& diffuse) { diffuse_ = diffuse; }

	/**
	 * get specular
	 */
	const UMVec4f& specular() const { return specular_; }
	
	/**
	 * set ambient
	 */
	void set_specular(const UMVec4f& specular) { specular_ = specular; }

	/**
	 * get ambient
	 */
	const UMVec4f& ambient() const { return ambient_; }

	/**
	 * set ambient
	 */
	void set_ambient(const UMVec4f& ambient) { ambient_ = ambient; }

	/**
	 * get shader flags
	 * @note x is uvflag, yzw not defined now
	 */
	const UMVec4f shader_flags() const { return shader_flags_; }

	/** 
	 * set shader flags
	 * @note x is uvflag, yzw not defined now
	 */
	void set_shader_flags(const UMVec4f& flags) { shader_flags_ = flags; }

	/**
	 * get diffuse texture
	 */
	UMDirectX11TexturePtr diffuse_texture() const { return diffuse_texture_; }

	/**
	 * get polygon count
	 */
	int polygon_count() const;
	
	/**
	 * set ummaterial
	 */
	void set_ummaterial(UMMaterialPtr ummaterial) { ummaterial_ = ummaterial; }

	/**
	 * get ummaterial
	 */
	UMMaterialPtr ummaterial() { return ummaterial_.lock(); }
	
	/**
	 * get ummaterial
	 */
	UMMaterialPtr ummaterial() const { return ummaterial_.lock(); }

private:
	UMVec4f diffuse_;
	// .w is specular_factor;
	UMVec4f specular_;
	UMVec4f ambient_;
	UMVec4f shader_flags_;

	UMDirectX11TexturePtr diffuse_texture_;
	
	int polygon_count_;
	UMMaterialWeakPtr ummaterial_;
};

} // umdraw
