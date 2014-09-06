/**
 * @file UMFont.cpp
 * font
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_STBTRUETYPE

#include "UMFont.h"

#include <map>
#include <ft2build.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#include <ctype.h>

#ifdef WITH_EMSCRIPTEN
	#include <emscripten.h>
#else
	#include <tchar.h>
	#include <Windows.h>
#endif // WITH_EMSCRIPTEN

#include "UMImage.h"
#include "UMPath.h"
#include "UMStringUtil.h"
#include "UMVector.h"
#include "UMTextureAtlas.h"

namespace
{
	// font name, face
	typedef std::map<umstring, stbtt_fontinfo> FontFaceMap;
	FontFaceMap font_face_map;

	typedef std::vector<char> FontBuffer;
	// font name, buffer
	typedef std::map<umstring, FontBuffer> FontBufferMap;
	FontBufferMap font_buffer_map;

	const int initial_font_size = 32;

	bool load_font_face_from_memory(const umstring& font_name, const std::string& data)
	{
		std::string name = umbase::UMStringUtil::utf16_to_utf8(font_name);
		stbtt_fontinfo& font = font_face_map[font_name];
		const unsigned char* buffer = reinterpret_cast<const unsigned char*>(data.c_str());
		stbtt_InitFont(&font, buffer, stbtt_GetFontOffsetForIndex(buffer,0) );
		return true;
	}

	typedef std::vector<umimage::UMTextureAtlasPtr> AtlasList;
	AtlasList atlas_list;

	const int atlas_width = 256;
	const int atlas_height = 256;

} // anonymouse namespace

namespace umimage
{
	using namespace umbase;

/**
 * constructor
 */
UMFont::UMFont()
{
	is_valid_ = true;
	atlas_list.push_back(std::make_shared<umimage::UMTextureAtlas>(atlas_width, atlas_height));
}

/**
 * destructor
 */
UMFont::~UMFont()
{
}

/**
 * singleton
 */
const UMFont* UMFont::instance()
{
	static UMFont font;
	if (font.is_valid_)
	{
		return &font;
	}
	return NULL;
}

/**
 * font image
 */
UMImagePtr UMFont::font_image(const umstring& font_name, const umtextstring& text, int font_size, int image_width, int image_height) const
{
	if (text.empty()) return UMImagePtr();
	if (!is_font_loaded(font_name)) return UMImagePtr();

	UMImagePtr image;
	FontFaceMap::const_iterator it = font_face_map.find(font_name);
	if (it != font_face_map.end())
	{
		// create image
		image = std::make_shared<UMImage>();
		image->init(image_width, image_height);
		image->fill(umbase::UMVec4d(1));

		// image  : base point is left-top.
		const int image_w = image->width();
		const int image_h = image->height();

		// bitmap : base point is left-bottom.
		int posx = 0;
		int posy = 0;
		int imagex = 0;
		int imagey = 0;

		const  stbtt_fontinfo& font_face = it->second;
		
		for (int i = 0; i < static_cast<int>(text.size()); ++i)
		{
			int bitmap_w = 0;
			int bitmap_h = 0;
			int ixoffset = 0;
			int iyoffset = 0;

			const int xsize = posx + bitmap_w;
			const int ysize = posy + bitmap_h;
			
			unsigned char* bitmap_buffer = stbtt_GetCodepointBitmap(
				&font_face, 
				0, 
				stbtt_ScaleForPixelHeight(&font_face, static_cast<float>(font_size)),
				text[i],
				&bitmap_w,
				&bitmap_h,
				&ixoffset,
				&iyoffset);

			for (int y = posy, iy = imagey; y < ysize; ++y, ++iy)
			{
				for (int x = posx, ix = imagex; x < xsize; ++x, ++ix)
				{
					const int buffer_pos = bitmap_w * (y - posy) + (x - posx);
					const int image_pos = image_w * (iy + iyoffset) + (ix + ixoffset);
					const double alpha = bitmap_buffer[buffer_pos] / static_cast<double>(0xFF);
					const double inv_alpha = 1.0 - alpha;
					// for alpha image
					// image->mutable_list().at(image_pos) = umbase::UMVec4d(0, 0, 0, alpha);
					image->mutable_list().at(image_pos) = umbase::UMVec4d(inv_alpha, inv_alpha, inv_alpha, 1.0);
				};
			}
			posx += bitmap_w;
			imagex += bitmap_w;
			//imagex += advance_x;
			if (imagex >= image_w - font_size)
			{
				imagex = 0;
				imagey += bitmap_h;
				//imagey += advance_y;
			}
		}
	}
	return image;
}

