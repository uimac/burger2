/**
 * @file UMOpenGLTexture.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */

#ifdef WITH_OPENGL

#ifdef WITH_EMSCRIPTEN
	#include <GL/glew.h>
	#include <GL/glu.h>
	//#include <GLES2/gl2.h>
	//#include <EGL/egl.h>
#else
	#include <GL/glew.h>
	#include <GL/glu.h>
#endif

#include "UMOpenGLTexture.h"
#include "UMListener.h"
#include "UMListenerConnector.h"
#include "UMImageEventType.h"
#include "UMVector.h"
#include "UMStringUtil.h"
#include "UMPath.h"
#include "UMImage.h"
#include "UMAny.h"

namespace
{
	typedef std::map<umstring, unsigned int> UMOpenGLTexturePool;
	UMOpenGLTexturePool texture_pool;
	UMOpenGLTexturePool fbo_pool;
	UMOpenGLTexturePool rb_pool;

	typedef std::map<umstring, umimage::UMImagePtr> UMOpenGLTextureImagePool;
	UMOpenGLTextureImagePool texture_image_pool;

	
	int color_attachments[] = {
		GL_COLOR_ATTACHMENT0,
#if !defined(WITH_EMSCRIPTEN)
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3,
		GL_COLOR_ATTACHMENT4,
		GL_COLOR_ATTACHMENT5,
		GL_COLOR_ATTACHMENT6,
		GL_COLOR_ATTACHMENT7,
		GL_COLOR_ATTACHMENT8,
		GL_COLOR_ATTACHMENT9,
		GL_COLOR_ATTACHMENT10,
		GL_COLOR_ATTACHMENT11,
		GL_COLOR_ATTACHMENT12,
		GL_COLOR_ATTACHMENT13,
		GL_COLOR_ATTACHMENT14,
		GL_COLOR_ATTACHMENT15,
#endif //  !defined(WITH_EMSCRIPTEN)
	};
	int color_attachments_size = static_cast<int>(sizeof(color_attachments) / sizeof(color_attachments[0]));
};

namespace umdraw
{

class UMOpenGLTextureListener;
typedef std::shared_ptr<UMOpenGLTextureListener> UMOpenGLTextureListenerPtr;
class UMOpenGLTextureListener : public umbase::UMListener
{
	DISALLOW_COPY_AND_ASSIGN(UMOpenGLTextureListener);
public:
	UMOpenGLTextureListener()
		: id_(0)
	{}

	~UMOpenGLTextureListener()
	{}

	void set_texture_id(unsigned int texture_id) { id_ = texture_id; }

	unsigned int texture_id() const { return id_; }

	void set_image(umimage::UMImagePtr image)
	{
		image_ = image;
	}

	virtual void update(umbase::UMEventType event_type, umbase::UMAny& parameter) 
	{
		if (event_type == umimage::eImageEventImageChaged)
		{
			const UMVec4d uv = umbase::any_cast<UMVec4d>(parameter);
			const int width = image_->width();
			const int height = image_->height();
			int x = static_cast<int>(width * uv[0]);
			int y = static_cast<int>(height * uv[1]);
			int w = static_cast<int>(ceil(width * (uv[2] - uv[0]))) + 1;
			int h = static_cast<int>(ceil(height * (uv[3] - uv[1]))) + 1;
			if (uv[0] > uv[2])
			{
				x = static_cast<int>(width * uv[2]);
				w = static_cast<int>(ceil(width * (uv[0] - uv[2]))) + 1;
			}
			if (uv[1] > uv[3])
			{
				y = static_cast<int>(height * uv[3]);
				h = static_cast<int>(ceil(height * (uv[1] - uv[3]))) + 1;
			}
			buffer_.clear();
			image_->create_r8g8b8a8_buffer(buffer_, umbase::UMVec4ui(x, y, x+w, y+h));
			//image_->create_r8g8b8a8_buffer(buffer_, umbase::UMVec4ui(0, 0, 128, 128));
			if (!buffer_.empty())
			{
				glBindTexture(GL_TEXTURE_2D, id_);
				//printf("a %d %d %d %d\n", x, y, w, h);
				glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, &(*buffer_.begin()));
				//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 128, 128, GL_RGBA, GL_UNSIGNED_BYTE, &(*buffer_.begin()));
				glBindTexture(GL_TEXTURE_2D, 0);
			}
		}
	}
private:
	umimage::UMImagePtr image_;
	umimage::UMImage::R8G8B8A8Buffer buffer_;
	unsigned int id_;
};

