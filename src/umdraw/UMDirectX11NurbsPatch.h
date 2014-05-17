/**
 * @file UMDirectX11NurbsPatch.h
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

class UMDirectX11NurbsPatch;
typedef std::shared_ptr<UMDirectX11NurbsPatch> UMDirectX11NurbsPatchPtr;

/**
 * a shader
 */
class UMDirectX11NurbsPatch 
{
	DISALLOW_COPY_AND_ASSIGN(UMDirectX11NurbsPatch);
public:

	UMDirectX11NurbsPatch();

	~UMDirectX11NurbsPatch();
	
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
