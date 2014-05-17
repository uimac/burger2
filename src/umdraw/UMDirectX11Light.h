/**
 * @file UMDirectX11Light.h
 * a textured rectangle
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
#include "UMVector.h"
#include "UMLight.h"

namespace umdraw
{

class UMDirectX11Light;
typedef std::shared_ptr<UMDirectX11Light> UMDirectX11LightPtr;
typedef std::vector<UMDirectX11LightPtr> UMDirectX11LightList;

/**
 * light
 */
class UMDirectX11Light
{
	DISALLOW_COPY_AND_ASSIGN(UMDirectX11Light);

public:
	
	/**
	 * constructor.
	 */
	explicit UMDirectX11Light(UMLightPtr umlight) :
		umlight_(umlight)
	{}

	/**
	 * destructor
	 */
	~UMDirectX11Light();

	/** 
	 * initialize
	 */
	bool init(ID3D11Device *device_pointer);
	
	/** 
	 * update
	 */
	bool update(ID3D11Device *device_pointer);

	/**
	 * draw light
	 */
	void draw(ID3D11Device* device_pointer);
	
	/**
	 * set umlight
	 */
	void set_umlight(UMLightPtr umlight) { umlight_ = umlight; }

	/**
	 * get umlight
	 */
	UMLightPtr umlight() { return umlight_.lock(); }

private:
	UMVec4f position_;
	UMVec4f color_;
	UMVec4f ambient_color_;
	
	UMLightWeakPtr umlight_;
};

} // umdraw
