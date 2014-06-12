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
#ifdef WITH_FREETYPE

#include "UMFont.h"

#include <map>
#include <ft2build.h>
#include FT_FREETYPE_H
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
	FT_Library  library;
	// font name, face
	typedef std::map<umstring, FT_Face> FontFaceMap;
	FontFaceMap font_face_map;

	typedef std::vector<char> FontBuffer;
	// font name, buffer
	typedef std::map<umstring, FontBuffer> FontBufferMap;
	FontBufferMap font_buffer_map;

	const int initial_font_size = 32;

#if !defined(WITH_EMSCRIPTEN)
	bool load_font_face(void* hWnd, const umstring& font_name)
	{
		if (!hWnd) return false;
		if (font_face_map.find(font_name) != font_face_map.end()) return true;

		FontBuffer& font_buffer = font_buffer_map[font_name];

		// get font binary
		HWND hwnd = *(reinterpret_cast<HWND*>(&hWnd));
		HDC hdc = GetDC(hwnd);
		std::wstring wname = umbase::UMStringUtil::utf16_to_wstring(font_name);
		const wchar_t* name = wname.c_str();
		if (HFONT font = CreateFont(
			initial_font_size,
			0,
			0,
			0,
			FW_REGULAR,
			FALSE, // italic
			FALSE, // underline
			FALSE, // strikeout
			DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE,
			name))
		{
			if (HGDIOBJ old = SelectObject(hdc, font))
			{
				if (UINT buffer_size = GetOutlineTextMetrics(hdc, 0, NULL))
				{
					std::vector<char> metric_buffer(buffer_size);
					LPOUTLINETEXTMETRIC metric = reinterpret_cast<LPOUTLINETEXTMETRIC>(&(*metric_buffer.begin()));
					if (GetOutlineTextMetrics(hdc, buffer_size, metric))
					{
						if ((metric->otmfsType & 1) == 0)
						{
							if (DWORD byte = GetFontData(hdc, 0x66637474, 0, NULL, 0))
							{
								if (byte != GDI_ERROR)
								{
									font_buffer.resize(byte);
									if (GetFontData(hdc, 0x66637474, 0, &(*font_buffer.begin()), byte) == GDI_ERROR)
									{
										font_buffer.clear();
									}
								}
							}
						}
					}
				}
				DeleteObject(old);
			}
		}
		ReleaseDC(hwnd, hdc);
		
		FT_Face& font_face = font_face_map[font_name];
		
		if (int error = FT_New_Memory_Face(
			library, 
			reinterpret_cast<const FT_Byte*>(&(*font_buffer.begin())),
			static_cast<FT_Long>(font_buffer.size()),
			0,
			&font_face))
		{
			return false;
		}
		// for FT_LOAD_DEFAULT
		if (int error = FT_Set_Pixel_Sizes(
			font_face,
			0,
			initial_font_size))
		{
			return false;
		}
		return true;
	}
#endif // !defined(WITH_EMSCRIPTEN)
	
	bool load_font_face_from_file(const umstring& file_name)
	{
		printf("load font file name %s\n", file_name.c_str());
		std::string name = umbase::UMStringUtil::utf16_to_utf8(file_name);
		FT_Face font_face;
		if (int error = FT_New_Face(
			library, 
			name.c_str(),
			0,
			&font_face))
		{
			return false;
		}
		if (int error = FT_Set_Pixel_Sizes(
			font_face,
			0,
			initial_font_size))
		{
			return false;
		}
		font_face_map[file_name] = font_face;
			
		return true;
	}

	bool load_font_face_from_memory(const umstring& font_name, const std::string& data)
	{
		std::string name = umbase::UMStringUtil::utf16_to_utf8(font_name);
		FT_Face font_face;
		if (int error = FT_New_Memory_Face(
			library, 
			reinterpret_cast<const FT_Byte*>(data.c_str()),
			data.size(),
			0,
			&font_face))
		{
			return false;
		}
		if (int error = FT_Set_Pixel_Sizes(
			font_face,
			0,
			initial_font_size))
		{
			return false;
		}
		font_face_map[font_name] = font_face;
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
	int error = FT_Init_FreeType( &library );
	if (error) 
	{
		is_valid_ = false;
	}
	else
	{
		is_valid_ = true;
	}
	atlas_list.push_back(std::make_shared<umimage::UMTextureAtlas>(atlas_width, atlas_height));
}

