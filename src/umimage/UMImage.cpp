/**
 * @file UMImage.cpp
 * Image
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

#ifdef WITH_OIIO
#include <OpenImageIO/imageio.h>
#endif

#include <memory>
#include "UMImage.h"
#include "UMVector.h"
#include "UMPath.h"
#include "UMStringUtil.h"
#include "UMImageEventType.h"
#include "UMEvent.h"

namespace umimage
{

namespace
{
	unsigned int global_id_counter = 0;
	
#ifdef WITH_OIIO
	OIIO_NAMESPACE_USING

	UMImagePtr load_image_by_oiio(const umstring& filepath)
	{
		const std::string filename = umbase::UMStringUtil::utf16_to_utf8(filepath);

		ImageInput *in = ImageInput::open(filename);
		if (!in)
		{
			return UMImagePtr();
		}
	
		const ImageSpec& spec = in->spec();
		int width = spec.width;
		int height = spec.height;
		int channels = spec.nchannels;
		std::vector<unsigned char> buffer(width * height * channels);
		in->read_image(TypeDesc::UINT8, &buffer[0]);
		in->close();
		delete in;
		in = NULL;

		UMImagePtr image  = std::make_shared<UMImage>();
		if (image->init(width, height))
		{
			UMImage::ImageBuffer& dst = image->mutable_list();
			double inv_ff = 1.0 / (double)0xFF;
			for (int i = 0; i < height; ++i)
			{
				for (int k = 0; k < width; ++k)
				{
					int pos =  i * width + k;
					if (channels == 3)
					{
						double r = buffer[pos*3 + 0] * inv_ff;
						double g = buffer[pos*3 + 1] * inv_ff;
						double b = buffer[pos*3 + 2] * inv_ff;
						double a = 1.0;
						dst[ i * width + k] = UMVec4d(r, g, b, a);
					}
					else if (channels == 4)
					{
						double r = buffer[pos*4 + 0] * inv_ff;
						double g = buffer[pos*4 + 1] * inv_ff;
						double b = buffer[pos*4 + 2] * inv_ff;
						double a = buffer[pos*4 + 3] * inv_ff;
						dst[ i * width + k] = UMVec4d(r, g, b, a);
					}
				}
			}
		}
		return image;
	}
#endif

	UMImagePtr load_image_by_soil(const umstring& filepath)
	{
		const std::string filename = umbase::UMStringUtil::utf16_to_utf8(filepath);
		int width = 0;
		int height = 0;
		int channels = 0;
		unsigned char* buffer = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
		if (!buffer) return UMImagePtr();

		UMImagePtr image  = std::make_shared<UMImage>();
		if (image->init(width, height))
		{
			UMImage::ImageBuffer& dst = image->mutable_list();
			double inv_ff = 1.0 / (double)0xFF;
			for (int i = 0; i < height; ++i)
			{
				for (int k = 0; k < width; ++k)
				{
					int pos =  i * width + k;
					if (channels == 3)
					{
						double r = buffer[pos*4 + 0] * inv_ff;
						double g = buffer[pos*4 + 1] * inv_ff;
						double b = buffer[pos*4 + 2] * inv_ff;
						double a = 1.0;
						dst[ i * width + k] = UMVec4d(r, g, b, a);
					}
					else if (channels == 4)
					{
						double r = buffer[pos*4 + 0] * inv_ff;
						double g = buffer[pos*4 + 1] * inv_ff;
						double b = buffer[pos*4 + 2] * inv_ff;
						double a = buffer[pos*4 + 3] * inv_ff;
						dst[ i * width + k] = UMVec4d(r, g, b, a);
					}
				}
			}
		}
		free(buffer);
		return image;
	}
}

/**
 * constructor
 */
UMImage::UMImage() 
	: width_(0)
	, height_(0)
	, id_(global_id_counter++)
	, image_change_event_(new umbase::UMEvent(eImageEventImageChaged))
{
}

/**
 * destructor
 */
UMImage::~UMImage() 
{
}

/**
 * load image from file
 */
UMImagePtr UMImage::load(const umstring& filepath)
{
	#ifdef WITH_OIIO
		return load_image_by_oiio(filepath);
	#else
		return load_image_by_soil(filepath);
	#endif
}

/**
 * load image from memory
 */
