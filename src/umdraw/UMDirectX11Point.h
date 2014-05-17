/**
 * @file UMDirectX11Point.h
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <memory>
#include <dxgi.h>
#include <d3d11.h>
#include <string>
#include "UMMacro.h"

namespace umdraw
{

class UMDirectX11Point;
typedef std::shared_ptr<UMDirectX11Point> UMDirectX11PointPtr;

/**
 * a shader
 */
class UMDirectX11Point 
{
	DISALLOW_COPY_AND_ASSIGN(UMDirectX11Point);
public:

	UMDirectX11Point();

	~UMDirectX11Point();
	
	/** 
	 * initialize
	 */
	bool init(ID3D11Device *device_pointer) { return false; }
	
	/**
	 * update
	 */
	bool update(ID3D11Device* device_pointer) { return false; }

	/**
	 * draw
	 */
	void draw(ID3D11Device* device_pointer) {}

private:
};

} // umdraw