class UMOpenGLTexture::Impl : public umbase::UMListenerConnector
{
	DISALLOW_COPY_AND_ASSIGN(Impl);
public:
	
	Impl(bool can_overwrite) 
		: can_overwrite_(can_overwrite)
		, is_valid_texture_(false)
		, is_valid_frame_buffer_(false)
		, is_valid_render_buffer_(false)
		, id_(0)
		, render_buffer_id_(0)
		, frame_buffer_id_(0)
		, listener_(new UMOpenGLTextureListener())
	{
	}

	~Impl() 
	{
		// erase texture from pool
		{
			UMOpenGLTexturePool::iterator it = texture_pool.find(file_path_);
			if (it != texture_pool.end())
			{
				texture_pool.erase(it);
			}
		}
		// erase image from pool
		{
			UMOpenGLTextureImagePool::iterator it = texture_image_pool.find(file_path_);
			if (it != texture_image_pool.end())
			{
				texture_image_pool.erase(it);
			}
		}
		// erase fbo from pool
		{
			UMOpenGLTexturePool::iterator it = fbo_pool.find(file_path_);
			if (it != fbo_pool.end())
			{
				fbo_pool.erase(it);
			}
		}
		// erase rb from pool
		{
			UMOpenGLTexturePool::iterator it = rb_pool.find(file_path_);
			if (it != rb_pool.end())
			{
				rb_pool.erase(it);
			}
		}
		if (is_valid_texture_)
		{
			glDeleteTextures(1, &id_);
		}
		if (is_valid_frame_buffer_)
		{
			glDeleteFramebuffers(1, &frame_buffer_id_);
		}
		if (is_valid_render_buffer_)
		{
			glDeleteRenderbuffers(1, &render_buffer_id_);
		}
	}

	bool load(const umstring& file_path)
	{
		// find texture from pool
		if (texture_pool.find(file_path) != texture_pool.end())
		{
			id_ = texture_pool[file_path];
			is_valid_texture_ = true;
			file_path_ = file_path;
			image_ = texture_image_pool[file_path];
			return true;
		}

		//  load new image from file
		image_ = umimage::UMImage::load(file_path);
		if (!image_) return false;

		umimage::UMImage::R8G8B8A8Buffer buffer;
		umimage::UMImagePtr flipped = image_->create_flip_image(false, true);
		flipped->create_r8g8b8a8_buffer(buffer);

		// create texture
		GLuint new_tex = -1;
		glGenTextures(1, &new_tex);
		id_ = new_tex;

		const int width = image_->width();
		const int height = image_->height();
		glBindTexture(GL_TEXTURE_2D, id_);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef WITH_EMSCRIPTEN
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(*buffer.begin()));
#else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		gluBuild2DMipmaps(
			GL_TEXTURE_2D, GL_RGBA, width, height,
			GL_RGBA, GL_UNSIGNED_BYTE, &(*buffer.begin())
		);
#endif
		glBindTexture(GL_TEXTURE_2D, 0);
		
		connect_event();

		if (can_overwrite_)
		{
			umimage::UMImagePtr image = convert_to_image();
			if (image)
			{
				umimage::UMImagePtr fliped = image->create_flip_image(false, true);
				if (fliped)
				{
					convert_from_image(*fliped);
				}
			}
		}

		file_path_ = file_path;
		texture_pool[file_path] = id_;
		texture_image_pool[file_path] = image_;
		is_valid_texture_ = true;

		return true;
	}

	bool convert_from_image(const umimage::UMImage& image)
	{
		if (!image.is_valid()) return false;
	
		// find texture from pool
		umstring id = 
			umbase::UMStringUtil::utf8_to_utf16(umbase::UMStringUtil::number_to_string(image.id()));
		// find texture from pool
		if (texture_pool.find(id) != texture_pool.end())
		{
			id_ = texture_pool[id];
			is_valid_texture_ = true;
			file_path_ = id;
			image_ = texture_image_pool[id];
			return true;
		}

		umimage::UMImage::R8G8B8A8Buffer buffer;
		image.create_r8g8b8a8_buffer(buffer);
	
		// create texture
		GLuint new_tex = -1;
		glGenTextures(1, &new_tex);
		id_ = new_tex;

		const int width = image.width();
		const int height = image.height();
		glBindTexture(GL_TEXTURE_2D, id_);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef WITH_EMSCRIPTEN
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(*buffer.begin()));
#else
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		gluBuild2DMipmaps(
			GL_TEXTURE_2D, GL_RGBA, width, height,
			GL_RGBA, GL_UNSIGNED_BYTE, &(*buffer.begin())
		);
