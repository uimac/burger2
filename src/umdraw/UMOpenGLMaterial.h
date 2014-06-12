/**
 * @file UMOpenGLMaterial.h
 * a material
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <memory>
#include <vector>
#include <string>
#include <map>
#include "UMMacro.h"
#include "UMMath.h"
#include "UMMathTypes.h"

namespace umdraw
{

class UMOpenGLMaterial;
typedef std::shared_ptr<UMOpenGLMaterial> UMOpenGLMaterialPtr;
typedef std::map<umstring, UMOpenGLMaterialPtr> UMOpenGLMaterialMap;
typedef std::vector<UMOpenGLMaterialPtr> UMOpenGLMaterialList;

class UMMaterial;
typedef std::shared_ptr<UMMaterial> UMMaterialPtr;
typedef std::weak_ptr<UMMaterial> UMMaterialWeakPtr;

class UMOpenGLTexture;
typedef std::shared_ptr<UMOpenGLTexture> UMOpenGLTexturePtr;

/**
 * opengl material
 */
class UMOpenGLMaterial
{
	DISALLOW_COPY_AND_ASSIGN(UMOpenGLMaterial);

public:
	typedef std::vector<umstring> TexturePathList;
	
	/**
	 * constructor.
	 */
	explicit UMOpenGLMaterial(UMMaterialPtr ummaterial);
	
	/**
	 * destructor
	 */
	~UMOpenGLMaterial();

	static UMOpenGLMaterialPtr default_material();

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
	 * get diffuse
	 */
	const UMVec4f& diffuse() const;
	
	/**
	 * set diffuse
	 */
	void set_diffuse(const UMVec4f& diffuse);

	/**
	 * get specular
	 */
	const UMVec4f& specular() const;
	
	/**
	 * set specular
	 */
	void set_specular(const UMVec4f& specular);

	/**
	 * get ambient
	 */
	const UMVec4f& ambient() const;

	/**
	 * set ambient
	 */
	void set_ambient(const UMVec4f& ambient);

	/**
	 * get shader flags
	 * @note x is uvflag, yzw not defined now
	 */
	const UMVec4f& shader_flags() const;

	/** 
	 * set shader flags
	 * @note x is uvflag, yzw not defined now
	 */
	void set_shader_flags(const UMVec4f& flags);

	/**
	 * get diffuse texture
	 */
	UMOpenGLTexturePtr diffuse_texture() const;

	/**
	 * get polygon count
	 */
	int polygon_count() const;

	/**
	 * set polygon count
	 */
	void set_polygon_count(int count);

	/**
	 * set ummaterial
	 */
	void set_ummaterial(UMMaterialPtr ummaterial);

	/**
	 * get ummaterial
	 */
	UMMaterialPtr ummaterial();
	
	/**
	 * get ummaterial
	 */
	UMMaterialPtr ummaterial() const;

	/**
	 * set name
	 */
	void set_name(const umstring& name);

	/**
	 * get name
	 */
	const umstring& name();
	
private:
	class Impl;
	typedef std::unique_ptr<Impl> ImplPtr;
	ImplPtr impl_;
};

} // umdraw
