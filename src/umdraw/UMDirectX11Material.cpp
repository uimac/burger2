/**
 * @file UMDirectX11Material.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_DIRECTX

#include "UMDirectX11Material.h"
#include "UMDirectX11Texture.h"

namespace
{
	using namespace umdraw;

	UMVec2f to_dx(const UMVec2d& v) { return UMVec2f((float)v.x, (float)v.y); }

	UMVec3f to_dx(const UMVec3d& v) { return UMVec3f((float)v.x, (float)v.y, (float)v.z); }

	UMVec4f to_dx(const UMVec4d& v) { return UMVec4f((float)v.x, (float)v.y, (float)v.z, (float)v.w); } 

} // anonymouse namespace

namespace umdraw
{

UMDirectX11Material::~UMDirectX11Material()
{
}

/**
 * get polygon count
 */
int UMDirectX11Material::polygon_count() const
{
	if (UMMaterialPtr material = ummaterial())
	{
		return material->polygon_count();
	}
	return 0;
}

/** 
 * initialize
 */
bool UMDirectX11Material::init(ID3D11Device *device_pointer)
{
	if (!device_pointer) return false;
	
	ID3D11DeviceContext* device_context_pointer(NULL);
	device_pointer->GetImmediateContext(&device_context_pointer);

	if (UMMaterialPtr material = ummaterial())
	{
		set_ambient(to_dx(material->ambient()));
		set_diffuse(to_dx(material->diffuse()));
		set_specular(to_dx(material->specular()));

		UMMaterial::TexturePathList::const_iterator it = material->texture_path_list().begin();
		for (; it != material->texture_path_list().end(); ++it)
		{
			const umstring& path = *it;
			if (diffuse_texture_->load(device_pointer, path))
			{
				set_shader_flags(UMVec4f(1.0f, 0.0f, 0.0f, 0.0f));
			}
		}
		if (material->texture_path_list().empty())
		{
			UMMaterial::TextureList::const_iterator it =  material->texture_list().begin();
			for (; it != material->texture_list().end(); ++it)
			{
				if (diffuse_texture_->convert_from_image(device_pointer, *(*it)))
				{
					shader_flags_.x = 1.0;
				}
			}
		}
	}
	
	SAFE_RELEASE(device_context_pointer);

	return true;
}

/** 
 * update
 */
bool UMDirectX11Material::update(ID3D11Device *device_pointer)
{
	if (!device_pointer) return false;
	
	UMMaterialPtr material = ummaterial();
	if (!material) return false;

	return true;
}

/**
 * draw
 */
void UMDirectX11Material::draw(ID3D11Device* device_pointer)
{
}

} // umdraw

#endif // WITH_DIRECTX
