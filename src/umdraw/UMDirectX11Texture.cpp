/**
 * @file UMDirectX11Texture.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_DIRECTX

#include "UMDirectX11Texture.h"

#include <d3dx11.h>
#include <map>

#include "UMVector.h"
#include "UMStringUtil.h"
#include "UMListener.h"
#include "UMListenerConnector.h"
#include "UMImageEventType.h"

namespace
{
	struct DirectX11TextureResource
	{
		ID3D11ShaderResourceView* resource_view;
		ID3D11DepthStencilView* depth_stencil_view;
		ID3D11RenderTargetView* render_target_view;
		ID3D11SamplerState* sampler_state_pointer;
		ID3D11Texture2D* texture_2d;
	};
	typedef std::map<std::u16string, DirectX11TextureResource > UMDirectX11TexturePool;
	UMDirectX11TexturePool texture_pool;

	typedef std::map<std::u16string, umimage::UMImagePtr> UMDirectX11TextureImagePool;
	UMDirectX11TextureImagePool texture_image_pool;
};

namespace umdraw
{
	
class UMDirectX11TextureListener;
typedef std::shared_ptr<UMDirectX11TextureListener> UMDirectX11TextureListenerPtr;
class UMDirectX11TextureListener : public umbase::UMListener
{
	DISALLOW_COPY_AND_ASSIGN(UMDirectX11TextureListener);
public:
	UMDirectX11TextureListener()
		: texture_id_(0)
	{}

	~UMDirectX11TextureListener()
	{}

	void set_texture_id(unsigned int texture_id) { texture_id_ = texture_id; }

	unsigned int texture_id() const { return texture_id_; }

	virtual void update(umbase::UMEventType event_type, umbase::UMAny& parameter) 
	{
		if (event_type == umimage::eImageEventImageChaged)
		{
			// TODO
		}
	}
private:
	unsigned int texture_id_;
};

class UMDirectX11Texture::Impl : public umbase::UMListenerConnector
{
	DISALLOW_COPY_AND_ASSIGN(Impl);
public:
	
	Impl(bool can_overwrite) 
		: can_overwrite_(can_overwrite)
		, resource_view_pointer_(NULL)
		, sampler_state_pointer_(NULL)
		, depth_stencil_view_pointer_(NULL)
		, render_target_view_pointer_(NULL)
		, texture_2d_(NULL)
		, listener_(new UMDirectX11TextureListener())
	{
	}

	~Impl() 
	{
		// erase texture from pool
		{
			UMDirectX11TexturePool::iterator it = texture_pool.find(id_);
			if (it != texture_pool.end())
			{
				texture_pool.erase(it);
				SAFE_RELEASE(resource_view_pointer_);
				SAFE_RELEASE(depth_stencil_view_pointer_);
				SAFE_RELEASE(render_target_view_pointer_);
				SAFE_RELEASE(sampler_state_pointer_);
				SAFE_RELEASE(texture_2d_);
			}
		}
		// erase image from pool
		{
			UMDirectX11TextureImagePool::iterator it = texture_image_pool.find(id_);
			if (it != texture_image_pool.end())
			{
				texture_image_pool.erase(it);
			}
		}
	}

	bool load(ID3D11Device *device_pointer, const std::u16string& file_path)
	{
		if (!device_pointer) return false;
		if (sampler_state_pointer_) return false;
		if (texture_2d_) return false;
	
		// find texture from pool
		if (texture_pool.find(file_path) != texture_pool.end())
		{
			DirectX11TextureResource src_resource = texture_pool[file_path];
			resource_view_pointer_ = src_resource.resource_view;
			sampler_state_pointer_ = src_resource.sampler_state_pointer;
			texture_2d_ = src_resource.texture_2d;
			is_valid_texture_ = true;
			id_ = file_path;
			return true;
		}

		if FAILED(D3DX11CreateShaderResourceViewFromFileW(
			device_pointer,
			umbase::UMStringUtil::utf16_to_wstring(file_path).c_str(),
			NULL,
			NULL,
			&resource_view_pointer_,
			NULL))
		{
			resource_view_pointer_ = NULL;
			return false;
		}

		// connect event
		if (image_)
		{
			mutable_event_list().clear();
			mutable_event_list().push_back(image_->image_change_event());
			listener_list().clear();
			connect(listener_);
		}
	
		ID3D11DeviceContext* device_context(NULL);
		device_pointer->GetImmediateContext(&device_context);
	
		create_default_sampler_state(device_pointer, device_context);

		if (can_overwrite_)
		{
			UMImagePtr image = convert_to_image(device_pointer, device_context);
			if (image)
			{
				UMImagePtr fliped = image->create_flip_image(false, true);
				if (fliped)
				{
					SAFE_RELEASE(resource_view_pointer_);
					convert_from_image(device_pointer, *fliped);
				}
			}
		}

		id_ = file_path;
		DirectX11TextureResource resource;
		resource.resource_view = resource_view_pointer_;
		resource.sampler_state_pointer = sampler_state_pointer_;
		resource.texture_2d = texture_2d_;
		texture_pool[file_path] = resource;
		texture_image_pool[id_] = image_;
		is_valid_texture_ = true;

		SAFE_RELEASE(device_context);

		return true;
	}

	bool convert_from_image(ID3D11Device *device_pointer, const UMImage& image)
	{
		if (sampler_state_pointer_) return false;
		if (texture_2d_) return false;
		if (!image.is_valid()) return false;
	
		// find texture from pool
		std::u16string id = 
			umbase::UMStringUtil::utf8_to_utf16(umbase::UMStringUtil::number_to_string(image.id()));
		if (texture_pool.find(id) != texture_pool.end())
		{
			DirectX11TextureResource src_resource = texture_pool[id];
			resource_view_pointer_ = src_resource.resource_view;
			sampler_state_pointer_ = src_resource.sampler_state_pointer;
			texture_2d_ = src_resource.texture_2d;
			is_valid_texture_ = true;
			id_ = id;
			return true;
		}

		UMImagePtr filped = image.create_flip_image(false, true);
		if (!filped) return false;

		UMImage::R8G8B8A8Buffer buffer;
		filped->create_r8g8b8a8_buffer(buffer);

		// create texture
		D3D11_TEXTURE2D_DESC texture_desc = create_texture_desc(image.width(), image.height(), Format());
		D3D11_SUBRESOURCE_DATA  data;
		::ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
		data.pSysMem = &buffer.front();
		data.SysMemPitch = image.width() * 4;

		if FAILED(device_pointer->CreateTexture2D(
			&texture_desc, 
			&data, 
			&texture_2d_))
		{
			return false;
		}
		if FAILED(device_pointer->CreateShaderResourceView(
			texture_2d_, 
			NULL, 
			&resource_view_pointer_))
		{
			return false;
		}
	
		ID3D11DeviceContext* device_context(NULL);
		device_pointer->GetImmediateContext(&device_context);

		// create default sampler state
		if (!sampler_state_pointer_)
		{
			if (!create_default_sampler_state(device_pointer, device_context))
			{
				return false;
			}
		}
		SAFE_RELEASE(device_context);
	
		id_ = id;
		DirectX11TextureResource resource;
		resource.resource_view = resource_view_pointer_;
		resource.sampler_state_pointer = sampler_state_pointer_;
		texture_pool[id_] = resource;
		texture_image_pool[id_] = image_;
		is_valid_texture_ = true;

		return true;
	}
	
	UMImagePtr convert_to_image(ID3D11Device *device_pointer, ID3D11DeviceContext *device_context_pointer) const
	{
		if (!device_pointer) return UMImagePtr();
		if (!device_context_pointer) return UMImagePtr();
		if (!resource_view_pointer_) return UMImagePtr();

		// simple 2d texture ?
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		resource_view_pointer_->GetDesc(&desc);
		if (desc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D) {
			return UMImagePtr();
		}

		// validate format
		if (desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM ||
			desc.Format == DXGI_FORMAT_R8G8B8A8_UINT)
		{
			// get original texture resrouce
			ID3D11Resource * texture = NULL;
			resource_view_pointer_->GetResource(&texture);

			// get original texture2d desc
			D3D11_TEXTURE2D_DESC  texture2d_desc;
			if (ID3D11Texture2D* texture2d = static_cast<ID3D11Texture2D*>(texture))
			{
				texture2d->GetDesc(&texture2d_desc);
			}

			// create readable copy
			D3D11_TEXTURE2D_DESC  copy_desc;
			ZeroMemory(&copy_desc, sizeof(copy_desc));
			copy_desc.Width = texture2d_desc.Width;
			copy_desc.Height = texture2d_desc.Height;
			copy_desc.MipLevels = texture2d_desc.MipLevels;
			copy_desc.ArraySize  = texture2d_desc.ArraySize;
			copy_desc.Format = texture2d_desc.Format;
			copy_desc.SampleDesc = texture2d_desc.SampleDesc;
			copy_desc.Usage = D3D11_USAGE_STAGING;
			copy_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			ID3D11Texture2D* copy_texture = NULL;
			if FAILED(device_pointer->CreateTexture2D(&copy_desc, NULL, &copy_texture))
			{
				return false;
			}
			device_context_pointer->CopyResource(copy_texture, texture);
		
			// release original
			if (texture) { SAFE_RELEASE(texture); }

			// map texture
			D3D11_MAPPED_SUBRESOURCE subresource;
			if FAILED(device_context_pointer->Map(copy_texture, 0, D3D11_MAP_READ, 0, &subresource))
			{
				if (copy_texture)  { SAFE_RELEASE(copy_texture); }
				return false;
			}

			// create buffer and read pixels
			UMImagePtr image(std::make_shared<UMImage>());
			image->set_width(texture2d_desc.Width);
			image->set_height(texture2d_desc.Height);
			image->mutable_list().resize(image->width() * image->height());
			const double ffinv = 1.0 / static_cast<double>(0xFF);
			unsigned char* data_pointer = reinterpret_cast<unsigned char*>(subresource.pData);
			for (int y = 0, height = image->height(); y < height; ++y)
			{
				for (int x = 0, width = image->width(); x < width; ++x)
				{
					const int pos = y * width * 4 + x * 4;
					if (desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM)
					{
						int r = data_pointer[ pos + 0 ];
						int g = data_pointer[ pos + 1 ];
						int b = data_pointer[ pos + 2 ];
						int a = data_pointer[ pos + 3 ];
						UMVec4d& dst = image->mutable_list()[ y * width + x ];
						dst.x = r*ffinv;
						dst.y = g*ffinv;
						dst.z = b*ffinv;
						dst.w = a*ffinv;
					}
				}
			}

			// unmap texture
			device_context_pointer->Unmap(copy_texture, 0);
			if (copy_texture) { SAFE_RELEASE(copy_texture); }

			return image;
		}
		return UMImagePtr();
	}

	bool overwrite(
		ID3D11Device *device_pointer,
		ID3D11DeviceContext *device_context_pointer,
		const UMImage& image)
	{
		if (!can_overwrite_) return false;
		if (!device_pointer) return false;
		if (!device_context_pointer) return false;
		if (!resource_view_pointer_) return false;

		// simple 2d texture ?
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		resource_view_pointer_->GetDesc(&desc);
		if (desc.ViewDimension != D3D11_SRV_DIMENSION_TEXTURE2D) {
			return false;
		}

		// validate format
		if (desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM ||
			desc.Format == DXGI_FORMAT_R8G8B8A8_UINT)
		{
			// get original texture resrouce
			ID3D11Resource * texture = texture_2d_;
			if (!texture_2d_)
			{
				resource_view_pointer_->GetResource(&texture);
			}
			// map texture
			D3D11_MAPPED_SUBRESOURCE subresource;
			if FAILED(device_context_pointer->Map(texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource))
			{
				if (texture)  { SAFE_RELEASE(texture); }
				return false;
			}
			const double inv_gamma = 1.0 / 2.2;
			unsigned char* data_pointer = reinterpret_cast<unsigned char*>(subresource.pData);
			for (int y = 0, height = image.height(); y < height; ++y)
			{
				for (int x = 0, width = image.width(); x < width; ++x)
				{
					const int pos = y * width * 4 + x * 4;
					if (desc.Format == DXGI_FORMAT_R8G8B8A8_UNORM)
					{
						const UMVec4d& color = image.list()[ (height-y-1) * width + x ];
						data_pointer[ pos + 0 ] = static_cast<int>(pow(color.x, inv_gamma) * 0xFF + 0.5);
						data_pointer[ pos + 1 ] = static_cast<int>(pow(color.y, inv_gamma) * 0xFF + 0.5);
						data_pointer[ pos + 2 ] = static_cast<int>(pow(color.z, inv_gamma) * 0xFF + 0.5);
						data_pointer[ pos + 3 ] = static_cast<int>(pow(color.w, inv_gamma) * 0xFF + 0.5);
					}
				}
			}
			// unmap texture
			device_context_pointer->Unmap(texture, 0);
			if (!texture_2d_)
			{
				SAFE_RELEASE(texture);
			}
			return true;
		}
		return false;
	}
	
	ID3D11ShaderResourceView* texture_view_pointer()
	{
		return resource_view_pointer_;
	}
	
	ID3D11DepthStencilView* depth_stencil_view_pointer()
	{
		return depth_stencil_view_pointer_;
	}

	ID3D11RenderTargetView* render_target_view_pointer()
	{
		return render_target_view_pointer_;
	}

	ID3D11SamplerState* sampler_state_pointer() {
		return sampler_state_pointer_; 
	}
	
	ID3D11Texture2D* texture_2d() const
	{
		return texture_2d_;
	}

	bool create_texture(
		ID3D11Device *device_pointer,
		int width,
		int height,
		const Format& format)
	{
		if (!device_pointer) return false;
		if (sampler_state_pointer_) return false;
		if (texture_2d_) return false;
	
		static int image_id = 100000;
		++image_id;

		// find texture from pool
		std::u16string id = umbase::UMStringUtil::utf8_to_utf16(
			umbase::UMStringUtil::number_to_string(image_id));

		// find texture from pool
		if (texture_pool.find(id) != texture_pool.end())
		{
			DirectX11TextureResource src_resource = texture_pool[id];
			resource_view_pointer_ = src_resource.resource_view;
			sampler_state_pointer_ = src_resource.sampler_state_pointer;
			texture_2d_ = src_resource.texture_2d;
			is_valid_texture_ = true;
			id_ = id;
			return true;
		}

		// create texture
		D3D11_TEXTURE2D_DESC texture_desc = create_texture_desc(width, height, format);
		D3D11_SUBRESOURCE_DATA  data;
		::ZeroMemory(&data, sizeof(D3D11_SUBRESOURCE_DATA));
		data.SysMemPitch = width * 4;

		if FAILED(device_pointer->CreateTexture2D(
			&texture_desc, 
			&data, 
			&texture_2d_))
		{
			return false;
		}
		if FAILED(device_pointer->CreateShaderResourceView(
			texture_2d_, 
			NULL, 
			&resource_view_pointer_))
		{
			return false;
		}
	
		ID3D11DeviceContext* device_context(NULL);
		device_pointer->GetImmediateContext(&device_context);

		// create default sampler state
		if (!sampler_state_pointer_)
		{
			if (!create_default_sampler_state(device_pointer, device_context))
			{
				return false;
			}
		}
		SAFE_RELEASE(device_context);

		connect_event();

		id_ = id;
		DirectX11TextureResource resource;
		resource.resource_view = resource_view_pointer_;
		resource.sampler_state_pointer = sampler_state_pointer_;
		texture_pool[id_] = resource;
		texture_image_pool[id_] = image_;
		is_valid_texture_ = true;
		return true;
	}

	bool create_depth_stencil_texture(
		ID3D11Device *device_pointer,
		int width,
		int height)
	{
		if (!device_pointer) return false;
		if (depth_stencil_view_pointer_) return false;
		if (texture_2d_) return false;
	
		static int image_id = 200000;
		++image_id;

		// find texture from pool
		std::u16string id = umbase::UMStringUtil::utf8_to_utf16(
			umbase::UMStringUtil::number_to_string(image_id));

		// find texture from pool
		if (texture_pool.find(id) != texture_pool.end())
		{
			DirectX11TextureResource src_resource = texture_pool[id];
			depth_stencil_view_pointer_ = src_resource.depth_stencil_view;
			sampler_state_pointer_ = src_resource.sampler_state_pointer;
			texture_2d_ = src_resource.texture_2d;
			is_valid_texture_ = true;
			id_ = id;
			return true;
		}

		// create texture
		Format format;
		format.internal_format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		D3D11_TEXTURE2D_DESC texture_desc = create_depth_stencil_texture_desc(width, height, format);

		if FAILED(device_pointer->CreateTexture2D(
			&texture_desc, 
			NULL,
			&texture_2d_))
		{
			return false;
		}
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC depth_view_desc;
			depth_view_desc.Format = static_cast<DXGI_FORMAT>(format.internal_format);
			depth_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			depth_view_desc.Flags = 0;
			depth_view_desc.Texture2D.MipSlice = 0;
			if FAILED(device_pointer->CreateDepthStencilView(
				texture_2d_, 
				&depth_view_desc,
				&depth_stencil_view_pointer_))
			{
				return false;
			}
		}
	
		ID3D11DeviceContext* device_context(NULL);
		device_pointer->GetImmediateContext(&device_context);

		// create default sampler state
		if (!sampler_state_pointer_)
		{
			if (!create_default_sampler_state(device_pointer, device_context))
			{
				return false;
			}
		}
		SAFE_RELEASE(device_context);

		connect_event();
		
		id_ = id;
		DirectX11TextureResource resource;
		resource.depth_stencil_view = depth_stencil_view_pointer_;
		resource.sampler_state_pointer = sampler_state_pointer_;
		texture_pool[id_] = resource;
		texture_image_pool[id_] = image_;
		is_valid_texture_ = true;
		return true;
	}
	
	bool create_depth_stencil(
		ID3D11Device *device_pointer,
		ID3D11Texture2D* back_buffer)
	{
		if (!device_pointer) return false;
		if (depth_stencil_view_pointer_) return false;
		if (texture_2d_) return false;
	
		static int image_id = 250000;
		++image_id;

		// find texture from pool
		std::u16string id = umbase::UMStringUtil::utf8_to_utf16(
			umbase::UMStringUtil::number_to_string(image_id));

		// find texture from pool
		if (texture_pool.find(id) != texture_pool.end())
		{
			DirectX11TextureResource src_resource = texture_pool[id];
			depth_stencil_view_pointer_ = src_resource.depth_stencil_view;
			sampler_state_pointer_ = src_resource.sampler_state_pointer;
			texture_2d_ = src_resource.texture_2d;
			is_valid_texture_ = true;
			id_ = id;
			return true;
		}

		// create texture
		Format format;
		format.internal_format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		D3D11_TEXTURE2D_DESC texture_desc;
		back_buffer->GetDesc(&texture_desc);
		texture_desc.MipLevels = 1;
		texture_desc.ArraySize = 1;
		texture_desc.Format = DXGI_FORMAT_D32_FLOAT;
		texture_desc.Usage = D3D11_USAGE_DEFAULT;
		texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		texture_desc.CPUAccessFlags = 0; // don't access from cpu
		texture_desc.MiscFlags = 0;

		if FAILED(device_pointer->CreateTexture2D(
			&texture_desc, 
			NULL,
			&texture_2d_))
		{
			return false;
		}
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC depth_view_desc;
			depth_view_desc.Format = texture_desc.Format;
			depth_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			depth_view_desc.Flags = 0;
			depth_view_desc.Texture2D.MipSlice = 0;
			if FAILED(device_pointer->CreateDepthStencilView(
				texture_2d_, 
				&depth_view_desc,
				&depth_stencil_view_pointer_))
			{
				return false;
			}
		}
	
		ID3D11DeviceContext* device_context(NULL);
		device_pointer->GetImmediateContext(&device_context);

		// create default sampler state
		if (!sampler_state_pointer_)
		{
			if (!create_default_sampler_state(device_pointer, device_context))
			{
				return false;
			}
		}
		SAFE_RELEASE(device_context);

		connect_event();
		
		id_ = id;
		DirectX11TextureResource resource;
		resource.depth_stencil_view = depth_stencil_view_pointer_;
		resource.sampler_state_pointer = sampler_state_pointer_;
		texture_pool[id_] = resource;
		texture_image_pool[id_] = image_;
		is_valid_texture_ = true;
		return true;
	}

	bool create_render_target_texture(
		ID3D11Device *device_pointer, 
		UMDirectX11TextureList& textures, 
		UMDirectX11TexturePtr depth_stencil_view)
	{
		if (!device_pointer) return false;
		if (sampler_state_pointer_) return false;
		if (texture_2d_) return false;
		if (!depth_stencil_view) return false;
	
		static int image_id = 300000;
		++image_id;

		// find texture from pool
		std::u16string id = umbase::UMStringUtil::utf8_to_utf16(
			umbase::UMStringUtil::number_to_string(image_id));

		// find texture from pool
		if (texture_pool.find(id) != texture_pool.end())
		{
			DirectX11TextureResource src_resource = texture_pool[id];
			render_target_view_pointer_ = src_resource.render_target_view;
			sampler_state_pointer_ = src_resource.sampler_state_pointer;
			texture_2d_ = src_resource.texture_2d;
			is_valid_texture_ = true;
			id_ = id;
			return true;
		}
		
		D3D11_TEXTURE2D_DESC  texture2d_desc;
		if (depth_stencil_view->texture_2d())
		{
			depth_stencil_view->texture_2d()->GetDesc(&texture2d_desc);
		}
		Format format;
		format.internal_format = DXGI_FORMAT_R32_FLOAT;

		// create texture
		D3D11_TEXTURE2D_DESC texture_desc = create_render_target_texture_desc(texture2d_desc.Width, texture2d_desc.Height, format);
		
		if FAILED(device_pointer->CreateTexture2D(
			&texture_desc, 
			NULL,
			&texture_2d_))
		{
			return false;
		}

		unsigned int array_size = 1;
		if (!textures.empty())
		{
			array_size = static_cast<unsigned int>(textures.size());
		}
		D3D11_RENDER_TARGET_VIEW_DESC render_target_desc = create_render_target_desc(array_size, format);

		if FAILED(device_pointer->CreateRenderTargetView(
			texture_2d_, 
			&render_target_desc, 
			&render_target_view_pointer_))
		{
			return false;
		}
	
		ID3D11DeviceContext* device_context(NULL);
		device_pointer->GetImmediateContext(&device_context);

		// create default sampler state
		if (!sampler_state_pointer_)
		{
			if (!create_default_sampler_state(device_pointer, device_context))
			{
				return false;
			}
		}
		SAFE_RELEASE(device_context);

		connect_event();
		
		id_ = id;
		DirectX11TextureResource resource;
		resource.render_target_view = render_target_view_pointer_;
		resource.sampler_state_pointer = sampler_state_pointer_;
		texture_pool[id_] = resource;
		texture_image_pool[id_] = image_;
		is_valid_texture_ = true;
		return true;
	}

	bool create_render_target(
		ID3D11Device *device_pointer,
		ID3D11Texture2D* back_buffer,
		UMDirectX11TexturePtr depth_stencil_view)
	{
		if (!device_pointer) return false;
		if (sampler_state_pointer_) return false;
		if (texture_2d_) return false;
		if (!depth_stencil_view) return false;
	
		static int image_id = 400000;
		++image_id;

		// find texture from pool
		std::u16string id = umbase::UMStringUtil::utf8_to_utf16(
			umbase::UMStringUtil::number_to_string(image_id));

		// find texture from pool
		if (texture_pool.find(id) != texture_pool.end())
		{
			DirectX11TextureResource src_resource = texture_pool[id];
			render_target_view_pointer_ = src_resource.render_target_view;
			sampler_state_pointer_ = src_resource.sampler_state_pointer;
			texture_2d_ = src_resource.texture_2d;
			is_valid_texture_ = true;
			id_ = id;
			return true;
		}

		if FAILED(device_pointer->CreateRenderTargetView(
			back_buffer, 
			NULL,
			&render_target_view_pointer_))
		{
			return false;
		}
	
		ID3D11DeviceContext* device_context(NULL);
		device_pointer->GetImmediateContext(&device_context);

		// create default sampler state
		if (!sampler_state_pointer_)
		{
			if (!create_default_sampler_state(device_pointer, device_context))
			{
				return false;
			}
		}
		SAFE_RELEASE(device_context);

		connect_event();
		
		id_ = id;
		DirectX11TextureResource resource;
		resource.render_target_view = render_target_view_pointer_;
		resource.sampler_state_pointer = sampler_state_pointer_;
		texture_pool[id_] = resource;
		texture_image_pool[id_] = image_;
		is_valid_texture_ = true;
		return true;
	}

private:
	// connect event
	void connect_event()
	{
		;
		//if (!image_) return;
		//mutable_event_list().clear();
		//mutable_event_list().push_back(image_->image_change_event());
		//listener_list().clear();
		//listener_->set_image(image_);
		//listener_->set_texture_id(id_);
		//connect(listener_);
	}

	bool create_default_sampler_state(ID3D11Device *device_pointer, ID3D11DeviceContext* device_context)
	{
		D3D11_SAMPLER_DESC sampler_desc;
		::ZeroMemory(&sampler_desc, sizeof(D3D11_SAMPLER_DESC));
		sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_desc.MipLODBias = 0;
		sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		sampler_desc.MinLOD = 0;
		sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

		if FAILED(device_pointer->CreateSamplerState(
			&sampler_desc, 
			&sampler_state_pointer_ ))
		{
			SAFE_RELEASE(device_context);
			return false;
		}
		return true;
	}

	DXGI_FORMAT convert_format(const Format& format)
	{
		if (format.internal_format == eRGBA_UNORM)
		{
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		}
		else
		{
			return static_cast<DXGI_FORMAT>(format.internal_format);
		}
	}

	D3D11_TEXTURE2D_DESC create_texture_desc(int width, int height, const Format& format)
	{
		D3D11_TEXTURE2D_DESC texture_desc;
		::ZeroMemory(&texture_desc, sizeof(D3D11_TEXTURE2D_DESC));
		texture_desc.Width = width;
		texture_desc.Height = height;
		texture_desc.MipLevels = 1;
		texture_desc.ArraySize = 1;
		texture_desc.Format = convert_format(format);
		texture_desc.SampleDesc.Count   = 1;
		texture_desc.SampleDesc.Quality = 0;
		if (can_overwrite_)
		{
			texture_desc.Usage = D3D11_USAGE_DYNAMIC;
			texture_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		else
		{
			texture_desc.Usage = D3D11_USAGE_DEFAULT;
			texture_desc.CPUAccessFlags = 0; // don't access from cpu
		}
		texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texture_desc.MiscFlags = 0;
		return texture_desc;
	}
	
	D3D11_TEXTURE2D_DESC create_depth_stencil_texture_desc(int width, int height, const Format& format)
	{
		D3D11_TEXTURE2D_DESC desc = create_texture_desc(width, height, format);
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		return desc;
	}
	
	D3D11_TEXTURE2D_DESC create_render_target_texture_desc(int width, int height, const Format& format)
	{
		D3D11_TEXTURE2D_DESC desc = create_texture_desc(width, height, format);
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.MipLevels = 0;
		return desc;
	}
	
	
	D3D11_RENDER_TARGET_VIEW_DESC create_render_target_desc(unsigned int array_size, const Format& format)
	{
		D3D11_RENDER_TARGET_VIEW_DESC desc;
		desc.Format = convert_format(format);
		desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.FirstArraySlice = 0;
		desc.Texture2DArray.ArraySize = array_size;
		desc.Texture2DArray.MipSlice = 0;
		return desc;
	}

	UMDirectX11TextureListenerPtr listener_;
	umimage::UMImagePtr image_;
	bool can_overwrite_;
	ID3D11ShaderResourceView* resource_view_pointer_;
	ID3D11DepthStencilView* depth_stencil_view_pointer_;
	ID3D11RenderTargetView* render_target_view_pointer_;
	ID3D11SamplerState * sampler_state_pointer_;
	ID3D11Texture2D * texture_2d_;
	bool is_valid_texture_;
	std::u16string id_;
};

/**
 * create depth stencil view
 */