UMImagePtr UMImage::load_from_memory(const std::string& data)
{
	int width = 0;
	int height = 0;
	int channels = 0;
	unsigned char* buffer = stbi_load_from_memory(
		reinterpret_cast<const unsigned char*>(data.c_str()), 
		static_cast<int>(data.size()),
		&width, 
		&height, 
		&channels, 
		STBI_rgb_alpha);
	if (!buffer) return UMImagePtr();

	UMImagePtr image  = std::make_shared<UMImage>();
	if (image->init(width, height))
	{
		ImageBuffer& dst = image->mutable_list();
		double inv_ff = 1.0 / (double)0xFF;
		for (int i = 0; i < height; ++i)
		{
			for (int k = 0; k < width; ++k)
			{
				int pos =  i * width + k;
				if (channels == 3)
				{
					double r = buffer[pos*4 + 0] * inv_ff;
					double g = buffer[pos*4 + 1] * inv_ff;
					double b = buffer[pos*4 + 2] * inv_ff;
					double a = 1.0;
					dst[ i * width + k] = UMVec4d(r, g, b, a);
				}
				else if (channels == 4)
				{
					double r = buffer[pos*4 + 0] * inv_ff;
					double g = buffer[pos*4 + 1] * inv_ff;
					double b = buffer[pos*4 + 2] * inv_ff;
					double a = buffer[pos*4 + 3] * inv_ff;
					dst[ i * width + k] = UMVec4d(r, g, b, a);
				}
			}
		}
	}
	free(buffer);
	return image;
}

bool UMImage::save(const umstring& filepath, UMImagePtr src, ImageType type)
{
	if (!src) return false;
	if (!src->is_valid()) return false;
	std::string filename = umbase::UMStringUtil::utf16_to_utf8(filepath);
	
	int result = 0;
	if (type == eImageTypeBMP_RGB)
	{
		UMImage::R8G8B8Buffer img;
		src->create_r8g8b8_buffer(img);
		stbi_write_bmp(
			filename.c_str(), 
			src->width(), 
			src->height(), 
			STBI_rgb,
			&(*img.begin()));
	}
	else if (type == eImageTypeTGA_RGB)
	{
		UMImage::R8G8B8Buffer img;
		src->create_r8g8b8_buffer(img);
		stbi_write_tga(
			filename.c_str(), 
			src->width(), 
			src->height(), 
			STBI_rgb,
			&(*img.begin()));
	}
	else if (type == eImageTypeTGA_RGBA)
	{
		UMImage::R8G8B8A8Buffer img;
		src->create_r8g8b8a8_buffer(img);
		stbi_write_tga(
			filename.c_str(), 
			src->width(), 
			src->height(), 
			STBI_rgb_alpha,
			&(*img.begin()));
	}
	else if (type == eImageTypePNG_RGBA)
	{
		UMImage::R8G8B8A8Buffer img;
		src->create_r8g8b8a8_buffer(img);
		
		stbi_write_png(
			filename.c_str(), 
			src->width(), 
			src->height(), 
			STBI_rgb_alpha, 
			&(*img.begin()), 
			0);
	}
	return (result == 1);
}

/**
 * init image
 */
bool UMImage::init(int width, int height)
{
	width_ = width;
	height_ = height;
	buffer_.clear();
	buffer_.resize(width * height);
	return true;
}
/**
 * create r8g8b8a8 buffer
 */
void UMImage::create_r8g8b8a8_buffer(UMImage::R8G8B8A8Buffer& img) const 
{
	img.resize(width() * height() * 4);
	for (int y = 0; y < height(); ++y)
	{
		for (int x = 0; x < width(); ++x)
		{
			const int pos = width() * y + x;
			const UMVec4d& col = list().at(pos);
			img[pos * 4 + 0] = static_cast<int>(col.x * 255.0 + 0.5);
			img[pos * 4 + 1] = static_cast<int>(col.y * 255.0 + 0.5);
			img[pos * 4 + 2] = static_cast<int>(col.z * 255.0 + 0.5);
			img[pos * 4 + 3] = static_cast<int>(col.w * 255.0 + 0.5);
		}
	}
}

/**
 * create buffer by rect
 */
