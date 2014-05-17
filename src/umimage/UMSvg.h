/**
 * @file UMSvg.h
 * svg
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <vector>
#include <memory>
#include "UMMacro.h"
#include "UMMathTypes.h"
#include "UMVector.h"

namespace umimage
{

class UMImage;
typedef std::shared_ptr<UMImage> UMImagePtr;

class UMSvg;
typedef std::shared_ptr<UMSvg> UMSvgPtr;

/**
 * svg
 */
class UMSvg
{
	DISALLOW_COPY_AND_ASSIGN(UMSvg);
public:
	UMSvg();
	
	~UMSvg();

	/**
	 * load image from file
	 */
	static UMSvgPtr load(const std::u16string& filepath);
	
	/**
	 * load image from memory
	 */
	static UMSvgPtr load_from_memory(const std::string& data);
	
	/**
	 * create image
	 */
	UMImagePtr create_image(int width, int height);

private:
	class Impl;
	typedef std::unique_ptr<Impl> ImplPtr;
	ImplPtr impl_;
};

} // umimage