/**
 * destructor
 */
UMFont::~UMFont()
{
	for (FontFaceMap::iterator it = font_face_map.begin(); it != font_face_map.end(); ++it)
	{
		FT_Done_Face(it->second);
	}
	if (library) FT_Done_FreeType(library);
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

		const  FT_Face& font_face = it->second;
		
		if (int error = FT_Set_Pixel_Sizes(
			font_face,
			0,
			font_size))
		{
			return UMImagePtr();
		}
		for (int i = 0; i < static_cast<int>(text.size()); ++i)
		{
			FT_UInt glyph_index = FT_Get_Char_Index(font_face, text[i]);
			if (static_cast<int>(text[i]) < 0xFF && isgraph(text[i]))
			{
				// ascii
				if (int error = FT_Load_Glyph(font_face, glyph_index, FT_LOAD_NO_BITMAP))
				{
					continue;
				}
			}
			else
			{
				if (int error = FT_Load_Glyph(font_face, glyph_index, FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT))
				{
					continue;
				}
			}

			if (int error = FT_Render_Glyph(font_face->glyph, FT_RENDER_MODE_NORMAL))
			{
				continue;
			}
				
			const FT_GlyphSlot slot = font_face->glyph;
			unsigned char* bitmap_buffer = slot->bitmap.buffer;
			const int bitmap_w = slot->bitmap.width;
			const int bitmap_h = slot->bitmap.rows;
			const int advance_x = static_cast<int>(slot->metrics.horiAdvance/64.0);
			const int advance_y = static_cast<int>(slot->metrics.vertAdvance/64.0);
			const int iyoffset = font_size - slot->bitmap_top;
			const int ixoffset = slot->bitmap_left;
				
			const int xsize = posx + bitmap_w;
			const int ysize = posy + bitmap_h;

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
			imagex += advance_x;
			if (imagex >= image_w - font_size)
			{
				imagex = 0;
				imagey += advance_y;
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
#if !defined(WITH_EMSCRIPTEN)
	if (load_font_face(hWnd, font_name))
	{
		return true;
	}
#endif
	return false;
}

/**
 * load font from file
 */
bool UMFont::load_font_from_file(const umstring& font_name) const
{
	if (is_font_loaded(font_name)) return true;
	if (load_font_face_from_file(font_name))
	{
		return true;
	}
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
	
	FT_Face font_face;
	
	if (is_found_font)
	{
		font_face = font_face_map.find(font_name)->second;
	}
	else
	{
		font_face = font_face_map.begin()->second;
	}

	if (int error = FT_Set_Pixel_Sizes(
		font_face,
		0,
		font_size))
	{
		return UMTextureAtlasPtr();
	}
	for (int i = 0; i < static_cast<int>(text.size()); ++i)
	{
		if (texture_atlas->is_exist(text[i]))
		{
			continue;
		}
		FT_UInt glyph_index = FT_Get_Char_Index(font_face, text[i]);
		if (static_cast<int>(text[i]) < 0xFF && isgraph(text[i]))
		{
			// ascii
			if (int error = FT_Load_Glyph(font_face, glyph_index, FT_LOAD_NO_BITMAP))
			{
				continue;
			}
		}
		else
		{
			if (int error = FT_Load_Glyph(font_face, glyph_index, FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT))
			{
				continue;
			}
		}

		if (int error = FT_Render_Glyph(font_face->glyph, FT_RENDER_MODE_NORMAL))
		{
			continue;
		}
				
		const FT_GlyphSlot slot = font_face->glyph;
		unsigned char* bitmap_buffer = slot->bitmap.buffer;
		const int bitmap_w = slot->bitmap.width;
		const int bitmap_h = slot->bitmap.rows;
		const int advance_x = static_cast<int>(slot->metrics.horiAdvance/64.0);
		const int advance_y = static_cast<int>(slot->metrics.vertAdvance/64.0);
		const int iyoffset = font_size - slot->bitmap_top;
		const int ixoffset = slot->bitmap_left;
				
		// bitmap : base point is left-bottom.
		int posx = 0;
		int posy = 0;
		int imagex = 0;
		int imagey = 0;
				
		const int xsize = posx + bitmap_w;
		const int ysize = posy + bitmap_h;
		// create image
		UMImagePtr image(std::make_shared<UMImage>());
		image->init(advance_x + 2, ysize + iyoffset + 2);
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

#endif // WITH_FREETYPE