void UMImage::create_r8g8b8a8_buffer(R8G8B8A8Buffer& img, const UMVec4ui& src_rect) const
{
	const double inv_gamma = 1.0;
	int w = src_rect[2] - src_rect[0];
	int h = src_rect[3] - src_rect[1];

	int y = src_rect[1];
	img.resize(w * h * 4);
	for (int i = 0; i < h; ++y, ++i)
	{
		int x = src_rect[0];
		for (int k = 0; k < w; ++x, ++k)
		{
			int src_pos = width() * (height() - y - 1) + x;
			int dst_pos = w * i + k;
			const UMVec4d col = list().at(src_pos);
			img[dst_pos * 4 + 0] = static_cast<int>(pow(col.x, inv_gamma) * 0xFF + 0.5);
			img[dst_pos * 4 + 1] = static_cast<int>(pow(col.y, inv_gamma) * 0xFF + 0.5);
			img[dst_pos * 4 + 2] = static_cast<int>(pow(col.z, inv_gamma) * 0xFF + 0.5);
			img[dst_pos * 4 + 3] = static_cast<int>(pow(col.w, inv_gamma) * 0xFF + 0.5);
		}
	}
}

/**
 * create b8g8r8 buffer
 */
void UMImage::create_r8g8b8_buffer(UMImage::R8G8B8Buffer& img) const 
{
	const double inv_gamma = 1.0;// / 2.2;
	img.resize(width() * height() * 3);
	for (int y = 0; y < height(); ++y)
	{
		for (int x = 0; x < width(); ++x)
		{
			int pos = width() * y + x;
			const UMVec4d col = list().at(pos);
			img[pos * 3 + 0] = static_cast<int>(pow(col.x, inv_gamma) * 0xFF + 0.5);
			img[pos * 3 + 1] = static_cast<int>(pow(col.y, inv_gamma) * 0xFF + 0.5);
			img[pos * 3 + 2] = static_cast<int>(pow(col.z, inv_gamma) * 0xFF + 0.5);
		}
	}
}

/**
 * create b8g8r8 buffer
 */
void UMImage::create_b8g8r8_buffer(UMImage::B8G8R8Buffer& img) const 
{
	const double inv_gamma = 1.0;// / 2.2;
	img.resize(width() * height() * 3);
	for (int y = 0; y < height(); ++y)
	{
		for (int x = 0; x < width(); ++x)
		{
			int pos = width() * y + x;
			const UMVec4d col = list().at(pos);
			img[pos * 3 + 0] = static_cast<int>(pow(col.z, inv_gamma) * 0xFF + 0.5);
			img[pos * 3 + 1] = static_cast<int>(pow(col.y, inv_gamma) * 0xFF + 0.5);
			img[pos * 3 + 2] = static_cast<int>(pow(col.x, inv_gamma) * 0xFF + 0.5);
		}
	}
}

/**
 * clear image buffer
 */
void UMImage::clear()
{
	for (int y = 0; y < height(); ++y)
	{
		for (int x = 0; x < width(); ++x)
		{
			int pos = width() * y + x;
			mutable_list().at(pos) = UMVec4d(0);
		}
	}
}

/**
 * create flip image
 */
UMImagePtr UMImage::create_flip_image(bool horizon, bool vertical) const
{
	if (!is_valid()) return UMImagePtr();
	
	UMImagePtr dst(std::make_shared<UMImage>());
	dst->mutable_list().resize(list().size());
	dst->set_width(width());
	dst->set_height(height());

	for (int y = 0; y < height(); ++y)
	{
		for (int x = 0; x < width(); ++x)
		{
			int dstx = x;
			int dsty = y;
			if (horizon) { dstx = width() - x - 1; }
			if (vertical) { dsty = height() - y - 1; }
			
			int src_pos = width() * y + x;
			int dst_pos = width() * dsty + dstx;
			dst->mutable_list().at(dst_pos) = list().at(src_pos);
		}
	}
	return dst;
}

/**
 * copy image to dst
 */
bool UMImage::copy(UMImagePtr dst, const UMVec4ui& dst_rect)
{
	int left = dst_rect[0];
	int top = dst_rect[1];
	int right = dst_rect[2];
	int bottom = dst_rect[3];
	int width = right - left;
	int height = bottom - top;

	// range check
	if (!dst) return false;
	if (right < left) return false;
	if (bottom < top) return false;
	if (dst->width() < right) return false;
	if (dst->height() < bottom) return false;
	if (width_ != width) return false;
	if (height_ != height) return false;
	
	for (int y = top; y < bottom; ++y)
	{
		for (int x = left; x < right; ++x)
		{
			dst->mutable_list().at(y * dst->width() + x)
				= buffer_.at( (y-top) * width_ + (x-left) );
		}
	}
	return true;
}

/**
 * fill image
 */
void UMImage::fill(const UMVec4d& color)
{
	for (UMImage::ImageBuffer::iterator it = mutable_list().begin(); it != mutable_list().end(); ++it)
	{
		(*it) = umbase::UMVec4d(color);
	}
}

} // umimage
