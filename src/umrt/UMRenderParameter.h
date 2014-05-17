/**
 * @file UMRenderParameter.h
 * rendering parameters
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <vector>
#include "UMMacro.h"
#include "UMImage.h"
#include "UMVector.h"

namespace umrt
{

/**
 * rendering parameters
 */
class UMRenderParameter
{
	DISALLOW_COPY_AND_ASSIGN(UMRenderParameter);
public:
	UMRenderParameter() 
		: super_sampling_count_(2, 2)
		, sample_count_(4)
		, output_image_(std::make_shared<UMImage>())
	{}

	UMRenderParameter(int width, int height)
		: super_sampling_count_(2, 2)
		, sample_count_(4)
		, output_image_(std::make_shared<UMImage>())
	{
		if (UMImagePtr image = output_image())
		{
			image->init(width, height);
		}
	}

	~UMRenderParameter() {}

	/**
	 * get output image
	 */
	UMImagePtr output_image() { return output_image_; } 

	/** 
	 * get sample count par pixel
	 */
	int sample_count() const { return sample_count_; }

	/**
	 * get super sampling
	 */
	UMVec2i super_sampling_count() const { return super_sampling_count_; }
	
private:
	UMImagePtr output_image_;
	//UMImagePtr temporary_image_;
	int sample_count_;
	UMVec2i super_sampling_count_;
};

} // umrt
