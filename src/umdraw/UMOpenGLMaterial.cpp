/**
 * @file UMOpenGLMaterial.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_OPENGL

#include "UMOpenGLMaterial.h"
#include "UMOpenGLTexture.h"
#include "UMMaterial.h"

namespace
{
	using namespace umdraw;

	UMVec2f to_gl(const UMVec2d& v) { return UMVec2f((float)v.x, (float)v.y); }

	UMVec3f to_gl(const UMVec3d& v) { return UMVec3f((float)v.x, (float)v.y, (float)v.z); }

	UMVec4f to_gl(const UMVec4d& v) { return UMVec4f((float)v.x, (float)v.y, (float)v.z, (float)v.w); } 

} // anonymouse namespace

namespace umdraw
{

class UMOpenGLMaterial::Impl
{
	DISALLOW_COPY_AND_ASSIGN(Impl);
public:

	Impl(UMMaterialPtr ummaterial) 
		: ummaterial_(ummaterial)
		, polygon_count_(0)
		, diffuse_texture_(std::make_shared<UMOpenGLTexture>(false))
	{}

	~Impl() {}

	bool init()
	{
		if (UMMaterialPtr material = ummaterial())
		{
			set_ambient(to_gl(material->ambient()));
			set_diffuse(to_gl(material->diffuse()));
			set_specular(to_gl(material->specular()));
			set_polygon_count(material->polygon_count());

			UMMaterial::TexturePathList::const_iterator it = material->texture_path_list().begin();
			for (; it != material->texture_path_list().end(); ++it)
			{
				const umstring& path = *it;
				if (diffuse_texture_->load(path))
				{
					if (UMImagePtr image = diffuse_texture_->convert_to_image())
					{
						material->mutable_texture_list().push_back(image);
					}
					shader_flags_.x = 1.0;
				}
			}
			if (material->texture_path_list().empty())
			{
				UMMaterial::TextureList::const_iterator it =  material->texture_list().begin();
				for (; it != material->texture_list().end(); ++it)
				{
					if (diffuse_texture_->convert_from_image(*(*it)))
					{
						shader_flags_.x = 1.0;
					}
				}
			}
		}

		return true;
	}

	bool update()
	{
		return true;
	}

	void draw()
	{
	}
	
	const UMVec4f& diffuse() const { return diffuse_; }
	
	void set_diffuse(const UMVec4f& diffuse) { diffuse_ = diffuse; }

	const UMVec4f& specular() const { return specular_; }
	
	void set_specular(const UMVec4f& specular) { specular_ = specular; }

	const UMVec4f& ambient() const { return ambient_; }

	void set_ambient(const UMVec4f& ambient) { ambient_ = ambient; }

	const UMVec4f& shader_flags() const { return shader_flags_; }

	void set_shader_flags(const UMVec4f& flags) { shader_flags_ = flags; }

	UMOpenGLTexturePtr diffuse_texture() const { return diffuse_texture_; }

	int polygon_count() const { return polygon_count_; }

	void set_polygon_count(int count) { polygon_count_ = count; }

	void set_ummaterial(UMMaterialPtr ummaterial) { ummaterial_ = ummaterial; }

	UMMaterialPtr ummaterial() { return ummaterial_.lock(); }
	
	UMMaterialPtr ummaterial() const { return ummaterial_.lock(); }

	void set_name(const umstring& name) { name_ = name; }

	const umstring& name() { return name_; }

private:
	UMVec4f diffuse_;
	// .w is specular_factor;
	UMVec4f specular_;
	UMVec4f ambient_;
	UMVec4f shader_flags_;
	
	umstring name_;
	UMOpenGLTexturePtr diffuse_texture_;
	
	int polygon_count_;
	UMMaterialWeakPtr ummaterial_;
};

/**
 * constructor.
 */
UMOpenGLMaterial::UMOpenGLMaterial(UMMaterialPtr ummaterial)
	: impl_(new UMOpenGLMaterial::Impl(ummaterial))
{}

/**
 * destructor.
 */
UMOpenGLMaterial::~UMOpenGLMaterial()
{
}

UMOpenGLMaterialPtr UMOpenGLMaterial::default_material()
{
	UMMaterialPtr default_mat = UMMaterial::default_material();
	UMOpenGLMaterialPtr material(std::make_shared<UMOpenGLMaterial>(default_mat));
	material->set_diffuse(UMVec4f(0.0f, 0.7f, 0.7f, 1.0f));
	material->set_specular(UMVec4f(0.9f, 0.9f, 0.9f, 1.0f));
	material->set_ambient(UMVec4f(0.3f, 0.3f, 0.3f, 1.0f));
	return material;
}

/** 
 * initialize
 */
bool UMOpenGLMaterial::init()
{
	return impl_->init();
}

/** 
 * update
 */
bool UMOpenGLMaterial::update()
{
	return impl_->update();
}

/**
 * draw
 */
void UMOpenGLMaterial::draw()
{
	impl_->draw();
}

/**
 * get diffuse
 */
const UMVec4f& UMOpenGLMaterial::diffuse() const
{
	return impl_->diffuse();
}
	
/**
 * set diffuse
 */
void UMOpenGLMaterial::set_diffuse(const UMVec4f& diffuse)
{
	impl_->set_diffuse(diffuse);
}

/**
 * get specular
 */
const UMVec4f& UMOpenGLMaterial::specular() const
{
	return impl_->specular();
}
	
/**
 * set specular
 */
void UMOpenGLMaterial::set_specular(const UMVec4f& specular)
{
	impl_->set_specular(specular);
}

/**
 * get ambient
 */
const UMVec4f& UMOpenGLMaterial::ambient() const
{
	return impl_->ambient();
}

/**
 * set ambient
 */
void UMOpenGLMaterial::set_ambient(const UMVec4f& ambient)
{
	impl_->set_ambient(ambient);
}

/**
 * get shader flags
 * @note x is uvflag, yzw not defined now
 */
const UMVec4f& UMOpenGLMaterial::shader_flags() const
{
	return impl_->shader_flags();
}

/** 
 * set shader flags
 * @note x is uvflag, yzw not defined now
 */
void UMOpenGLMaterial::set_shader_flags(const UMVec4f& flags)
{
	impl_->set_shader_flags(flags);
}

/**
 * get diffuse texture
 */
UMOpenGLTexturePtr UMOpenGLMaterial::diffuse_texture() const
{
	return impl_->diffuse_texture();
}

/**
 * get polygon count
 */
int UMOpenGLMaterial::polygon_count() const
{
	return impl_->polygon_count();
}

/**
 * set polygon count
 */
void UMOpenGLMaterial::set_polygon_count(int count)
{
	impl_->set_polygon_count(count);
}

/**
 * set ummaterial
 */
void UMOpenGLMaterial::set_ummaterial(UMMaterialPtr ummaterial)
{
	impl_->set_ummaterial(ummaterial);
}

/**
 * get ummaterial
 */
UMMaterialPtr UMOpenGLMaterial::ummaterial()
{
	return impl_->ummaterial();
}
	
/**
 * get ummaterial
 */
UMMaterialPtr UMOpenGLMaterial::ummaterial() const
{
	return impl_->ummaterial();
}

/**
 * set name
 */
void UMOpenGLMaterial::set_name(const umstring& name)
{
	impl_->set_name(name);
}

/**
 * get name
 */
const umstring& UMOpenGLMaterial::name()
{
	return impl_->name();
}


} // umdraw

#endif // WITH_OPENGL
