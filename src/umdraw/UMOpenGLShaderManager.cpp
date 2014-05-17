/**
 * @file UMOpenGLShaderManager.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_OPENGL

#include "UMOpenGLShaderManager.h"

#include <GL/glew.h>

#include "UMMathTypes.h"
#include "UMVector.h"
#include "UMMatrix.h"
#include "UMStringUtil.h"
#include "UMPath.h"
#include "UMShaderEntry.h"

namespace umdraw
{

class UMOpenGLShaderManager::Impl
{
	DISALLOW_COPY_AND_ASSIGN(Impl);
public:

	Impl() :
		is_deferred_(false)
	{}
	~Impl() {}
	
	bool init(ShaderType type)
	{
		if (type == eConstants)
		{
			uniform_buffer_list_.resize(2);
			{
				unsigned int light_ubo = 0;
				glGenBuffers(1, &light_ubo);
				glBindBuffer(GL_UNIFORM_BUFFER, light_ubo);
				uniform_buffer_list_[0] = light_ubo;
			}
			{
				unsigned int material_ubo = 0;
				glGenBuffers(1, &material_ubo);
				glBindBuffer(GL_UNIFORM_BUFFER, material_ubo);
				uniform_buffer_list_[1] = material_ubo;
			}
		}
		else if (type == eModel)
		{
			UMOpenGLShaderPtr shader(std::make_shared<UMOpenGLShader>());

			const std::string& vertex_shader = UMShaderEntry::instance().gl_vertex_shader();
			const std::string& fragment_shader = UMShaderEntry::instance().gl_fragment_shader();
	#ifndef _DEBUG
			if (shader->create_shader_from_memory(vertex_shader, fragment_shader))
			{
				// save shader
				mutable_shader_list().push_back(shader);
			}
			else
	#endif // not _DEBUG
			{
				// shader from resource directory (for debug)
				std::u16string vs_path = umbase::UMPath::resource_absolute_path(umbase::UMStringUtil::utf8_to_utf16("UMModelShader.vs"));
				std::u16string fs_path = umbase::UMPath::resource_absolute_path(umbase::UMStringUtil::utf8_to_utf16("UMModelShader.fs"));

				if (shader->create_shader_from_file(
					vs_path,
					fs_path))
				{
					// save shader
					mutable_shader_list().push_back(shader);
				}
			}
		}
		else if (type == ePoint)
		{
			UMOpenGLShaderPtr shader(std::make_shared<UMOpenGLShader>());

			const std::string& vertex_shader = UMShaderEntry::instance().gl_point_vertex_shader();
			const std::string& fragment_shader = UMShaderEntry::instance().gl_point_fragment_shader();
	#ifndef _DEBUG
			if (shader->create_shader_from_memory(vertex_shader, fragment_shader))
			{
				// save shader
				mutable_shader_list().push_back(shader);
			}
			else
	#endif // not _DEBUG
			{
				// shader from resource directory (for debug)
				std::u16string vs_path = umbase::UMPath::resource_absolute_path(umbase::UMStringUtil::utf8_to_utf16("UMPointShader.vs"));
				std::u16string fs_path = umbase::UMPath::resource_absolute_path(umbase::UMStringUtil::utf8_to_utf16("UMPointShader.fs"));

				if (shader->create_shader_from_file(
					vs_path,
					fs_path))
				{
					// save shader
					mutable_shader_list().push_back(shader);
				}
			}
		}
		else if (type == eOriginal)
		{
			// do nothing
			;
		}
		else if (type == eBoard)
		{
			// board shader
			{
				UMOpenGLShaderPtr shader(std::make_shared<UMOpenGLShader>());
				const std::string& vertex_shader = UMShaderEntry::instance().gl_board_vertex_shader();
				const std::string& fragment_shader = UMShaderEntry::instance().gl_board_fragment_shader();
		#ifndef _DEBUG
				if (shader->create_shader_from_memory(vertex_shader, fragment_shader))
				{
					// save shader
					mutable_shader_list().push_back(shader);
				}
				else
		#endif // not _DEBUG
				{
					// shader from resource directory (for debug)
					std::u16string vs_path = umbase::UMPath::resource_absolute_path(umbase::UMStringUtil::utf8_to_utf16("UMBoardShader.vs"));
					std::u16string fs_path = umbase::UMPath::resource_absolute_path(umbase::UMStringUtil::utf8_to_utf16("UMBoardShader.fs"));

					if (shader->create_shader_from_file(
						vs_path,
						fs_path))
					{
						// save shader
						mutable_shader_list().push_back(shader);
					}
				}
			}
		}
		else if (type == eBoardForDeferred)
		{
			is_deferred_ = true;
			// board shader
			{
				UMOpenGLShaderPtr shader(std::make_shared<UMOpenGLShader>());
				const std::string& vertex_shader = UMShaderEntry::instance().gl_board_light_pass_vertex_shader();
				const std::string& fragment_shader = UMShaderEntry::instance().gl_board_light_pass_fragment_shader();
		#ifndef _DEBUG
				if (shader->create_shader_from_memory(vertex_shader, fragment_shader))
				{
					// save shader
					mutable_shader_list().push_back(shader);
				}
				else
		#endif // not _DEBUG
				{
					// shader from resource directory (for debug)
					std::u16string vs_path = umbase::UMPath::resource_absolute_path(umbase::UMStringUtil::utf8_to_utf16("UMBoardLightPass.vs"));
					std::u16string fs_path = umbase::UMPath::resource_absolute_path(umbase::UMStringUtil::utf8_to_utf16("UMBoardLightPass.fs"));

					if (shader->create_shader_from_file(
						vs_path,
						fs_path))
					{
						// save shader
						mutable_shader_list().push_back(shader);
					}
				}
			}
		}
		else if (type == eModelDeferredGeo)
		{
			is_deferred_ = true;
			// board shader
			{
				UMOpenGLShaderPtr shader(std::make_shared<UMOpenGLShader>());
				const std::string& vertex_shader = UMShaderEntry::instance().gl_vertex_geo_shader();
				const std::string& fragment_shader = UMShaderEntry::instance().gl_fragment_geo_shader();
		#ifndef _DEBUG
				if (shader->create_shader_from_memory(vertex_shader, fragment_shader))
				{
					// save shader
					mutable_shader_list().push_back(shader);
				}
				else
		#endif // not _DEBUG
				{
					// shader from resource directory (for debug)
					std::u16string vs_path = umbase::UMPath::resource_absolute_path(umbase::UMStringUtil::utf8_to_utf16("UMModelGeometryPass.vs"));
					std::u16string fs_path = umbase::UMPath::resource_absolute_path(umbase::UMStringUtil::utf8_to_utf16("UMModelGeometryPass.fs"));

					if (shader->create_shader_from_file(
						vs_path,
						fs_path))
					{
						// save shader
						mutable_shader_list().push_back(shader);
					}
				}
			}
		}
		return true;
	}

	const BufferList& uniform_buffer_list() const
	{
		return uniform_buffer_list_;
	}
	
	const ShaderList& shader_list() const
	{
		return shader_list_;
	}

	ShaderList& mutable_shader_list() 
	{
		return shader_list_;
	}

	bool is_deferred() const
	{
		return is_deferred_;
	}

private:
	ShaderList shader_list_;
	BufferList uniform_buffer_list_;
	bool is_deferred_;
};

/// constructer
UMOpenGLShaderManager::UMOpenGLShaderManager()
	: impl_(new UMOpenGLShaderManager::Impl())
{
}

/// destructor
UMOpenGLShaderManager::~UMOpenGLShaderManager()
{
}

/**
 * initialize
 */
bool UMOpenGLShaderManager::init(ShaderType type)
{
	return impl_->init(type);
}

/**
 * get constant buffer list
 */
const UMOpenGLShaderManager::BufferList& UMOpenGLShaderManager::uniform_buffer_list() const
{
	return impl_->uniform_buffer_list();
}
	
/**
 * get shader list
 */
const UMOpenGLShaderManager::ShaderList& UMOpenGLShaderManager::shader_list() const
{
	return impl_->shader_list();
}

/**
 * get shader list
 */
UMOpenGLShaderManager::ShaderList& UMOpenGLShaderManager::mutable_shader_list() 
{
	return impl_->mutable_shader_list();
}

/**
 * is deferred lighting
 */
bool UMOpenGLShaderManager::is_deferred() const
{
	return impl_->is_deferred();
}

} // umdraw

#endif // WITH_OPENGL
