/**
 * @file UMTextureAtlas.h
 * texture atlas
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
#include "UMMacro.h"
#include "UMMathTypes.h"
#include "UMVector.h"

namespace umimage
{
	
class UMImage;
typedef std::shared_ptr<UMImage> UMImagePtr;

class UMTextureAtlas;
typedef std::shared_ptr<UMTextureAtlas> UMTextureAtlasPtr;

/**
 * texture atlas
 */
class UMTextureAtlas
{
	DISALLOW_COPY_AND_ASSIGN(UMTextureAtlas);
public:
	UMTextureAtlas(int width, int height);
	~UMTextureAtlas();

	/**
	 * get atlas image
	 */
	UMImagePtr atlas_image();
	
	/**
	 * add text image to atlas
	 */
	bool add_text_image(UMImagePtr image, const char16_t& text);
	
	/**
	 * text exists or not
	 */
	bool is_exist(const char16_t& text) const;

	/**
	 * get text bounds
	 * @retval UMVec4i (left, top, right, bottom)
	 */
	umbase::UMVec4ui text_rect(const char16_t& text) const;

private:
	class AtlasImpl;
	typedef std::unique_ptr<AtlasImpl> AtlasImplPtr;
	AtlasImplPtr impl_;
};

} // umdraw
