/**
 * @file UMViewer.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */

#ifdef WITH_EMSCRIPTEN
	#include <GLES2/gl2.h>
	#include <EGL/egl.h>
#else
	#include <GL/glew.h>
#endif

#include "UMViewer.h"
#include "UMCamera.h"
#include "UMAbc.h"
#include "UMAbcIO.h"
#include "UMAbcCallback.h"
#include "UMAbcScene.h"

#include "UMStringUtil.h"
#include "UMPath.h"
#include "UMRT.h"

#include <windows.h>
#include <map>
#include <fstream>
#include <string>
#include <iterator>
#include <iostream>
#include <tchar.h>
#include <thread>

#include <windows.h>
#include <mmsystem.h>
#include <GL/glfw3.h>
#include <GL/glfw3native.h>

namespace test_viewer
{
	using namespace umdraw;
	using namespace umabc;
	
int UMViewer::width_(0);
int UMViewer::height_(0);
bool UMViewer::is_disable_update_(false);
GLFWwindow* UMViewer::sub_window_(NULL);
UMScenePtr UMViewer::scene_;
UMCameraPtr UMViewer::temporary_camera_;
UMViewerPtr UMViewer::viewer_;
UMAbcSceneList UMViewer::abc_scene_list_;

class UMFileLoadThread
{
public:
	UMFileLoadThread() : is_loaded_(false) {}
	~UMFileLoadThread() 
	{
		if (thread_.joinable())
		{
			thread_.join();
		}
	}

	void load(
		GLFWwindow * window,
		int count,
		const char** files,
		UMViewerPtr viewer,
		umdraw::UMScenePtr scene,
		umabc::UMAbcSceneList& abc_scene_list)
	{
		window_ = window;
		count_ = count;
		files_ = files;
		viewer_ = viewer;
		scene_ = scene;
		abc_scene_list_ = abc_scene_list;
		thread_ = std::thread([this] { do_(); });
	}

	void done()
	{
		if (thread_.joinable())
		{
			thread_.join();
		}
		viewer_->file_loaded_callback(window_);
		window_ = NULL;
		is_loaded_ = false;
		count_ = 0;
		files_ = NULL;
		viewer_ = UMViewerPtr();
		scene_ = umdraw::UMScenePtr();
		abc_scene_list_.clear();
	}

	bool is_loaded() const { return is_loaded_; }

	const UMAbcSceneList& abc_scene_list() { return abc_scene_list_; }
private:

	void do_()
	{
		umabc::UMAbcIO abcio;
		umabc::UMAbcSetting setting;
		setting.set_reference_scene(scene_);
		for (int i = 0; i < count_; ++i)
		{
			printf("%d: '%s'\n", i + 1, files_[i]);
			std::string utf8path(files_[i]);
			std::u16string path = umbase::UMStringUtil::utf8_to_utf16(utf8path);
			if (scene_->load(path))
			{
				is_loaded_ = true;
			}
			else
			{
				if (umabc::UMAbcScenePtr abc_scene = abcio.load(path, setting))
				{
					abc_scene_list_.push_back(abc_scene);
					is_loaded_ = true;
				}
			}
		}
	}
	GLFWwindow* window_;
	std::thread thread_;
	bool is_loaded_;
	int count_;
	const char** files_;
	UMViewerPtr viewer_;
	umdraw::UMScenePtr scene_;
	umabc::UMAbcSceneList abc_scene_list_;
};

static UMFileLoadThread load_thread;

bool UMViewer::init(
	GLFWwindow* window,
	GLFWwindow* sub_window, 
	UMScenePtr scene, 
	UMAbcSceneList& abc_scene_list,
	UMDraw::DrawType type, 
	int width, 
	int height)
{
	if (!scene) return false;

	UMViewer::width_ = width;
	UMViewer::height_ = height;

	sub_window_ = sub_window;
	scene_ = scene;
	abc_scene_list_ = abc_scene_list;
	viewer_ = create(window, sub_window, scene, abc_scene_list, type);
	
	if (viewer_)
	{
		return true;
	}
	return false;
}

UMViewerPtr UMViewer::create(
	GLFWwindow* window,
	GLFWwindow* sub_window, 
	UMScenePtr scene, 
	UMAbcSceneList& abc_scene_list, 
	UMDraw::DrawType type)
{
	HWND hwnd = glfwGetWin32Window(window);
	if (type == UMDraw::eOpenGL)
	{
		UMDrawPtr drawer = UMDraw::create(UMDraw::eOpenGL);
		UMAbcPtr abc = UMAbc::create(UMAbc::eOpenGL);
		
		if (drawer->init(hwnd, scene))
		{
			drawer->resize(UMViewer::width_, UMViewer::height_);
			if (abc) abc->init(abc_scene_list);
			return UMViewerPtr(new UMViewer(drawer, abc));
		}
	}
	if (type == UMDraw::eDirectX)
	{
		UMDrawPtr drawer = UMDraw::create(UMDraw::eDirectX);
		UMAbcPtr abc = UMAbc::create(UMAbc::eDirectX);

		if (drawer->init(hwnd, scene))
		{
			drawer->resize(UMViewer::width_, UMViewer::height_);
			if (abc) abc->init(abc_scene_list);
			return UMViewerPtr(new UMViewer(drawer, abc));
		}
	}
	return UMViewerPtr();
}

void UMViewer::call_paint()
{
	if (!viewer_) return;
	viewer_->on_paint();

	if (load_thread.is_loaded())
	{
		abc_scene_list_ = load_thread.abc_scene_list();
		load_thread.done();
	}
}

void UMViewer::key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	if (!viewer_) return;
	viewer_->on_keyboard(window, key, action);

