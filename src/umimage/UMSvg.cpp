/**
 * @file UMSvg.cpp
 * svg
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */

#include "UMSvg.h"
#include "UMVector.h"
#include "UMPath.h"
#include "UMStringUtil.h"
#include "UMEvent.h"
#include "UMImage.h"

namespace umimage
{

class UMSvg::Impl
{
	DISALLOW_COPY_AND_ASSIGN(Impl);
public:

	Impl()
	{}

	~Impl() 
	{
	}
	
	bool load(const std::u16string& filepath)
	{
		//image_ = nsvgParseFromFile(filepath.c_str(), "px", 96.0f);
		//if (!image_) return false;
		return true;
	}

	bool load_from_memory(const std::string& data)
	{
		//std::string copy(data);
		//image_ = nsvgParse(const_cast<char*>(copy.c_str()), "px", 96.0f);
		//if (!image_) return false;
		return true;
	}

	UMImagePtr create_image(int width, int height)
	{
		//if (rasterizer_ = nsvgCreateRasterizer())
		//{
		//	UMImage::R8G8B8A8Buffer buffer(width * height);
		//	nsvgRasterize(
		//		rasterizer_, 
		//		image_, 
		//		0, 
		//		0, 
		//		1, 
		//		&(*buffer.begin()),
		//		width,
		//		height,
		//		width * 4);
		//	
		//	UMImagePtr image  = UMImagePtr(new UMImage());
		//	image->init(width, height);
		//	double inv_ff = 1.0 / static_cast<double>(0xFF);
		//	for (int i = 0, isize = static_cast<int>(image->mutable_list().size()); i < isize; ++i)
		//	{
		//		double r = static_cast<double>(buffer.at(i * 4 + 0)) + 0.5;
		//		double g = static_cast<double>(buffer.at(i * 4 + 1)) + 0.5;
		//		double b = static_cast<double>(buffer.at(i * 4 + 2)) + 0.5;
		//		double a = static_cast<double>(buffer.at(i * 4 + 3)) + 0.5;
		//		UMVec4d color(r * inv_ff, g * inv_ff, b * inv_ff, a * inv_ff);
		//		image->mutable_list().at(i) = color;
		//	}
		//	return image;
		//}
		return UMImagePtr();
	}

private:
};

/**
 * constructor
 */
UMSvg::UMSvg() 
	: impl_(new UMSvg::Impl())
{
}

/**
 * destructor
 */
UMSvg::~UMSvg() 
{
}

/**
 * load image from file
 */
UMSvgPtr UMSvg::load(const std::u16string& filepath)
{
	UMSvgPtr svg(std::make_shared<UMSvg>());
	if (svg->impl_->load(filepath))
	{
		return svg;
	}
	return UMSvgPtr();
}

/**
 * load image from memory
 */
UMSvgPtr UMSvg::load_from_memory(const std::string& data)
{
	UMSvgPtr svg(std::make_shared<UMSvg>());
	if (svg->impl_->load_from_memory(data))
	{
		return svg;
	}
	return UMSvgPtr();
}

/**
 * create image
 */
UMImagePtr UMSvg::create_image(int width, int height)
{
	return impl_->create_image(width, height);
}

} // umimage