UMDirectX11TexturePtr UMDirectX11Texture::create_texture(ID3D11Device *device_pointer, int width, int height, const Format& format)
{
	UMDirectX11TexturePtr texture = std::make_shared<UMDirectX11Texture>(false);
	if (texture && texture->impl_->create_texture(device_pointer, width, height, format))
	{
		return texture;
	}
	return UMDirectX11TexturePtr();
}

/**
 * create depth stencil view
 */
UMDirectX11TexturePtr UMDirectX11Texture::create_depth_stencil_texture(ID3D11Device *device_pointer, int width, int height)
{
	UMDirectX11TexturePtr texture = std::make_shared<UMDirectX11Texture>(false);
	if (texture && texture->impl_->create_depth_stencil_texture(device_pointer, width, height))
	{
		return texture;
	}
	return UMDirectX11TexturePtr();
}

/**
 * create render target view
 */
UMDirectX11TexturePtr UMDirectX11Texture::create_render_target_texture(
	ID3D11Device *device_pointer,
	UMDirectX11TextureList& textures, 
	UMDirectX11TexturePtr depth_stencil_view)
{
	UMDirectX11TexturePtr texture = std::make_shared<UMDirectX11Texture>(false);
	if (texture)
	{
		if (texture->impl_->create_render_target_texture(device_pointer, textures, depth_stencil_view))
		{
			return texture;
		}
	}
	return UMDirectX11TexturePtr();
}