/**
 * load font from system
 */
bool UMFont::load_font(void* hWnd, const umstring& font_name) const
{
	if (is_font_loaded(font_name)) return true;
//#if !defined(WITH_EMSCRIPTEN)
//	if (load_font_face(hWnd, font_name))
//	{
//		return true;
//	}
//#endif
	return false;
}

/**
 * load font from file
 */
bool UMFont::load_font_from_file(const umstring& font_name) const
{
	if (is_font_loaded(font_name)) return true;
	//if (load_font_face_from_file(font_name))
	//{
	//	return true;
	//}
	return false;
}

/**
 * load font from memory
 */
bool UMFont::load_font_from_memory(const umstring& font_name, const std::string& data) const
{
	if (is_font_loaded(font_name)) return true;
	if (load_font_face_from_memory(font_name, data))
	{
		return true;
	}
	return false;
}

/**
 * is font loaded
 */
bool UMFont::is_font_loaded(const umstring& font_name) const
{
	return (font_face_map.find(font_name) != font_face_map.end());
}

/**
 * get font image
 */
UMTextureAtlasPtr UMFont::font_atlas(const umstring& font_name, const umtextstring& text, int font_size) const
{
	if (text.empty()) return UMTextureAtlasPtr();
	bool is_found_font = is_font_loaded(font_name);
	if (!is_found_font && font_face_map.empty()) return UMTextureAtlasPtr();

	UMTextureAtlasPtr texture_atlas = atlas_list.back();
	
	stbtt_fontinfo font_face;
	
	if (is_found_font)
	{
		font_face = font_face_map.find(font_name)->second;
	}
	else
	{
		font_face = font_face_map.begin()->second;
	}
	
	// bitmap : base point is left-bottom.
	int posx = 0;
	int posy = 0;
	int imagex = 0;
	int imagey = 0;

	for (int i = 0; i < static_cast<int>(text.size()); ++i)
	{
		if (texture_atlas->is_exist(text[i]))
		{
			continue;
		}

		int bitmap_w = 0;
		int bitmap_h = 0;
		int ixoffset = 0;
		int iyoffset = 0;
		
		int ascent;
		int descent;
		int line_gap;
		stbtt_GetFontVMetrics(&font_face, &ascent, &descent, &line_gap);

		int bitmap_left;
		int advance_x;
		stbtt_GetCodepointHMetrics(&font_face, text[i], &advance_x, &bitmap_left);

		unsigned char* bitmap_buffer = stbtt_GetCodepointBitmap(
			&font_face, 
			0, 
			stbtt_ScaleForPixelHeight(&font_face, static_cast<float>(font_size)),
			text[i],
			&bitmap_w,
			&bitmap_h,
			&ixoffset,
			&iyoffset);

		iyoffset += font_size;
		
		const int xsize = posx + bitmap_w;
		const int ysize = posy + bitmap_h;
			
		// create image
		UMImagePtr image = std::make_shared<UMImage>();
		image->init(bitmap_w + ixoffset + 2, bitmap_h + iyoffset + 2);
		image->fill(umbase::UMVec4d(0));

		// image  : base point is left-top.
		const int image_w = image->width();
		const int image_h = image->height();
			
		for (int y = posy, iy = imagey; y < ysize; ++y, ++iy)
		{
			for (int x = posx, ix = imagex; x < xsize; ++x, ++ix)
			{
				const int buffer_pos = bitmap_w * (y - posy) + (x - posx);
				const int image_pos = image_w * (iy + iyoffset) + (ix + ixoffset);
				const double alpha = bitmap_buffer[buffer_pos] / static_cast<double>(0xFF);
				const double inv_alpha = 1.0 - alpha;
				// for alpha image
				image->mutable_list().at(image_pos) = umbase::UMVec4d(1, 1, 1, alpha);
				//image->mutable_list().at(image_pos) = umbase::UMVec4d(inv_alpha, inv_alpha, inv_alpha, 1.0);
			};
		}

		stbtt_FreeBitmap(bitmap_buffer, 0);

		if (!texture_atlas->add_text_image(image, text[i]))
		{
			UMTextureAtlasPtr new_atlas = std::make_shared<UMTextureAtlas>(atlas_width, atlas_height);
			atlas_list.push_back(new_atlas);
			if (font_atlas(font_name, text, font_size))
			{
				return new_atlas;
			}
		}
	}
	return texture_atlas;
}

} // umimage

#endif // WITH_STBTRUETYPE
