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
	#include <GL/glew.h>
	//#include <GLES2/gl2.h>
	//#include <EGL/egl.h>
#else
	#include <GL/glew.h>
#endif

#include "UMViewer.h"
#include "UMCamera.h"
#include "UMAbc.h"
#include "UMAbcIO.h"
#include "UMAbcCallback.h"
#include "UMAbcScene.h"
#include "UMBurgerGUI.h"

#include "UMStringUtil.h"
#include "UMPath.h"
#include "UMRT.h"
#include "UMTime.h"
#include "UMSceneAccess.h"

#ifdef WITH_WSIO
	#include "UMWSIO.h"
	#include "UMWSIOEventType.h"
#endif

#if !defined(WITH_EMSCRIPTEN)
	#include <windows.h>
	#include <tchar.h>
	#include <mmsystem.h>
#endif

#include <map>
#include <fstream>
#include <string>
#include <iterator>
#include <iostream>
#include <thread>
#include <memory>

#include <GL/glfw3.h>

namespace burger
{
	using namespace umbase;
	using namespace umdraw;
	using namespace umabc;
	using namespace umgui;
	
static bool is_raytracing_camp_ = true;

int UMViewer::width_(0);
int UMViewer::height_(0);
bool UMViewer::is_disable_update_(false);
bool UMViewer::is_gui_drawing_(false);
GLFWwindow* UMViewer::sub_window_(NULL);
GLFWwindow* UMViewer::window_(NULL);
UMScenePtr UMViewer::scene_;
UMCameraPtr UMViewer::temporary_camera_;
UMBurgerGUIPtr UMViewer::gui_scene_;
UMViewerPtr UMViewer::viewer_;

#ifdef WITH_WSIO
	static umwsio::UMWSIOPtr wsio_;
#endif

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
			umstring path = umbase::UMStringUtil::utf8_to_utf16(utf8path);
			
			if (umabc::UMAbcScenePtr abc_scene = abcio.load(path, setting))
			{
				abc_scene_list_.push_back(abc_scene);
				is_loaded_ = true;
			}
			if (scene_->load(path))
			{
				is_loaded_ = true;
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
	UMBurgerGUIPtr gui_scene,
	UMAbcSceneList& abc_scene_list,
	UMDraw::DrawType type, 
	int width, 
	int height)
{
	if (!scene) return false;

	UMViewer::width_ = width;
	UMViewer::height_ = height;

	window_ = window;
	sub_window_ = sub_window;
	scene_ = scene;
	gui_scene_ = gui_scene;
	abc_scene_list_ = abc_scene_list;
	gui_scene->set_umdraw_scene(scene);
	viewer_ = create(window, sub_window, scene, gui_scene, abc_scene_list, type);
	
	if (viewer_)
	{
		if (is_raytracing_camp_) {
			viewer_->um30min_ = std::make_shared<UM30min>(viewer_);
		}
		return true;
	}
	return false;
}

UMViewerPtr UMViewer::create(
	GLFWwindow* window,
	GLFWwindow* sub_window, 
	UMScenePtr scene, 
	UMBurgerGUIPtr gui_scene, 
	UMAbcSceneList& abc_scene_list, 
	UMDraw::DrawType type)
{
	void* hwnd = NULL;
	UMViewerPtr viewer;

	if (type == UMDraw::eOpenGL)
	{
		UMDrawPtr drawer = UMDraw::create(UMDraw::eOpenGL);
		UMAbcPtr abc = UMAbc::create(UMAbc::eOpenGL);
		UMGUIPtr gui;
		if (gui_scene) gui = UMGUI::create(UMGUI::eOpenGL);
		
		if (drawer->init(hwnd, scene))
		{
			drawer->resize(UMViewer::width_, UMViewer::height_);
			if (abc) abc->init(abc_scene_list);
			if (gui) gui->init(gui_scene);
			viewer = UMViewerPtr(new UMViewer(drawer, gui, abc));
		}
	}
	if (type == UMDraw::eDirectX)
	{
		UMDrawPtr drawer = UMDraw::create(UMDraw::eDirectX);
		UMAbcPtr abc = UMAbc::create(UMAbc::eDirectX);
		UMGUIPtr gui;
		if (gui_scene) gui = UMGUI::create(UMGUI::eDirectX);

		if (drawer->init(hwnd, scene))
		{
			drawer->resize(UMViewer::width_, UMViewer::height_);
			if (abc) abc->init(abc_scene_list);
			if (gui) gui->init(gui_scene);
			viewer = UMViewerPtr(new UMViewer(drawer, gui, abc));
		}
	}
	
	//viewer->mutable_event_list().push_back(viewer->wsio_->model_loaded_event());
	//viewer->connect(viewer->shared_from_this());

	return viewer;
}

bool UMViewer::call_paint()
{
	if (!viewer_) return false;
	if (!viewer_->on_paint()) return false;

	if (load_thread.is_loaded())
	{
		abc_scene_list_ = load_thread.abc_scene_list();
		load_thread.done();
	}

#ifdef WITH_WSIO
	if (wsio_->is_loaded())
	{
		wsio_->done();
		viewer_->file_loaded_callback(sub_window_);
	}
#endif // WITH_WSIO

	return true;
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
			viewer_ = burger::UMViewerPtr();
			viewer_ = create(window, sub_window_, scene_, gui_scene_, abc_scene_list_, UMDraw::eOpenGL);
		}
		else
		{
			// change to dx_drawer_
			viewer_->close_view();
			viewer_ = burger::UMViewerPtr();
			viewer_ = create(window, sub_window_, scene_, gui_scene_, abc_scene_list_, UMDraw::eDirectX);
		}
	}
}

