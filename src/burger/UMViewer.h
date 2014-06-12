/**
 * @file UMViewer.h
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include "UMMacro.h"
#include "UMDraw.h"
#include "UMAbc.h"
#include "UMGUI.h"
#include "UMScene.h"

struct GLFWwindow;

namespace umrt
{
	class UMRT;
	typedef std::shared_ptr<UMRT> UMRTPtr;
} // umrt

namespace umpaint
{
	class UMPaint;
	typedef std::shared_ptr<UMPaint> UMPaintPtr;
} // umpaint

/// test viewer(application)
namespace test_viewer
{

class UMViewer;
typedef std::shared_ptr<UMViewer> UMViewerPtr;

class UMViewer
{
	DISALLOW_COPY_AND_ASSIGN(UMViewer);
public:
	virtual ~UMViewer() {}
	
	/**
	 * initialize viewer with scene and draw type
	 */
	static bool init(
		GLFWwindow* window,
		GLFWwindow* sub_window, 
		umdraw::UMScenePtr scene, 
		umgui::UMGUIScenePtr gui_scene,
		//umabc::UMAbcSceneList& abc_scene_list, 
		umdraw::UMDraw::DrawType type, 
		int width, 
		int height);

	/**
	 * call a paint method of the current draw type 
	 */
	static void call_paint();

	/**
	 * key event callback
	 */
	static void key_callback(int key, int action);
	
	/**
	 * mouse event callback
	 */
	static void mouse_button_callback(int button, int action);
	
	/**
	 * mouse position event callback
	 */
	static void cursor_pos_callback(int x, int y);
	
	/**
	 * window size event callback
	 */
	static void window_size_callback(int width, int height);
	
	/**
	 * window close event callback
	 */
	static int window_close_callback();

	/**
	 * drop file callback
	 */
	static void drop_files_callback(GLFWwindow * window, int n, const char** files);

	/**
	 * get current draw type
	 */
	static umdraw::UMDraw::DrawType draw_type();
	
	/**
	 *
	 */
	void file_loaded_callback(GLFWwindow * window);

protected:
	
	static UMViewerPtr create(
		GLFWwindow* window,
		GLFWwindow* sub_window, 
		umdraw::UMScenePtr scene,
		umgui::UMGUIScenePtr gui_scene,
		//umabc::UMAbcSceneList& abc_scene_list, 
		umdraw::UMDraw::DrawType type);

	/**
	 * constructor
	 */
	explicit UMViewer(umdraw::UMDrawPtr drawer, umgui::UMGUIPtr gui);

	/**
	 * refresh frame
	 */
	bool on_paint();
	
	/**
	 * keyboard
	 */
	void on_keyboard(GLFWwindow * window,int key, int action);
	
	/**
	 * mouse button up/down
	 */
	void on_mouse(GLFWwindow * window, int button, int action);
	
	/**
	 * mouse move
	 */
	void on_mouse_move(GLFWwindow *, double x, double y);
	
	/**
	 * resize
	 */
	void on_resize(GLFWwindow *, int width, int height);
	
	/**
	 * window close
	 */
	void on_close(GLFWwindow *);
	
	/**
	 * view close
	 */
	void close_view();

private:
	static int width_;
	static int height_;
	static bool is_disable_update_;
	static bool is_gui_drawing_;
	static umdraw::UMScenePtr scene_;
	static umdraw::UMCameraPtr temporary_camera_;
	static umgui::UMGUIScenePtr gui_scene_;
	//static umabc::UMAbcSceneList abc_scene_list_;
	static UMViewerPtr viewer_;
	static GLFWwindow* sub_window_;

	double pre_x_;
	double pre_y_;
	double current_x_;
	double current_y_;
	double current_seconds_;
	unsigned long fps_base_time_;
	unsigned long motion_base_time_;
	unsigned long current_frames_;
	bool is_ctrl_button_down_;
	bool is_left_button_down_;
	bool is_right_button_down_;
	bool is_middle_button_down_;
	bool is_alt_down_;
	umdraw::UMDrawPtr drawer_;
	//umabc::UMAbcPtr abc_;
	umgui::UMGUIPtr gui_;
	umrt::UMRTPtr rays_;
	//umpaint::UMPaintPtr paint_;
};

} // test_viewer
