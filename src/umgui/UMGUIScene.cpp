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
#include <tchar.h>
#include "UMGUIObject.h"
#include "UMStringUtil.h"
#include "UMPath.h"
#include "UMMaterial.h"
#include "UMGUIBoard.h"
#include "UMResource.h"
#include "UMScene.h"

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
		const std::u16string path = umbase::UMPath::resource_absolute_path(
			umbase::UMStringUtil::utf8_to_utf16("test.jpg"));

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

		std::u16string test = umbase::UMStringUtil::wstring_to_utf16(_T("ペイントテスト(｡ŏ__ŏ)"));
		std::u16string check_box_text = umbase::UMStringUtil::wstring_to_utf16(_T("チェックボックス"));
		if (object_ = UMGUIBoard::create_root_board(width, height))
		{
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
					std::u16string file = umbase::UMStringUtil::wstring_to_utf16(_T("ファイル"));
					std::u16string window = umbase::UMStringUtil::wstring_to_utf16(_T("ウィンドウ"));
					std::u16string help = umbase::UMStringUtil::wstring_to_utf16(_T("ヘルプ"));
					menu_front->add_text_panel(width, height, 30, 4, 12, file);
					menu_front->add_text_panel(width, height, 100, 4, 12, window);
					menu_front->add_text_panel(width, height, 180, 4, 12, help);
				}
			}

			// left
			UMGUIBoardPtr left_board = std::make_shared<UMGUIBoard>(-100);
			left_board->add_color_panel(width, height, 10, 50, 280, height-60, UMVec4d(0.1, 0.1, 0.1, 0.9));
			object_->mutable_children().push_back(left_board);
			{
				UMGUIBoardPtr left_board_front = std::make_shared<UMGUIBoard>(-10);
				left_board->mutable_children().push_back(left_board_front);
				{
					// text text board
					left_board_front->add_text_panel(width, height, 15, 50, 12, test);

					left_board_front->add_texture_panel(width, height, 80, 80, 20, 20, round_rect_image);
					left_board_front->add_text_panel(width, height, 110, 82, 14, check_box_text);

					// pen icon
					left_board_front->add_texture_panel(width, height, 15, 70 * 1, 32, 32, pen_icon_image);
					// hoge icon
					left_board_front->add_texture_panel(width, height, 15, 70 * 2, 32, 32, hoge_icon_image);
					// piyo icon
					left_board_front->add_texture_panel(width, height, 15, 70 * 3, 32, 32, piyo_icon_image);
					// miku
					left_board_front->add_texture_panel(width, height, 15, 70 * 4, 80, 50, path);
				}
				// color circle
				UMGUIBoardPtr color_circle = UMGUIBoard::create_color_circle_board(width, height, 30, 350, 150, 150, -10);
				left_board->mutable_children().push_back(color_circle);
			}
			
			if (camera_ = std::make_shared<UMCamera>(true, width, height))
			{
				if (light_ = std::make_shared<UMLight>(UMVec3d(0, 0, 1000)))
				{
					return true;
				}
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
