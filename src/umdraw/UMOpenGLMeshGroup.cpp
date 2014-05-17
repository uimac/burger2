/**
 * @file UMOpenGLMeshGroup.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_OPENGL

#include "UMOpenGLMeshGroup.h"

#include "UMOpenGLMaterial.h"
#include "UMOpenGLTexture.h"
#include "UMOpenGLDrawParameter.h"
#include "UMOpenGLShaderManager.h"
#include "UMOpenGLMesh.h"
#include "UMMeshGroup.h"
#include "UMMesh.h"
#include "UMStringUtil.h"
#include "UMTime.h"
#include <GL/glew.h>

namespace umdraw
{

class UMOpenGLMeshGroup::Impl
{
	DISALLOW_COPY_AND_ASSIGN(Impl);
public:

	Impl()
		: shader_manager_(std::make_shared<UMOpenGLShaderManager>())
	{}

	~Impl() 
	{
	}

	bool init()
	{
		// init shader manager for constants
		shader_manager_->init(UMOpenGLShaderManager::eConstants);
		// init shader manager
		shader_manager_->init(UMOpenGLShaderManager::eModel);
		return true;
	}

	bool update()
	{
		return true;
	}

	void draw(UMOpenGLDrawParameterPtr parameter)
	{
		UMOpenGLMeshList::iterator it = gl_mesh_list_.begin();
		for (; it != gl_mesh_list_.end(); ++it)
		{
			UMOpenGLMeshPtr mesh = *it;
			if (!parameter->shader_manager())
			{
				parameter->set_shader_manager(shader_manager_);
			}
			mesh->draw(parameter);
		}
	}

	const UMOpenGLMeshList& gl_mesh_list() const { return gl_mesh_list_; }
	
	UMOpenGLMeshList& mutable_gl_mesh_list() { return gl_mesh_list_; }
	
	void set_ummesh_group(UMMeshGroupPtr ummesh_group) { ummesh_group_ = ummesh_group; }

	UMMeshGroupPtr ummesh_group() { return ummesh_group_.lock(); }

private:
	/**
	 * get shader manager
	 */
	UMOpenGLShaderManagerPtr shader_manager() const { return shader_manager_; }
	
	/**
	 * get shader manager
	 */
	UMOpenGLShaderManager& mutable_shader_manager() { return *shader_manager_; }
	
	UMOpenGLShaderManagerPtr shader_manager_;
	UMOpenGLMeshList gl_mesh_list_;
	UMMeshGroupWeakPtr ummesh_group_;
};

/**
 * constructor.
 */
UMOpenGLMeshGroup::UMOpenGLMeshGroup()
	: impl_(new UMOpenGLMeshGroup::Impl)
{}

/**
 * destructor.
 */
UMOpenGLMeshGroup::~UMOpenGLMeshGroup()
{
}

/**
 * init meseh group
 */
bool UMOpenGLMeshGroup::init()
{
	return impl_->init();
}

/**
 * update mesh group
 */
bool UMOpenGLMeshGroup::update()
{
	return impl_->update();
}

/**
 * draw mesh group
 */
void UMOpenGLMeshGroup::draw(UMOpenGLDrawParameterPtr parameter)
{
	impl_->draw(parameter);
}

/**
 * get OpenGL mesh list
 */
const UMOpenGLMeshList& UMOpenGLMeshGroup::gl_mesh_list() const 
{
	return impl_->gl_mesh_list();
}
	
/**
 * get OpenGL mesh list
 */
UMOpenGLMeshList& UMOpenGLMeshGroup::mutable_gl_mesh_list()
{
	return impl_->mutable_gl_mesh_list();
}
	
/**
 * set ummesh group
 */
void UMOpenGLMeshGroup::set_ummesh_group(UMMeshGroupPtr ummesh_group) 
{
	impl_->set_ummesh_group(ummesh_group);
}

/**
 * get ummesh group
 */
UMMeshGroupPtr UMOpenGLMeshGroup::ummesh_group()
{
	return impl_->ummesh_group();
}

} // umdraw

#endif // WITH_OPENGL
