/**
 * @file UMDirectX11Texture.h
 * a texture
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <dxgi.h>
#include <d3d11.h>
#include <string>
#include <memory>

#include "UMMacro.h"
#include "UMImageTypes.h"
#include "UMImage.h"

namespace umdraw
{
	
class UMDirectX11Texture;
typedef std::shared_ptr<UMDirectX11Texture> UMDirectX11TexturePtr;
typedef std::vector<UMDirectX11TexturePtr> UMDirectX11TextureList;

/**
 * a texture
 */
class UMDirectX11Texture
{
	DISALLOW_COPY_AND_ASSIGN(UMDirectX11Texture);

public:

	enum InternalFormat
	{
		eRGBA_UNORM,
	};

	struct Format
	{
		int internal_format;
		Format() : internal_format(eRGBA_UNORM) {}
	};

	/**
	 * constructor
	 * @param [in] can_overwrite if true,  can overwrite()
	 */
	explicit UMDirectX11Texture(bool can_overwrite);

	/**
	 * destructor
	 */
	~UMDirectX11Texture();
	
	/**
	 * get resource view pointer
	 */
	ID3D11ShaderResourceView* texture_view_pointer();

	/**
	 * get depth stencil view pointer
	 */
	ID3D11DepthStencilView* depth_stencil_view_pointer();
	
	/**
	 * get render target view pointer
	 */
	ID3D11RenderTargetView* render_target_view_pointer();

	/**
	 * create depth stencil view
	 * @param [in] device_pointer directx11 device pointer
	 * @param [in] width width
	 * @param [in] height height
	 * @param [in] format format
	 */
	static UMDirectX11TexturePtr create_texture(
		ID3D11Device *device_pointer, 
		int width, 
		int height, 
		const Format& format);

	/**
	 * create depth stencil view
	 * @param [in] device_pointer directx11 device pointer
	 * @param [in] width width
	 * @param [in] height height
	 * @retval UMDirectX11TexturePtr depth stencil view texture
	 */
	static UMDirectX11TexturePtr create_depth_stencil_texture(
		ID3D11Device *device_pointer, 
		int width, 
		int height);
	
	/**
	 * create depth stencil from back buffer
	 * @param [in] device_pointer directx11 device pointer
	 * @param [in] back_buffer back buffer
	 * @param [in] depth_stencil_view
	 * @retval UMDirectX11TexturePtr depth stencil view texture
	 */
	static UMDirectX11TexturePtr create_depth_stencil(
		ID3D11Device *device_pointer,
		ID3D11Texture2D* back_buffer);

	/**
	 * create render target for textures
	 * @param [in] device_pointer directx11 device pointer
	 * @param [in] textures
	 * @param [in] depth_stencil_view
	 * @retval UMDirectX11TexturePtr render target view texture
	 */
	static UMDirectX11TexturePtr create_render_target_texture(
		ID3D11Device *device_pointer,
		UMDirectX11TextureList& textures, 
		UMDirectX11TexturePtr depth_stencil_view);

	/**
	 * create render target from back buffer
	 * @param [in] device_pointer directx11 device pointer
	 * @param [in] back_buffer back buffer
	 * @param [in] depth_stencil_view
	 * @retval UMDirectX11TexturePtr render target view texture
	 */
	static UMDirectX11TexturePtr create_render_target(
		ID3D11Device *device_pointer,
		ID3D11Texture2D* back_buffer,
		UMDirectX11TexturePtr depth_stencil_view);

	/**
	 * get sampler state pointer
	 */
	ID3D11SamplerState* sampler_state_pointer();

	/**
	 * load an image file as a directx texture
	 * @param [in] device_pointer directx11 device pointer
	 * @param [in] file_path absolute texture file path
	 * @retval success or fail
	 */
	bool load(
		ID3D11Device *device_pointer,
		const std::u16string& file_path);

	/**
	 * convert UMImage to DirectX11 Texture
	 * @param [in] device_pointer  directx11 device pointer
	 * @param [in] image source image
	 * @retval success or fail
	 */
	bool convert_from_image(
		ID3D11Device *device_pointer,
		const umimage::UMImage& image);

	/**
	 * convert DirectX texture buffer to memory
	 * @param [in] device_pointer  directx11 device pointer
	 * @param [in] device_context_pointer directx11 device pointer context
	 * @retval UMImagePtr converted image buffer
	 */
	umimage::UMImagePtr convert_to_image(
		ID3D11Device *device_pointer,
		ID3D11DeviceContext *device_context_pointer) const;

	/**
	 * overwrite texture by image
	 * @info only valid when can_overwrite_ is true
	 */
	bool overwrite(
		ID3D11Device *device_pointer,
		ID3D11DeviceContext *device_context_pointer,
		const UMImage& image);

private:

	ID3D11Texture2D* texture_2d() const;

	class Impl;
	typedef std::unique_ptr<Impl> ImplPtr;
	ImplPtr impl_;
};

} // umdraw
