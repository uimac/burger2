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
#include "UMResource.h"
#include "UMScene.h"
#include "UMCamera.h"

namespace umgui
{
	using namespace umdraw;
	using namespace umresource;

class UMGUIScene::SceneImpl
{
	DISALLOW_COPY_AND_ASSIGN(SceneImpl);
public:

	SceneImpl() {}
	~SceneImpl() {
	}
	
	/**
	 * initialize
	 */
	bool init(int width, int height)
	{
		UMImagePtr test_image;
		{
			const std::string& data = UMResource::find_resource_data(UMResource::instance(), "test.jpg");
			test_image = UMImage::load_from_memory(data);
			test_image = test_image->create_flip_image(false, true);
		}

		UMImagePtr cabbage_image;
		{
			const std::string& data = UMResource::find_resource_data(UMResource::instance(), "cabbage.png");
			cabbage_image = UMImage::load_from_memory(data);
			cabbage_image = cabbage_image->create_flip_image(false, true);
		}
		
		UMImagePtr pen_icon_image;
		{
			const std::string& data = UMResource::find_resource_data(UMResource::instance(), "pen.png");
			pen_icon_image = UMImage::load_from_memory(data);
			pen_icon_image = pen_icon_image->create_flip_image(false, true);
		}

		UMImagePtr hoge_icon_image;
		{
			const std::string& data = UMResource::find_resource_data(UMResource::instance(), "hoge.png");
			hoge_icon_image = UMImage::load_from_memory(data);
			hoge_icon_image = hoge_icon_image->create_flip_image(false, true);
		}

		UMImagePtr piyo_icon_image;
		{
			const std::string& data = UMResource::find_resource_data(UMResource::instance(), "piyo.png");
			piyo_icon_image = UMImage::load_from_memory(data);
			piyo_icon_image = piyo_icon_image->create_flip_image(false, true);
		}

		UMImagePtr round_rect_image;
		{
			const std::string& data = UMResource::find_resource_data(UMResource::instance(), "round_rect.png");
			round_rect_image = UMImage::load_from_memory(data);
			round_rect_image = round_rect_image->create_flip_image(false, true);
		}
		object_ = UMGUIBoard::create_root_board(width, height);
		if (object_)
		{
#ifdef WITH_FREETYPE
			// menu back
			UMGUIBoardPtr menu_back = std::make_shared<UMGUIBoard>(-100);
			menu_back->add_color_panel(width, height, 0, 0, width, 20, UMVec4d(0.1, 0.1, 0.1, 1.0));
			object_->mutable_children().push_back(menu_back);
			{
				// menu front
				UMGUIBoardPtr menu_front = std::make_shared<UMGUIBoard>(-10);
				menu_back->mutable_children().push_back(menu_front);
				{
					// icon
					menu_front->add_texture_panel(width, height, 5, 2, 16, 16, cabbage_image);
					// menus
					menu_front->add_text_panel(width, height, 30, 4, 12, L"ファイル");
					menu_front->add_text_panel(width, height, 100, 4, 12, L"ウィンドウ");
					menu_front->add_text_panel(width, height, 180, 4, 12, L"ヘルプ");
				}
			}
#endif // WITH_FREETYPE

			// left
			UMGUIBoardPtr left_board = UMGUIBoardPtr(new UMGUIBoard(-100));
			left_board->add_color_panel(width, height, 10, 50, 280, height-60, UMVec4d(0.1, 0.1, 0.1, 0.9));
			object_->mutable_children().push_back(left_board);
			{
				UMGUIBoardPtr left_board_front = UMGUIBoardPtr(new UMGUIBoard(-10));
				left_board->mutable_children().push_back(left_board_front);
				{
					// text text board
#ifdef WITH_FREETYPE
					left_board_front->add_text_panel(width, height, 15, 50, 14, L"WebGLテスト");
#endif // WITH_FREETYPE

					left_board_front->add_texture_panel(width, height, 80, 80, 20, 20, round_rect_image);
#ifdef WITH_FREETYPE
					left_board_front->add_text_panel(width, height, 110, 82, 14, L"チェックボックス(予定)");
#endif // WITH_FREETYPE

					// pen icon
					left_board_front->add_texture_panel(width, height, 15, 75 * 1, 32, 32, pen_icon_image);
					// hoge icon
					left_board_front->add_texture_panel(width, height, 15, 75 * 2, 32, 32, hoge_icon_image);
					// piyo icon
					left_board_front->add_texture_panel(width, height, 15, 75 * 3, 32, 32, piyo_icon_image);
					// miku
					left_board_front->add_texture_panel(width, height, 15, 75 * 4, 80, 50, test_image);
				}
				// color circle
				UMGUIBoardPtr color_circle = UMGUIBoard::create_color_circle_board(width, height, 30, 380, 150, 150, -10);
				left_board->mutable_children().push_back(color_circle);
			}
		}
			
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

private:
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

} // UMGUI