#endif
		glBindTexture(GL_TEXTURE_2D, 0);
	
		connect_event();

		file_path_ = id;
		texture_pool[file_path_] = id_;
		texture_image_pool[file_path_] = image_;
		is_valid_texture_ = true;
		return true;
	}
	
	bool create_texture(int width, int height, const Format& format)
	{
		static int image_id = 100000;
		++image_id;

		// find texture from pool
		umstring id = umbase::UMStringUtil::utf8_to_utf16(
			umbase::UMStringUtil::number_to_string(image_id));

		// find texture from pool
		if (texture_pool.find(id) != texture_pool.end())
		{
			id_ = texture_pool[id];
			is_valid_texture_ = true;
			file_path_ = id;
			image_ = texture_image_pool[id];
			return true;
		}
	
		// create texture
		glActiveTexture(GL_TEXTURE0);
		GLuint new_tex = -1;
		glGenTextures(1, &new_tex);
		id_ = new_tex;

		glBindTexture(GL_TEXTURE_2D, id_);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		unsigned int gl_type = (format.type == eUNSIGNED_BYTE) ? GL_UNSIGNED_BYTE : 0;
		unsigned int gl_internal_format= (format.internal_format == eRGBA) ? GL_RGBA : format.internal_format;
		unsigned int gl_format= (format.format == eRGBA) ? GL_RGBA : format.format;
		glTexImage2D(GL_TEXTURE_2D, 0, gl_internal_format, width, height, 0, gl_format, gl_type, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
	
		connect_event();

		file_path_ = id;
		texture_pool[file_path_] = id_;
		texture_image_pool[file_path_] = image_;
		is_valid_texture_ = true;
		return true;
	}
	
	bool create_render_buffer(int width, int height)
	{
		static int image_id = 200000;
		++image_id;

		// find texture from pool
		umstring id = umbase::UMStringUtil::utf8_to_utf16(
			umbase::UMStringUtil::number_to_string(image_id));

		// find texture from pool
		if (rb_pool.find(id) != rb_pool.end())
		{
			render_buffer_id_ = rb_pool[id];
			is_valid_render_buffer_ = true;
			file_path_ = id;
			image_ = texture_image_pool[id];
			return true;
		}
	
		glGenRenderbuffers(1, &render_buffer_id_);
		glBindRenderbuffer(GL_RENDERBUFFER, render_buffer_id_);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		connect_event();

		file_path_ = id;
		rb_pool[file_path_] = render_buffer_id_;
		texture_image_pool[file_path_] = image_;
		is_valid_render_buffer_ = true;
		return true;
	}

	bool create_frame_buffer(const std::vector<unsigned int>& texture_id, unsigned int render_buffer)
	{
		if (texture_id.empty()) return false;
		if (render_buffer == 0) return false;
		
		static int image_id = 300000;
		++image_id;

		// find texture from pool
		umstring id = umbase::UMStringUtil::utf8_to_utf16(
			umbase::UMStringUtil::number_to_string(image_id));

		// find texture from pool
		if (fbo_pool.find(id) != fbo_pool.end())
		{
			frame_buffer_id_ = fbo_pool[id];
			is_valid_frame_buffer_ = true;
			file_path_ = id;
			image_ = texture_image_pool[id];
			return true;
		}

		frame_buffer_attachments_.clear();
		glGenFramebuffers(1, &frame_buffer_id_);
		glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_id_);
		for (int i = 0, isize = static_cast<int>(texture_id.size()); i < isize; ++i)
		{
			frame_buffer_attachments_.push_back(color_attachments[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, color_attachments[i], GL_TEXTURE_2D, texture_id[i], 0 );
		}
		glBindRenderbuffer(GL_RENDERBUFFER, render_buffer);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, render_buffer);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			return false;
		}
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		file_path_ = id;
		fbo_pool[file_path_] = frame_buffer_id_;
		texture_image_pool[file_path_] = image_;
		is_valid_frame_buffer_ = true;
		return true;
	}

	umimage::UMImagePtr convert_to_image() const
	{
		return image_;
	}

	bool overwrite(const umimage::UMImage& image)
	{
		if (!can_overwrite_) return false;

		return false;
	}

	unsigned int texture_id() const
	{
		return id_;
	}
	
	/**
	 * get render buffer id
	 */
	unsigned int render_buffer_id() const
	{
		return render_buffer_id_;
	}

	/**
	 * get frame buffer id
	 */
	unsigned int frame_buffer_id() const
	{
		return frame_buffer_id_;
	}
	
	/**
	 * get frame buffer attachments
	 */
	const std::vector<unsigned int>& frame_buffer_attachments() const
	{
		return frame_buffer_attachments_;
	}