	// change viewer
	if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
	{
		if (draw_type() == UMDraw::eDirectX) 
		{
			// change to gl
			viewer_->close_view();
			viewer_ = test_viewer::UMViewerPtr();
			viewer_ = create(window, sub_window_, scene_, abc_scene_list_, UMDraw::eOpenGL);
		}
		else
		{
			// change to dx_drawer_
			viewer_->close_view();
			viewer_ = test_viewer::UMViewerPtr();
			viewer_ = create(window, sub_window_, scene_, abc_scene_list_, UMDraw::eDirectX);
		}
	}
}

void UMViewer::mouse_button_callback(GLFWwindow * window, int button, int action, int mods)
{
	if (!viewer_) return;
	viewer_->on_mouse(window, button, action);
}

void UMViewer::cursor_pos_callback(GLFWwindow * window, double x, double y)
{
	if (!viewer_) return;
	viewer_->on_mouse_move(window, x, y);
}

void UMViewer::window_size_callback(GLFWwindow * window, int width, int height)
{
	if (!viewer_) return;
	viewer_->on_resize(window, width, height);
}

void UMViewer::window_close_callback(GLFWwindow * window)
{
	if (!viewer_) return;
	viewer_->on_close(window);
	viewer_ = test_viewer::UMViewerPtr();
	scene_ = UMScenePtr();
}


void UMViewer::file_loaded_callback(GLFWwindow * window)
{
	is_disable_update_ = true;
	const UMDraw::DrawType type = UMViewer::draw_type();
	viewer_->close_view();
	viewer_ = test_viewer::UMViewerPtr();
	viewer_ = UMViewer::create(window, sub_window_, scene_, abc_scene_list_, type);

	size_t polygons = 0;
	if (scene_)
	{
		polygons += scene_->total_polygon_size();
	}
	for (size_t i = 0, size = abc_scene_list_.size(); i < size; ++i)
	{
		polygons += abc_scene_list_.at(i)->total_polygon_size();
	}
	printf("%d polygons\n", polygons);
	is_disable_update_ = false;
}

void UMViewer::drop_files_callback(GLFWwindow * window, int count, const char** files)
{
	load_thread.load(window, count, files, viewer_, scene_, abc_scene_list_);
}

UMDraw::DrawType UMViewer::draw_type()
{
	if (!viewer_) return UMDraw::eSoftware;
	if (!viewer_->drawer_) return UMDraw::eSoftware;
	return viewer_->drawer_->draw_type();
}

/**
 * constructor
 */
UMViewer::UMViewer(UMDrawPtr drawer, UMAbcPtr abc)
	: pre_x_(0.0)
	, pre_y_(0.0)
	, current_x_(0.0)
	, current_y_(0.0)
	, current_frames_(0)
	, fps_base_time_(timeGetTime())
	, motion_base_time_(timeGetTime())
	, is_left_button_down_(false)
	, is_right_button_down_(false)
	, is_ctrl_button_down_(false)
	, is_middle_button_down_(false)
	, is_alt_down_(false)
	, drawer_(drawer)
	, abc_(abc)
	, current_seconds_(0.0)
	, rays_(std::make_shared<umrt::UMRT>())
{
	rays_->add_scene(scene_);
	if (!abc_scene_list_.empty())
	{
		for (size_t i = 0, size = abc_scene_list_.size(); i < size; ++i)
		{
			rays_->add_abc_scene(abc_scene_list_.at(i));
		}
	}
}

/**
 * refresh frame
 */
bool UMViewer::on_paint()
{
	if (is_disable_update_) return true;
	
	++current_frames_;
	unsigned long current_time = static_cast<unsigned long>(timeGetTime());
	unsigned long time_from_fps_base = current_time - fps_base_time_;
	unsigned long time_from_motion_base = current_time - motion_base_time_;
	if (time_from_fps_base > 1000)
	{
		int fps = (current_frames_ * 1000) / time_from_fps_base;
		fps_base_time_ = current_time;
		current_frames_ = 0;
		printf("fps %d \n", fps);
	}

	if (drawer_->clear())
	{
		if (drawer_->update())
		{
			drawer_->draw();
		}
		if (abc_)
		{
			if (abc_->update(time_from_motion_base))
			{
				abc_->draw();
			}
		}
		return true;
	}
	return false;
}

