/**
 * @file UMOpenGLTexture.h
 * a texture
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <string>
#include <memory>
#include <map>

#include "UMMacro.h"
#include "UMImage.h"

namespace umdraw
{
	
class UMOpenGLTexture;
typedef std::shared_ptr<UMOpenGLTexture> UMOpenGLTexturePtr;
typedef std::vector<UMOpenGLTexturePtr> UMOpenGLTextureList;

/**
 * a texture
 */
class UMOpenGLTexture
{
	DISALLOW_COPY_AND_ASSIGN(UMOpenGLTexture);

public:

	enum InternalFormat
	{
		eRGBA,
	};

	enum Type
	{
		eUNSIGNED_BYTE,
	};

	struct Format
	{
		int internal_format;
		int format;
		int type;
		Format() : internal_format(eRGBA)
			, format(eRGBA)
			, type(eUNSIGNED_BYTE) {}
	};

	/**
	 * constructor
	 * @param [in] can_overwrite if true,  can overwrite()
	 */
	explicit UMOpenGLTexture(bool can_overwrite);

	/**
	 * destructor
	 */
	~UMOpenGLTexture();
	
	/**
	 * create texture without image/file
	 * @param [in] width width
	 * @param [in] height height
	 * @param [in] format buffer format
	 */
	static UMOpenGLTexturePtr create_texture(int width, int height, const Format& format);
	
	/**
	 * create render buffer
	 * @param [in] width width
	 * @param [in] height height
	 */
	static UMOpenGLTexturePtr create_render_buffer(int width, int height);
	
	/**
	 * create frame buffer
	 * @param [in] textures color buffers
	 * @param [in] render_buffer depth buffer
	 */
	static UMOpenGLTexturePtr create_frame_buffer(UMOpenGLTextureList& textures, UMOpenGLTexturePtr render_buffer);

	/**
	 * load an image file as a directx texture
	 * @param [in] file_path absolute texture file path
	 * @retval success or fail
	 */
	bool load(const umstring& file_path);

	/**
	 * convert UMImage to DirectX11 Texture
	 * @param [in] image source image
	 * @retval success or fail
	 */
	bool convert_from_image(const umimage::UMImage& image);

	/**
	 * convert DirectX texture buffer to memory
	 * @retval UMImagePtr converted image buffer
	 */
	umimage::UMImagePtr convert_to_image() const;

	/**
	 * overwrite texture by image
	 * @info only valid when can_overwrite_ is true
	 */
	bool overwrite(const umimage::UMImage& image);

	/**
	 * get texture id
	 */
	unsigned int texture_id() const;
	
	/**
	 * get render buffer id
	 */
	unsigned int render_buffer_id() const;

	/**
	 * get frame buffer id
	 */
	unsigned int frame_buffer_id() const;

	/**
	 * get frame buffer attachments
	 */
	const std::vector<unsigned int>& frame_buffer_attachments() const;

private:
	class Impl;
	typedef std::unique_ptr<Impl> ImplPtr;
	ImplPtr impl_;
};

} // umdraw