/**
 * create depth stencil
 */
UMDirectX11TexturePtr UMDirectX11Texture::create_depth_stencil(
	ID3D11Device *device_pointer,
	ID3D11Texture2D* back_buffer)
{
	UMDirectX11TexturePtr texture = std::make_shared<UMDirectX11Texture>(false);
	if (texture && texture->impl_->create_depth_stencil(device_pointer, back_buffer))
	{
		return texture;
	}
	return UMDirectX11TexturePtr();
}

/**
 * create render target view
 */
UMDirectX11TexturePtr UMDirectX11Texture::create_render_target(
	ID3D11Device *device_pointer,
	ID3D11Texture2D* back_buffer,
	UMDirectX11TexturePtr depth_stencil_view)
{
	UMDirectX11TexturePtr texture = std::make_shared<UMDirectX11Texture>(false);
	if (texture)
	{
		if (texture->impl_->create_render_target(device_pointer, back_buffer, depth_stencil_view))
		{
			return texture;
		}
	}
	return UMDirectX11TexturePtr();
}

/**
 * constructor
 */
UMDirectX11Texture::UMDirectX11Texture(bool can_overwrite)
	: impl_(new UMDirectX11Texture::Impl(can_overwrite))
{
}

/**
 * destructor
 */
UMDirectX11Texture::~UMDirectX11Texture()
{
}

