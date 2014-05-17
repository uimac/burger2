/**
 * @file UMTga.h
 * TGA IO
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include "UMMacro.h"
#include "UMImage.h"

namespace umimage
{

/**
 * TGA IO
 */
class UMTga
{
	DISALLOW_COPY_AND_ASSIGN(UMTga);
public:
	UMTga() {}
	~UMTga() {}
	
	/**
	 * save tga to a file
	 * @param [in] path save path
	 * @param [in] image output image
	 */
	bool save(const std::string& path, const UMImage& image) const;
};

} // umdraw
