/**
 * @file UMImage.h
 * Image
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

namespace umbase
{
class UMEvent;
typedef std::shared_ptr<UMEvent> UMEventPtr;
} // umbase

/// Uimac image library
namespace umimage
{

class UMImage;
typedef std::shared_ptr<UMImage> UMImagePtr;
typedef std::weak_ptr<UMImage> UMImageWeakPtr;

/**
 * Image
 */
class UMImage
{
	DISALLOW_COPY_AND_ASSIGN(UMImage);
public:
	typedef std::vector<UMVec4d> ImageBuffer;
	typedef std::vector<unsigned char> R8G8B8A8Buffer;
	typedef std::vector<unsigned char> B8G8R8Buffer;
	typedef std::vector<unsigned char> R8G8B8Buffer;

	enum ImageType {
		eImageTypeBMP_RGB,
		eImageTypeTGA_RGB,
		eImageTypeTGA_RGBA,
	};
	
	UMImage();
	
	~UMImage();

	/**
	 * get id
	 */
	unsigned int id() const { return id_; }

	/**
	 * load image from file
	 */
	static UMImagePtr load(const std::u16string& filepath);
	
	/**
	 * load image from memory
	 */
	static UMImagePtr load_from_memory(const std::string& data);

	/**
	 * save image to file
	 */
	static bool save(const std::u16string& filepath, UMImagePtr src, ImageType type);

	/**
	 * init image
	 */
	bool init(int width, int height);

	/**
	 * get image buffer
	 */
	const ImageBuffer& list() const { return buffer_; } 
	
	/**
	 * get image buffer
	 */
	ImageBuffer&  mutable_list() { return buffer_; }

	/**
	 * create r8g8b8a8 buffer
	 */
	void create_r8g8b8a8_buffer(R8G8B8A8Buffer& buffer) const;

	/**
	 * create buffer by rect
	 */
	void create_r8g8b8a8_buffer(R8G8B8A8Buffer& buffer, const UMVec4ui& src_rect) const;

	/**
	 * create b8g8r8 buffer
	 */
	void create_b8g8r8_buffer(B8G8R8Buffer& buffer) const;
	
	/**
	 * create r8g8b8 buffer
	 */
	void create_r8g8b8_buffer(R8G8B8Buffer& buffer) const;

	/**
	 * clear image buffer
	 */
	void clear();

	/**
	 * get width
	 */
	int width() const { return width_; }

	/**
	 * set width
	 * @param [in] width width
	 */
	void set_width(int width) { width_ = width; }

	/**
	 * get height
	 */
	int height() const { return height_; }
	
	/**
	 * set height
	 * @param [in] height height
	 */
	void set_height(int height) { height_ = height; }

	/**
	 * validate image
	 * @retval valid or invalid
	 */
	bool is_valid() const { return (width_ * height_ == static_cast<int>(buffer_.size())); }

	/**
	 * create flip image
	 * @param [in] horizon flip horizontal
	 * @param [in] vertical flip vertical
	 * @retval filpped image
	 */
	UMImagePtr create_flip_image(bool horizon, bool vertical) const;

	/**
	 * copy image to dst
	 * @param [out] dst distination
	 * @param [in] dst_rect width and height must be same to this image
	 */
	bool copy(UMImagePtr dst, const UMVec4ui& dst_rect);

	/**
	 * fill image
	 */
	void fill(const UMVec4d& color);
	
	/**
	 * get image change event
	 */
	umbase::UMEventPtr image_change_event() { return image_change_event_; }

private:
	int width_;
	int height_;
	unsigned int id_;
	ImageBuffer buffer_;
	umbase::UMEventPtr image_change_event_;
};

} // umimage