/**
 * load an image file as a directx texture
 * @param [in] device_pointer directx11 device pointer
 * @param [in] file_path absolute texture file path
 */
bool UMDirectX11Texture::load(
		ID3D11Device *device_pointer,
		const std::u16string& file_path)
{
	return impl_->load(device_pointer, file_path);
}

/**
 * convert UMImage to DirectX11 Texture
 * @param [in] image source image
 * @retval success or fail
s */
bool UMDirectX11Texture::convert_from_image(
	ID3D11Device *device_pointer,
	const UMImage& image)
{
	return impl_->convert_from_image(device_pointer, image);
}

/**
 * convert DirectX texture buffer to memory
 * @param [in] device_pointer directx11 device pointer
 * @retval UMImagePtr converted image buffer
 */
UMImagePtr UMDirectX11Texture::convert_to_image(
	ID3D11Device *device_pointer,
	ID3D11DeviceContext *device_context_pointer) const
{
	return impl_->convert_to_image(device_pointer, device_context_pointer);
}


/**
 * overwrite texture by image
 * @info only valid when can_overwrite_ is true
 */
bool UMDirectX11Texture::overwrite(
	ID3D11Device *device_pointer,
	ID3D11DeviceContext *device_context_pointer,
	const UMImage& image)
{
	return impl_->overwrite(device_pointer, device_context_pointer, image);
}

/**
 * get resource view pointer
 */
ID3D11ShaderResourceView* UMDirectX11Texture::texture_view_pointer()
{
	return impl_->texture_view_pointer();
}

/**
 * get resource view pointer
 */
ID3D11DepthStencilView* UMDirectX11Texture::depth_stencil_view_pointer()
{
	return impl_->depth_stencil_view_pointer();
}

/**
 * get resource view pointer
 */
ID3D11RenderTargetView* UMDirectX11Texture::render_target_view_pointer()
{
	return impl_->render_target_view_pointer();
}

/**
 * get sampler state pointer
 */
ID3D11SamplerState* UMDirectX11Texture::sampler_state_pointer()
{
	return impl_->sampler_state_pointer();
}

/**
 * get internal texture 2d pointer
 */
ID3D11Texture2D* UMDirectX11Texture::texture_2d() const
{
	return impl_->texture_2d();
}

} // umdraw

#endif // WITH_DIRECTX