private:
	// connect event
	void connect_event()
	{
		if (!image_) return;
		mutable_event_list().clear();
		mutable_event_list().push_back(image_->image_change_event());
		listener_list().clear();
		listener_->set_image(image_);
		listener_->set_texture_id(id_);
		connect(listener_);
	}

	UMOpenGLTextureListenerPtr listener_;
	bool can_overwrite_;
	bool is_valid_texture_;
	bool is_valid_frame_buffer_;
	bool is_valid_render_buffer_;
	unsigned int id_;
	unsigned int render_buffer_id_;
	unsigned int frame_buffer_id_;
	umstring file_path_;
	umimage::UMImagePtr image_;
	std::vector<unsigned int> frame_buffer_attachments_;
};

UMOpenGLTexture::UMOpenGLTexture(bool can_overwrite) 
	: impl_(new UMOpenGLTexture::Impl(can_overwrite))
{
}

UMOpenGLTexture::~UMOpenGLTexture()
{
}

/**
 * create texture without image
 * @param [in] width width
 * @param [in] height height
 */
UMOpenGLTexturePtr UMOpenGLTexture::create_texture(int width, int height, const Format& format)
{
	UMOpenGLTexturePtr texture = std::make_shared<UMOpenGLTexture>(false);
	if (texture && texture->impl_->create_texture(width, height, format))
	{
		return texture;
	}
	return UMOpenGLTexturePtr();
}

/**
 * create render buffer
 * @param [in] width width
 * @param [in] height height
 */
UMOpenGLTexturePtr UMOpenGLTexture::create_render_buffer(int width, int height)
{
	UMOpenGLTexturePtr texture = std::make_shared<UMOpenGLTexture>(false);
	if (texture && texture->impl_->create_render_buffer(width, height))
	{
		return texture;
	}
	return UMOpenGLTexturePtr();
}

/**
 * create frame buffer
 * @param [in] textures
 * @param [in] render_buffer
 */
UMOpenGLTexturePtr UMOpenGLTexture::create_frame_buffer(
	UMOpenGLTextureList& textures, UMOpenGLTexturePtr render_buffer)
{
	UMOpenGLTexturePtr texture = std::make_shared<UMOpenGLTexture>(false);
	if (texture)
	{
		std::vector<unsigned int> texture_ids;
		for (int i = 0, isize = static_cast<int>(textures.size()); i < isize; ++i)
		{
			texture_ids.push_back(textures[i]->texture_id());
		}
		if (texture->impl_->create_frame_buffer(texture_ids, render_buffer->render_buffer_id()))
		{
			return texture;
		}
	}
	return UMOpenGLTexturePtr();
}

/**
 * load an image file as a directx texture
 * @param [in] file_path absolute texture file path
 */
bool UMOpenGLTexture::load(const umstring& file_path)
{
	return impl_->load(file_path);
}

/**
 * convert UMImage to DirectX11 Texture
 * @param [in] image source image
 * @retval success or fail
s */
bool UMOpenGLTexture::convert_from_image(const umimage::UMImage& image)
{
	return impl_->convert_from_image(image);
}

/**
 * convert DirectX texture buffer to memory
 * @param [in] device_pointer directx11 device pointer
 * @retval UMImagePtr converted image buffer
 */
umimage::UMImagePtr UMOpenGLTexture::convert_to_image() const
{
	return impl_->convert_to_image();
}


/**
 * overwrite texture by image
 * @info only valid when can_overwrite_ is true
 */
bool UMOpenGLTexture::overwrite(const umimage::UMImage& image)
{
	return impl_->overwrite(image);
}

/**
 * get texture id
 */
unsigned int UMOpenGLTexture::texture_id() const 
{
	return impl_->texture_id();
}

/**
 * get render buffer id
 */
unsigned int UMOpenGLTexture::render_buffer_id() const
{
	return impl_->render_buffer_id();
}

/**
 * get frame buffer id
 */
unsigned int UMOpenGLTexture::frame_buffer_id() const
{
	return impl_->frame_buffer_id();
}

/**
 * get frame buffer attachments
 */
const std::vector<unsigned int>& UMOpenGLTexture::frame_buffer_attachments() const
{
	return impl_->frame_buffer_attachments();
}

} // umdraw

#endif // WITH_OPENGL
