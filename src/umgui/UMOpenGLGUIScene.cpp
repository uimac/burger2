/**
 * @file UMGUI.cpp
 * any
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_OPENGL

#include <memory>
#include <queue>
#include "UMOpenGLGUIScene.h"
#include "UMGUIScene.h"
#include "UMOpenGLGUIBoard.h"
#include "UMOpenGLMesh.h"
#include "UMOpenGLMaterial.h"
#include "UMOpenGLShaderManager.h"
#include "UMOpenGLLight.h"
#include "UMOpenGLCamera.h"
#include "UMOpenGLPoint.h"
#include "UMOpenGLDrawParameter.h"
#include "UMOpenGLIO.h"
#include "UMCamera.h"
#include "UMGUIBoard.h"
#include "UMAny.h"
#include "UMListenerConnector.h"

#include <GL/glew.h>

namespace umgui
{
	using namespace umdraw;

class UMOpenGLGUIScene::SceneImpl : public umbase::UMListenerConnector
{
	DISALLOW_COPY_AND_ASSIGN(SceneImpl);
public:
	SceneImpl() {}
	~SceneImpl() {}

	void init_object(UMGUIObjectPtr obj)
	{
		if (!obj) return;
		std::queue<UMGUIObjectPtr> queue;
		queue.push(obj);
		while (!queue.empty())
		{
			UMGUIObjectPtr target = queue.front();
			queue.pop();
			
			if (UMOpenGLGUIBoardPtr gl_board = std::make_shared<UMOpenGLGUIBoard>(target))
			{
				if (gl_board->init())
				{
					gl_board_list_.push_back(gl_board);
					mutable_event_list().push_back(target->update_event());
					target->update_event()->add_listener(gl_board);
				}
			}

			UMGUIObjectList::iterator it = target->mutable_children().begin();
			for (; it != target->mutable_children().end(); ++it)
			{
				queue.push(*it);
			}
		}
	}

	bool init(UMGUIScenePtr gui_scene)
	{
		gui_scene_ = gui_scene;

		glEnable( GL_DEPTH_TEST );
		glClearColor(0.21f, 0.21f, 0.21f, 1.0f);
		

		// init shader manager
		{
			gl_shader_manager_ = std::make_shared<UMOpenGLShaderManager>();

			// init shader manager for constants
			gl_shader_manager_->init(UMOpenGLShaderManager::eConstants);
			// init shader manager
			gl_shader_manager_->init(UMOpenGLShaderManager::eModel);
			//// init shader manager
			//gl_shader_manager_->init(UMOpenGLShaderManager::ePoint);
		}

		init_object(gui_scene->root_object());

		if (umdraw::UMScenePtr scene = gui_scene->umdraw_scene())
		{
			UMOpenGLLightPtr gl_light = umdraw::UMOpenGLIO::convert_light_to_gl_light(gui_scene->light());
			//UMOpenGLLightPtr light = umdraw::UMOpenGLIO::convert_light_to_gl_light( scene->light_list().at(0) );

			// camera
			UMOpenGLCameraPtr gl_camera = umdraw::UMOpenGLIO::convert_camera_to_gl_camera(gui_scene->camera());
			gl_draw_parameter_ = std::make_shared<umdraw::UMOpenGLDrawParameter>();
			gl_draw_parameter_->set_camera(gl_camera);
			gl_draw_parameter_->set_light(gl_light);
			gl_draw_parameter_->set_shader_manager(gl_shader_manager_);
		}

		return true;
	}

	bool update()
	{
		return true;
	}

	bool clear()
	{
		return true;
	}

	bool draw()
	{
		glDepthMask(GL_FALSE);
		for (UMOpenGLGUIBoardList::iterator it = gl_board_list_.begin(); it != gl_board_list_.end(); ++it)
		{
			(*it)->draw(gl_draw_parameter_);
		}
		glDepthMask(GL_TRUE);
		return true;
	}

	void resize(int width, int height)
	{
	}
	
	/**
	 * keyboard
	 */
	bool on_keyboard(int key, int action)
	{
		if (gui_scene_)
		{
			return gui_scene_->on_keyboard(key, action);
		}
		return false;
	}

	/**
	 * mouse button up/down
	 */
	bool on_mouse(int button, int action)
	{
		if (gui_scene_)
		{
			return gui_scene_->on_mouse(button, action);
		}
		return false;
	}

	/**
	 * mouse move
	 */
	bool on_mouse_move(double x, double y)
	{
		if (gui_scene_)
		{
			return gui_scene_->on_mouse_move(x, y);
		}
		return false;
	}
	
	/**
	 * scroll
	 */
	bool on_scroll(double x, double y)
	{
		if (gui_scene_)
		{
			return gui_scene_->on_scroll(x, y);
		}
		return false;
	}

private:

	UMOpenGLShaderManagerPtr gl_shader_manager_;
	umdraw::UMOpenGLDrawParameterPtr gl_draw_parameter_;
	UMOpenGLGUIBoardList gl_board_list_;
	UMGUIScenePtr gui_scene_;
};

/**
 * constructor
 */
UMOpenGLGUIScene::UMOpenGLGUIScene()
	:  impl_(new UMOpenGLGUIScene::SceneImpl)
{
}

/**
 * destructor
 */
UMOpenGLGUIScene::~UMOpenGLGUIScene()
{
}

/**
 * initialize
 */
bool UMOpenGLGUIScene::init(UMGUIScenePtr scene)
{
	return impl_->init(scene);
}

/**
 * update
 */
bool UMOpenGLGUIScene::update()
{
	return impl_->update();
}

/**
 * create view/depth
 */
bool UMOpenGLGUIScene::clear()
{
	return impl_->clear();
}

/**
 * draw
 */
bool UMOpenGLGUIScene::draw()
{
	return impl_->draw();
}

/**
 * resize
 */
void UMOpenGLGUIScene::resize(int width, int height)
{
	return impl_->resize(width, height);
}

/**
 * keyboard
 */
bool UMOpenGLGUIScene::on_keyboard(int key, int action)
{
	return impl_->on_keyboard(key, action);
}

/**
 * mouse button up/down
 */
bool UMOpenGLGUIScene::on_mouse(int button, int action)
{
	return impl_->on_mouse(button, action);
}

/**
 * mouse move
 */
bool UMOpenGLGUIScene::on_mouse_move(double x, double y)
{
	return impl_->on_mouse_move(x, y);
}

/**
 * scroll
 */
bool UMOpenGLGUIScene::on_scroll(double x, double y)
{
	return impl_->on_scroll(x, y);
}

} // umgui

#endif // WITH_OPENGL
