/**
 * @file UMGUIScene.cpp
 * any
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#include "UMGUIScene.h"

#include <memory>
#if !defined(WITH_EMSCRIPTEN)
	#include <tchar.h>
#endif // WITH_EMSCRIPTEN
#include "UMGUIObject.h"
#include "UMStringUtil.h"
#include "UMPath.h"
#include "UMMaterial.h"
#include "UMGUIBoard.h"
#include "UMGUIScrollBoard.h"
#include "UMResource.h"
#include "UMScene.h"
#include "UMCamera.h"
#include <GL/glfw3.h>

namespace umgui
{
	using namespace umdraw;
	using namespace umresource;

class UMGUIScene::SceneImpl
{
	DISALLOW_COPY_AND_ASSIGN(SceneImpl);
public:

	SceneImpl() {
		intersect_list.reserve(10);
	}
	~SceneImpl() {
	}
	
	/**
	 * initialize
	 */
	bool init(int width, int height)
	{
		object_ = UMGUIBoard::create_root_board(width, height);
		
		UMScenePtr scene = scene_.lock();
		if (!scene) { return true; }
		
		double pre_x_ = 0.0;
		double pre_y_ = 0.0;
		double current_x_ = 0.0;
		double current_y_ = 0.0;
		bool is_ctrl_button_down_ = false;
		bool is_left_button_down_ = false;
		bool is_right_button_down_ = false;
		bool is_middle_button_down_ = false;
		bool is_alt_down_ = false;
		bool is_shift_down_ = false;

		camera_ = UMCameraPtr(new UMCamera(true, width, height));
		if (camera_)
		{
			light_ = std::make_shared<UMLight>(UMVec3d(0, 0, 1000));
			if (light_)
			{
				return true;
			}
		}
		return false;
	}
	
	/**
	 * set umdraw scene
	 */
	void set_umdraw_scene(umdraw::UMScenePtr scene)
	{
		scene_ = scene;
	}
	
	/**
	 * get umdraw scene
	 */
	umdraw::UMScenePtr umdraw_scene()
	{
		return scene_.lock();
	}
	
	/**
	 * get camera
	 */
	UMCameraPtr camera()
	{
		return camera_;
	}
	
	/**
	 * get light
	 */
	UMLightPtr light()
	{
		return light_;
	}

	/**
	 * get root object
	 */
	UMGUIObjectPtr root_object()
	{
		return object_;
	}
	
	/**
	 * keyboard
	 */
	bool on_keyboard(int key, int action)
	{
		return false;
	}

	/**
	 * mouse button up/down
	 */
	bool on_mouse(int button, int action)
	{
		if (action == GLFW_PRESS)
		{
			pre_x_ = current_x_;
			pre_y_ = current_y_;
			is_left_button_down_ = (button == GLFW_MOUSE_BUTTON_LEFT);
			is_right_button_down_ = (button == GLFW_MOUSE_BUTTON_RIGHT);
			is_middle_button_down_ = (button == GLFW_MOUSE_BUTTON_MIDDLE);

			if (is_left_button_down_)
			{
				if (object_)
				{
					intersect_list.clear();
					UMGUIObject::intersect(object_, intersect_list, current_x_, current_y_);
					if (!intersect_list.empty())
					{
						UMGUIObjectList::const_reverse_iterator it = intersect_list.rbegin();
						for (; it != intersect_list.rend(); ++it)
						{
							(*it)->on_left_button_down(current_x_, current_y_);
						}
					}
				}
			}
		}
		else
		{
			if (button == GLFW_MOUSE_BUTTON_LEFT) 
			{
				is_left_button_down_ = false;
				
				if (!intersect_list.empty())
				{
					UMGUIObjectList::const_reverse_iterator it = intersect_list.rbegin();
					for (; it != intersect_list.rend(); ++it)
					{
						(*it)->on_left_button_up(current_x_, current_y_);
					}
				}
			}
			if (button == GLFW_MOUSE_BUTTON_RIGHT)
			{
				is_right_button_down_ = false;
			}
			if (button == GLFW_MOUSE_BUTTON_MIDDLE)
			{
				is_middle_button_down_ = false;
			}
		}
		return false;
	}

	/**
	 * mouse move
	 */
	bool on_mouse_move(double x, double y)
	{
		current_x_ = x;
		current_y_ = y;

		if (!intersect_list.empty())
		{
			if (is_left_button_down_)
			{
				UMGUIObjectList::const_reverse_iterator it = intersect_list.rbegin();
				for (; it != intersect_list.rend(); ++it)
				{
					(*it)->on_left_button_move(current_x_, current_y_);
				}
			}
		}
		return false;
	}

	/**
	 * scroll
	 */
	bool on_scroll(double x, double y)
	{
		if (intersect_list.empty())
		{
			UMGUIObject::intersect(object_, intersect_list, current_x_, current_y_);
		}

		if (!intersect_list.empty())
		{
			UMGUIObjectList::const_reverse_iterator it = intersect_list.rbegin();
			for (; it != intersect_list.rend(); ++it)
			{
				(*it)->on_scroll(x, y);
			}
			intersect_list.clear();
			return true;
		}
		return false;
	}

private:
	UMGUIObjectList intersect_list;
	double pre_x_;
	double pre_y_;
	double current_x_;
	double current_y_;
	bool is_ctrl_button_down_;
	bool is_left_button_down_;
	bool is_right_button_down_;
	bool is_middle_button_down_;
	bool is_alt_down_;
	bool is_shift_down_;

	umdraw::UMSceneWeakPtr scene_;
	UMGUIObjectPtr object_;
	UMCameraPtr camera_;
	UMLightPtr light_;
};

/**
 * constructor
 */
UMGUIScene::UMGUIScene() :
	impl_(new UMGUIScene::SceneImpl())
{
}

/**
 * destructor
 */
UMGUIScene::~UMGUIScene()
{
}

/** 
 * set scene
 */
void UMGUIScene::set_umdraw_scene(umdraw::UMScenePtr scene)
{
	impl_->set_umdraw_scene(scene);
}

/**
 * get umdraw scene
 */
umdraw::UMScenePtr UMGUIScene::umdraw_scene()
{
	return impl_->umdraw_scene();
}

/**
 * initialize
 */
bool UMGUIScene::init(int width, int height)
{
	return impl_->init(width, height);
}

/**
 * get camera
 */
UMCameraPtr UMGUIScene::camera()
{
	return impl_->camera();
}

/**
 * get light
 */
UMLightPtr UMGUIScene::light()
{
	return impl_->light();
}


/**
 * get root object
 */
UMGUIObjectPtr UMGUIScene::root_object()
{
	return impl_->root_object();
}

/**
 * keyboard
 */
bool UMGUIScene::on_keyboard(int key, int action)
{
	return impl_->on_keyboard(key, action);
}

/**
 * mouse button up/down
 */
bool UMGUIScene::on_mouse(int button, int action)
{
	return impl_->on_mouse(button, action);
}

/**
 * mouse move
 */
bool UMGUIScene::on_mouse_move(double x, double y)
{
	return impl_->on_mouse_move(x, y);
}

/**
 * mouse move
 */
bool UMGUIScene::on_scroll(double x, double y)
{
	return impl_->on_scroll(x, y);
}

} // UMGUI
