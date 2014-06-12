/**
 * @file UMFont.h
 * font
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <memory>
#include <string>
#include <vector>
#include "UMMacro.h"
#include "UMMathTypes.h"

namespace umimage
{
	
class UMImage;
typedef std::shared_ptr<UMImage> UMImagePtr;

class UMTextureAtlas;
typedef std::shared_ptr<UMTextureAtlas> UMTextureAtlasPtr;

/**
 * Font
 */
class UMFont
{
	DISALLOW_COPY_AND_ASSIGN(UMFont);
public:
	~UMFont();
	static const UMFont* instance();

	/**
	 * load font from system
	 */
	bool load_font(void* hWnd, const umstring& font_name) const;

	/**
	 * load font from file
	 */
	bool load_font_from_file(const umstring& file_name) const;
	
	/**
	 * load font from memort
	 */
	bool load_font_from_memory(const umstring& file_name, const std::string& data) const;

	/**
	 * is font loaded
	 */
	bool is_font_loaded(const umstring& font_name) const;

	/**
	 * get font image. initialize is requred.
	 */
	UMTextureAtlasPtr font_atlas(const umstring& font_name, const umtextstring& text, int font_size) const;
	
	/**
	 * create font image 
	 */
	UMImagePtr font_image(const umstring& font_name, const umtextstring& text, int font_size, int image_width, int image_height) const;

private:
	bool is_valid_;
	UMFont();
};

} // umimage
