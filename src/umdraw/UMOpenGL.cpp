/**
 * @file UMOpenGL.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_OPENGL

#include "UMOpenGLScene.h"
#include "UMOpenGL.h"
#include "UMOpenGLShaderManager.h"
#include "UMOpenGLTexture.h"
#include "UMPath.h"
#include "UMStringUtil.h"
#include "UMAny.h"
#include "UMListenerConnector.h"

#include "UMScene.h"
#include "UMTime.h"
#include "UMImage.h"

#include <tchar.h>
#include <shlwapi.h>
#include <GL/glew.h>
#include "UMIO.h"

namespace umdraw
{

class UMOpenGLImpl : public umbase::UMListenerConnector
{
public:
	UMOpenGLImpl()
		:
	gl_scene(std::make_shared<UMOpenGLScene>())
	{}
	
	~UMOpenGLImpl()
	{}

	/**
	 * initialize
	 */
	virtual bool init(UMScenePtr scene);
	
	/**
	 * prep for next drawing
	 */
	bool update();

	/**
	 * draw frame
	 */
	bool draw();

	/**
	 * clear view/depth
	 */
	bool clear();
	
	/**
	 * resize
	 */
	void resize(int width, int height);

	/**
	 * get umdraw scene
	 */
	UMScenePtr scene() const {
		return gl_scene->scene();
	}

private:
	HWND handle_;
	UMOpenGLScenePtr gl_scene;
};

/**
 * initialize
 */
bool UMOpenGLImpl::init(UMScenePtr scene)
{
	glEnable( GL_DEPTH_TEST );
	//glDepthMask(GL_TRUE);
	//glDepthFunc( GL_LESS );
	glClearColor(0.21f, 0.21f, 0.21f, 1.0f);
	glEnable( GL_CULL_FACE );
	//glClearDepth(1.0f);
	
	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	mutable_event_list().clear();

	if (gl_scene->init(scene))
	{
		if (gl_scene->load(scene))
		{
			mutable_event_list().push_back(scene->camera_change_event());
			connect(gl_scene);
			return true;
		}
	}

	return false;
}

/**
 * update scene
 */
bool UMOpenGLImpl::update()
{
	if (!gl_scene->update())
	{
		return false;
	}
	return true;
}

/**
 * draw impl
 */
bool UMOpenGLImpl::draw()
{
	// draw scene
	if (!gl_scene->draw())
	{
		return false;
	}
	return true;
}

/**
 * clear view/depth
 */
bool UMOpenGLImpl::clear()
{
	if (!gl_scene->clear())
	{
		return false;
	}
	return true;
}

/**
 * resize
 */
void UMOpenGLImpl::resize(int width, int height)
{
	glViewport(0, 0, width, height);
}


/**
 * create instance
 */
UMOpenGLPtr UMOpenGL::create()
{
	UMOpenGLPtr viewer = UMOpenGLPtr(new UMOpenGL);
	return viewer;
}

/**
 * constructor
 */
UMOpenGL::UMOpenGL()
	:  impl_(std::make_shared<UMOpenGLImpl>())
{
}

/**
 * destructor
 */
UMOpenGL::~UMOpenGL()
{
}

/**
 * prep for next drawing
 */
bool UMOpenGL::update()
{
	return impl_->update();
}

/*
 * draw
 */
bool UMOpenGL::draw()
{
	return impl_->draw();
}

/**
 * create view/depth
 */
bool UMOpenGL::clear()
{
	return impl_->clear();
}

/**
 * resize
 */
void UMOpenGL::resize(int width, int height)
{
	impl_->resize(width, height);
}

/**
 * initialize
 */
bool UMOpenGL::init(void* hWnd, UMScenePtr scene)
{
	return impl_->init(scene);
}

/**
 * get umdraw scene
 */
UMScenePtr UMOpenGL::scene() const
{
	return impl_->scene();
}


} // umdraw

#endif // WITH_OPENGL