/**
 * keyboard
 */
void UMViewer::on_keyboard(GLFWwindow * window,int key, int action)
{
	if (key == GLFW_KEY_RIGHT_CONTROL)
	{
		is_ctrl_button_down_ = true;
	}
	else if (key == GLFW_KEY_ENTER)
	{
		if (rays_)
		{
			// subdiv test
			if (rays_->subdiv_test())
			{
				// refresh gl viewer
				viewer_->close_view();
				viewer_ = test_viewer::UMViewerPtr();
				HWND hwnd = glfwGetWin32Window(window);
				viewer_ = create(window, sub_window_, scene_, abc_scene_list_, UMDraw::eOpenGL);
			}

			//rays_->add_scene(scene_);
			//if (!abc_scene_list_.empty())
			//{
			//	for (size_t i = 0, size = abc_scene_list_.size(); i < size; ++i)
			//	{
			//		rays_->add_abc_scene(abc_scene_list_.at(i));
			//	}
			//}

			// render test
			//if (umimage::UMImagePtr image = rays_->render()->create_flip_image(false, true))
			//{
			//	std::u16string save_path = umbase::UMPath::resource_absolute_path(
			//		umbase::UMStringUtil::utf8_to_utf16("test.bmp"));
			//	if (UMImage::save(save_path, image, umimage::UMImage::eImageTypeBMP_RGB))
			//	{
			//		printf("image save success\n");
			//	}
			//}
		}
	}
	else if (key == GLFW_KEY_LEFT_ALT)
	{
		if (action == GLFW_PRESS)
		{
			is_alt_down_ = true;
		}
		else if (action == GLFW_RELEASE)
		{
			is_alt_down_ = false;
		}
	}
	else if (key == GLFW_KEY_A)
	{
		if (action == GLFW_RELEASE)
		{
			// change to abc camera if exists
			if (scene_ && !abc_scene_list_.empty())
			{
				if (!temporary_camera_)
				{
					temporary_camera_ = scene_->camera();
				}
				if (umdraw::UMCameraPtr camera = abc_scene_list_.at(0)->umdraw_camera(std::u16string()))
				{
					scene_->set_camera(abc_scene_list_.at(0)->umdraw_camera(std::u16string()));
				}
			}
		}
	}
	else if (key == GLFW_KEY_B)
	{
		if (action == GLFW_RELEASE)
		{
			// change to abc camera if exists
			if (scene_ && temporary_camera_)
			{
				scene_->set_camera(temporary_camera_);
				temporary_camera_ = umdraw::UMCameraPtr();
			}
		}
	}
}

/**
 * mouse button up/down
 */
void UMViewer::on_mouse(GLFWwindow * window, int button, int action)
{
	if (action == GLFW_PRESS)
	{
		pre_x_ = current_x_;
		pre_y_ = current_y_;
		is_left_button_down_ = (button == GLFW_MOUSE_BUTTON_LEFT);
		is_right_button_down_ = (button == GLFW_MOUSE_BUTTON_RIGHT);
		is_middle_button_down_ = (button == GLFW_MOUSE_BUTTON_MIDDLE);
	}
	else
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT) 
		{
			is_left_button_down_ = false;
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
}

/**
 * mouse move
 */
void UMViewer::on_mouse_move(GLFWwindow *, double x, double y)
{
	UMScenePtr scene = drawer_->scene();
	
	current_x_ = x;
	current_y_ = y;

	if (is_left_button_down_)
	{
		if (is_alt_down_)
		{
			if (UMCameraPtr camera = scene->camera())
			{
				camera->rotate(pre_x_ - x, pre_y_ - y);
				pre_x_ = x;
				pre_y_ = y;
			}
		}
	}

	if (is_right_button_down_)
	{
		if (UMCameraPtr camera = scene->camera())
		{
			if (is_ctrl_button_down_)
			{
				camera->zoom(pre_x_ - x, pre_y_ - y);
				pre_x_ = x;
				pre_y_ = y;
			}
			else
			{
				camera->dolly(pre_x_ - x, pre_y_ - y);
				pre_x_ = x;
				pre_y_ = y;
			}
		}
	}
	if (is_middle_button_down_)
	{
		if (UMCameraPtr camera = scene->camera())
		{
			camera->pan(pre_x_ - x, pre_y_ - y);
			pre_x_ = x;
			pre_y_ = y;
		}
	}
}

/**
 * resize
 */
void UMViewer::on_resize(GLFWwindow *, int width, int height)
{
	if (drawer_)
	{
		drawer_->resize(width, height);
	}
	UMViewer::width_ = width;
	UMViewer::height_ = height;
}

void UMViewer::close_view()
{
	abc_ = UMAbcPtr();
	drawer_ = UMDrawPtr();
}

/**
 * window close
 */
void UMViewer::on_close(GLFWwindow * window)
{
	if (abc_) { abc_->dispose(); }
	abc_ = UMAbcPtr();
	drawer_ = UMDrawPtr();
}

} // test_viewer