void UMViewer::mouse_button_callback(GLFWwindow * window, int button, int action, int mods)
{
	if (!viewer_) return;
	viewer_->on_mouse(window, button, action);
}

void UMViewer::scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
	if (!viewer_) return;
	viewer_->on_scroll(window, xoffset, yoffset);
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
#ifdef WITH_WSIO
	wsio_ = umwsio::UMWSIOPtr();
#endif // WITH_WSIO
	viewer_->on_close(NULL);
	viewer_ = burger::UMViewerPtr();
	scene_ = UMScenePtr();
}


void UMViewer::file_loaded_callback(GLFWwindow * window)
{
	is_disable_update_ = true;
	const UMDraw::DrawType type = UMViewer::draw_type();
	viewer_->close_view();
	viewer_ = burger::UMViewerPtr();
	gui_scene_->set_umdraw_scene(scene_);
	viewer_ = UMViewer::create(window, sub_window_, scene_, gui_scene_, abc_scene_list_, type);

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
UMViewer::UMViewer(UMDrawPtr drawer, UMGUIPtr gui, UMAbcPtr abc)
	: pre_x_(0.0)
	, pre_y_(0.0)
	, current_x_(0.0)
	, current_y_(0.0)
	, current_frames_(0)
	, fps_base_time_(UMTime::current_time())
	, motion_base_time_(UMTime::current_time())
	, is_left_button_down_(false)
	, is_right_button_down_(false)
	, is_ctrl_button_down_(false)
	, is_middle_button_down_(false)
	, is_alt_down_(false)
	, is_shift_down_(false)
	, drawer_(drawer)
	, abc_(abc)
	, gui_(gui)
	, current_seconds_(0.0)
	, rays_(std::make_shared<umrt::UMRT>())
	, current_frame_(0)
	, is_realtime_animation_(false)
{
	rays_->add_scene(scene_);

#ifdef WITH_WSIO
	if (!wsio_)
	{
		wsio_ = std::make_shared<umwsio::UMWSIO>();
		wsio_->start_server(scene_);
	}
#endif

#ifdef WITH_ALEMBIC
	if (!abc_scene_list_.empty())
	{
		for (size_t i = 0, size = abc_scene_list_.size(); i < size; ++i)
		{
			rays_->add_abc_scene(abc_scene_list_.at(i));
		}
	}
#endif
}

/**
 * update event
 */
void UMViewer::update(umbase::UMEventType event_type, umbase::UMAny& parameter)
{
#ifdef WITH_WSIO
	if (event_type == umwsio::eWSIOEventModelLoaded)
	{
		file_loaded_callback(sub_window_);
	}
#endif // WITH_WSIO
}

/**
 * refresh frame
 */
bool UMViewer::on_paint()
{
	if (is_disable_update_) return true;
	
	unsigned long update_time = 0;
	if (is_realtime_animation_)
	{
		++current_frames_;
		unsigned long current_time = static_cast<unsigned long>(UMTime::current_time());
		unsigned long time_from_fps_base = current_time - fps_base_time_;
		unsigned long time_from_motion_base = current_time - motion_base_time_;
		update_time = time_from_motion_base;
	}
	else
	{
		update_time = current_frame_ * (1.0 / 30.0) * 1000.0;
	}

	//if (time_from_fps_base > 1000)
	//{
	//	int fps = (current_frames_ * 1000) / time_from_fps_base;
	//	fps_base_time_ = current_time;
	//	current_frames_ = 0;
	//	printf("fps %d \n", fps);
	//}
	if (um30min_ && !um30min_->update())
	{
		return false;
	}

	if (drawer_->clear())
	{
		if (drawer_->update())
		{
			drawer_->draw();
		}
#ifdef WITH_ALEMBIC
		if (abc_)
		{
			if (abc_->update(update_time))
			{
				abc_->draw();
			}
		}
#endif // WITH_ALEMBIC
		if (is_gui_drawing_)
		{
			if (gui_ && gui_->update())
			{
				gui_->draw();
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
	UMScenePtr scene = drawer_->scene();
	if (!scene) return;
	UMCameraPtr camera = scene->camera();
	if (!camera) return;
	
	if (gui_ && is_gui_drawing_)
	{
		if (gui_->on_keyboard(key, action))
		{
			return;
		}
	}
	if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL)
	{
		if (action == GLFW_PRESS)
		{
			is_ctrl_button_down_ = true;
		}
		else if (action == GLFW_RELEASE)
		{
			is_ctrl_button_down_ = false;
		}
	}
	else if (key == GLFW_KEY_ENTER)
	{
		if (action == GLFW_PRESS)
		{
			if (scene_->foreground_image())
			{
				scene_->set_foreground_image(UMImagePtr());
			}
			else
			{
				// render test
				render();
			}
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
	else if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT)
	{
		if (action == GLFW_PRESS)
		{
			is_shift_down_ = true;
		}
		else if (action == GLFW_RELEASE)
		{
			is_shift_down_ = false;
		}
	}
	else if (key == GLFW_KEY_KP_0 && action == GLFW_PRESS)
	{
		camera->init(camera->is_ortho(), scene->width(), scene->height());
	}
	else if (key == GLFW_KEY_KP_2 && action == GLFW_PRESS)
	{
		camera->rotate(0, 15);
	}
	else if (key == GLFW_KEY_KP_8 && action == GLFW_PRESS)
	{
		camera->rotate(0, -15);
	}
	else if (key == GLFW_KEY_KP_4 && action == GLFW_PRESS)
	{
		camera->rotate(-15, 0);
	}
	else if (key == GLFW_KEY_KP_6 && action == GLFW_PRESS)
	{
		camera->rotate(15, 0);
	}
	else if (key == GLFW_KEY_KP_1 && action == GLFW_PRESS)
	{
		umbase::UMVec3d pos = camera->position();
		camera->init(camera->is_ortho(), scene->width(), scene->height());
		camera->set_position(pos);
		camera->rotate(-3, 0);
		camera->rotate(3, 0);
	}
	else if (key == GLFW_KEY_KP_3 && action == GLFW_PRESS)
	{
		umbase::UMVec3d pos = camera->position();
		camera->init(camera->is_ortho(), scene->width(), scene->height());
		camera->set_position(pos);
		if (is_shift_down_)
		{
			camera->rotate(-90, 0);
		}
		else
		{
			camera->rotate(90, 0);
		}
	}
	else if (key == GLFW_KEY_KP_7 && action == GLFW_PRESS)
	{
		umbase::UMVec3d pos = camera->position();
		camera->init(camera->is_ortho(), scene->width(), scene->height());
		camera->set_position(pos);
		if (is_shift_down_)
		{
			camera->rotate(0, 89.9);
		}
		else
		{
			camera->rotate(0, -89.9);
		}
	}
	else if (key == GLFW_KEY_TAB)
	{
		if (action == GLFW_PRESS)
		{
			is_gui_drawing_ = !is_gui_drawing_;
		}
	}
	else if (key == GLFW_KEY_A)
	{
		if (action == GLFW_RELEASE)
		{
			change_to_abc_camera();
		}
	}
	else if (key == GLFW_KEY_B)
	{
		if (action == GLFW_RELEASE)
		{
			// revert camera if exists
			if (scene_ && temporary_camera_)
			{
				scene_->mutable_camera_list().at(0) = temporary_camera_;
				temporary_camera_ = umdraw::UMCameraPtr();
			}
		}
	}
	else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
	{
		set_current_frame(current_frame_ - 1);
		on_paint();
	}
	else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
	{
		set_current_frame(current_frame_ + 1);
		on_paint();
	}
}

void UMViewer::change_to_abc_camera()
{
	// change to abc camera if exists
	if (scene_ && !abc_scene_list_.empty())
	{
		if (!temporary_camera_)
		{
			temporary_camera_ = scene_->camera();
		}
		if (umdraw::UMCameraPtr camera = abc_scene_list_.at(0)->umdraw_camera(umstring()))
		{
			scene_->mutable_camera_list().at(0) = abc_scene_list_.at(0)->umdraw_camera(umstring());
		}
	}

}

/**
 * set a number of current frame
 */
void UMViewer::set_current_frame(int frame)
{
	current_frame_ = frame;
	if (current_frame_ < 0)
	{
		current_frame_ = 0;
	}
	
	unsigned long update_time = current_frame_ * (1.0 / 30.0) * 1000.0;
	drawer_->update();
#ifdef WITH_ALEMBIC
	if (abc_)
	{
		abc_->update(update_time);
	}
#endif // WITH_ALEMBIC
	if (is_gui_drawing_)
	{
		if (gui_) gui_->update();
	}
}

/**
 * mouse button up/down
 */
void UMViewer::on_mouse(GLFWwindow * window, int button, int action)
{
	if (gui_ && is_gui_drawing_)
	{
		if (gui_->on_mouse(button, action))
		{
			return;
		}
	}
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
 * scroll
 */
void UMViewer::on_scroll(GLFWwindow *, double xoffset, double yoffset)
{
	if (gui_ && is_gui_drawing_)
	{
		if (gui_->on_scroll(xoffset, yoffset))
		{
			return;
		}
	}

	UMScenePtr scene = drawer_->scene();
	if (UMCameraPtr camera = scene->camera())
	{
		if (is_shift_down_)
		{
			camera->pan(0, yoffset * 10);
		}
		else if (is_ctrl_button_down_)
		{
			camera->pan(yoffset * 10, 0);
		}
		else
		{
			camera->dolly(0, yoffset * 30);
		}
	}
}

/**
 * mouse move
 */
void UMViewer::on_mouse_move(GLFWwindow *, double x, double y)
{
	UMScenePtr scene = drawer_->scene();
	if (!scene) return;
	UMCameraPtr camera = scene->camera();
	if (!camera) return;
	current_x_ = x;
	current_y_ = y;
	
	if (gui_ && is_gui_drawing_)
	{
		if (gui_->on_mouse_move(x, y))
		{
			return;
		}
	}

	if (is_middle_button_down_)
	{
		if (is_shift_down_)
		{
			camera->pan(pre_x_ - x, pre_y_ - y);
			pre_x_ = x;
			pre_y_ = y;
		}
		else if (is_ctrl_button_down_)
		{
			camera->dolly(pre_x_ - x, pre_y_ - y);
			pre_x_ = x;
			pre_y_ = y;
		}
		else
		{
			camera->rotate(pre_x_ - x, pre_y_ - y);
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
	gui_ = UMGUIPtr();
#ifdef WITH_ALEMBIC
	abc_ = UMAbcPtr();
#endif // WITH_ALEMBIC
	drawer_ = UMDrawPtr();
}

/**
 * window close
 */
void UMViewer::on_close(GLFWwindow * window)
{
	if (gui_) { gui_->dispose(); }
	gui_ = UMGUIPtr();
#ifdef WITH_ALEMBIC
	if (abc_) { abc_->dispose(); }
	abc_ = UMAbcPtr();
#endif // WITH_ALEMBIC
	drawer_ = UMDrawPtr();
}


/**
 * render
 */
UMImagePtr UMViewer::render()
{
	std::cout << "update bvh ...\n" << std::endl;
	rays_->scene_access()->update_bvh();

	std::cout << "render start...\n" << std::endl;
	umimage::UMImagePtr image;
	{
		umbase::UMTime time("render time:");
		image = rays_->render();
	}
	if (image)
	{
		std::cout << "render end...\n" << std::endl;
		scene_->set_foreground_image(image);
	}
	else
	{
		std::cout << "render failed...\n" << std::endl;
	}
	return image;
}

} // burger
