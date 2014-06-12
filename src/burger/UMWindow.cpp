/**
 * @file UMWindow.cpp
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
	#include <emscripten/emscripten.h>
#else
	#include <GL/glew.h>
#endif

#include "UMWindow.h"
#include "UMViewer.h"
#include "UMTime.h"
#include "UMScene.h"
#include "UMGUIScene.h"
#include "UMPath.h"
#include "UMStringUtil.h"
#include "UMResource.h"
#include "UMShaderEntry.h"
#include "UMAbcIO.h"
#include "UMAreaLight.h"
#include "UMLine.h"
#include "UMFont.h"
#include "UMStringUtil.h"
#include "UMTga.h"
#include "UMSvg.h"
#include <GL/glfw.h>

namespace test_viewer
{
	using namespace umresource;

/**
 * create initial scene
 */
static umdraw::UMScenePtr create_initial_scene(int width, int height, const umstring& initial_file)
{
	// create scene
	umdraw::UMScenePtr scene = (std::make_shared<umdraw::UMScene>());
	scene->init(width, height);
	
	// set light
	umdraw::UMLightPtr umlight = std::make_shared<umdraw::UMLight>(UMVec3d(200, 200, 500));
	
	//UMVec3d pos(-100, 400, -100);
	//UMVec3d edge1(200, 0, 0);
	//UMVec3d edge2(0, 0, 200);
	//UMVec3d normal = -UMVec3d(-10, 10,10).normalized();

	//umdraw::UMLightPtr umlight = umdraw::UMLightPtr(
	//	umrt::UMAreaLightPtr(new umrt::UMAreaLight(pos, edge1, edge2, normal, 0.0, 0.0, 1.0)));
	scene->mutable_light_list().push_back(umlight);

	umdraw::UMLinePtr line(std::make_shared<umdraw::UMLine>());
	
	const int quarter = 5;
	const double line_size = 25.0;
	const double delta = line_size / static_cast<double>(quarter);
	for (int k = 0; k < 4; ++k)
	{
		UMVec3d offset(0);
		if (k == 1)
		{
			offset = UMVec3d(-line_size, 0, 0);
		}
		else if (k == 2)
		{
			offset = UMVec3d(-line_size, 0, -line_size);
		}
		else if (k == 3)
		{
			offset = UMVec3d(0, 0, -line_size);
		}
		for (int i = 0; i <= quarter; i++) {
			umdraw::UMLine::Line line1(UMVec3d(i*delta,0,0), UMVec3d(i*delta,0,line_size));
			umdraw::UMLine::Line line2(UMVec3d(0,0,i*delta), UMVec3d(line_size,0,i*delta));
			line1.p0 += offset;
			line1.p1 += offset;
			line2.p0 += offset;
			line2.p1 += offset;
			line->mutable_line_list().push_back(line1);
			line->mutable_line_list().push_back(line2);
		};
	}
	{
		umdraw::UMMaterialPtr axis_mat0 = umdraw::UMMaterial::default_material();
		axis_mat0->set_diffuse(UMVec4d(0.3, 0.3, 0.6, 1.0));
		axis_mat0->set_polygon_count(1);
		line->mutable_material_list().push_back(axis_mat0);
	}
	{
		umdraw::UMMaterialPtr axis_mat1 = umdraw::UMMaterial::default_material();
		axis_mat1->set_diffuse(UMVec4d(0.6, 0.3, 0.3, 1.0));
		axis_mat1->set_polygon_count(1);
		line->mutable_material_list().push_back(axis_mat1);
	}
	{
		umdraw::UMMaterialPtr line_mat = umdraw::UMMaterial::default_material();
		line_mat->set_diffuse(UMVec4d(0.25, 0.25, 0.25, 1.0));
		line_mat->set_polygon_count(static_cast<int>(line->line_list().size()) - 2);
		line->mutable_material_list().push_back(line_mat);
	}

	scene->mutable_line_list().push_back(line);
	
	UMResource& resource = UMResource::instance();
	const umstring& resource_path = UMResource::default_resource_path();
	printf("resource path : %s\n", resource_path.c_str());
	if (resource.unpack_to_memory(resource_path))
	{
		printf("resource load sucess \n");
		UMResource::UnpackedNameList& name_list = resource.unpacked_name_list();
		//printf("name list success \n");
		UMResource::UnpackedDataList& data_list = resource.unpacked_data_list();
		//printf("data list success \n");
		
		//printf("hoge1");
		//printf("hoge2");
		// load shader
		{
			umdraw::UMShaderEntry& entry = umdraw::UMShaderEntry::instance();
			entry.set_gl_vertex_shader(UMResource::find_resource_data(resource, ("UMModelShader.vs")));
			entry.set_gl_fragment_shader(UMResource::find_resource_data(resource, ("UMModelShader.fs")));
			entry.set_gl_point_vertex_shader(UMResource::find_resource_data(resource, ("UMPointShader.vs")));
			entry.set_gl_point_fragment_shader(UMResource::find_resource_data(resource, ("UMPointShader.fs")));
			entry.set_gl_board_vertex_shader(UMResource::find_resource_data(resource, ("UMBoardShader.vs")));
			entry.set_gl_board_fragment_shader(UMResource::find_resource_data(resource, ("UMBoardShader.fs")));

			//entry.set_gl_board_light_pass_vertex_shader(UMResource::find_resource_data(resource, ("UMBoardLightPass.vs")));
			//entry.set_gl_board_light_pass_fragment_shader(UMResource::find_resource_data(resource, ("UMBoardLightPass.fs")));
			//entry.set_gl_vertex_geo_shader(UMResource::find_resource_data(resource, ("UMModelGeometryPass.vs")));
			//entry.set_gl_fragment_geo_shader(UMResource::find_resource_data(resource, ("UMModelGeometryPass.fs")));

			//entry.set_dx_include_shader(UMResource::find_resource_data(resource, ("common.hlsl")));
			//entry.set_dx_vertex_shader(UMResource::find_resource_data(resource, ("model_vs.hlsl")));
			//entry.set_dx_pixel_shader(UMResource::find_resource_data(resource, ("model_ps.hlsl")));

			std::cout << "This program used external libraries below." << std::endl;
			std::cout << "---\"GLFW\" LICENSE-----------------------------------------------" << std::endl;
			std::cout << UMResource::find_resource_data(resource, ("GLFW-LICENSE.txt")).c_str() << std::endl;
			std::cout << "---\"tinyobjloader\" LICENSE--------------------------------------" << std::endl;
			std::cout << UMResource::find_resource_data(resource, ("tinyobjloader-LICENSE.txt")).c_str() << std::endl;
			std::cout << "---\"snappy\" LICENSE---------------------------------------------" << std::endl;
			std::cout << UMResource::find_resource_data(resource, ("snappy-LICENSE.txt")).c_str() << std::endl;
		}
		
		if (const umimage::UMFont* font = umimage::UMFont::instance())
		{
			if (font->load_font_from_memory(
				umbase::UMStringUtil::utf8_to_utf16("KodomoRounded"), 
				UMResource::find_resource_data(resource, ("mplus-1c-medium-sub.ttf"))))
			{
				std::cout << "font load success!" << std::endl;
			}
		}
		//umstring kimura = umbase::UMPath::resource_absolute_path(umbase::UMStringUtil::utf8_to_utf16("kimura.obj"));
		//scene->load(kimura);
		//umstring kimura2 = umbase::UMPath::resource_absolute_path(umbase::UMStringUtil::utf8_to_utf16("kimura2.obj"));
		//scene->load(kimura2);
		//umstring kimura4 = umbase::UMPath::resource_absolute_path(umbase::UMStringUtil::utf8_to_utf16("kimura4.obj"));
		//scene->load(kimura4);
		umstring box = umbase::UMPath::resource_absolute_path(umbase::UMStringUtil::utf8_to_utf16("box.obj"));
		scene->load(box);
		umstring board = umbase::UMPath::resource_absolute_path(umbase::UMStringUtil::utf8_to_utf16("board.obj"));
		scene->load(board);
		//if (!initial_file.empty())
		//{
		//	scene->load(initial_file);
		//}
	}
	return scene;
}

//static bool create_initial_abc_scene_list(
//	umabc::UMAbcSceneList& scene_list, 
//	umdraw::UMScenePtr draw_scene)
//{
//	umabc::UMAbcIO abcio;
//	umstring filename(umbase::UMStringUtil::wstring_to_utf16(_T("out2\\alembic_file.abc")));
//	//umstring filename(umbase::UMStringUtil::wstring_to_utf16(_T("particle1.abc")));
//	umstring absolute_path = umbase::UMPath::resource_absolute_path(filename);
//	umabc::UMAbcSetting setting;
//	setting.set_reference_scene(draw_scene);
//	if (umabc::UMAbcScenePtr abc_scene = abcio.load(
//			absolute_path,
//			setting))
//	{
//		scene_list.push_back(abc_scene);
//		return true;
//	}
//	return false;
//}

static umgui::UMGUIScenePtr create_initial_gui_scene(
	umdraw::UMScenePtr draw_scene,
	int width, 
	int height)
{
	umgui::UMGUIScenePtr gui_scene = std::make_shared<umgui::UMGUIScene>();
	if (gui_scene->init(width, height))
	{
		gui_scene->set_umdraw_scene(draw_scene);
		return gui_scene;
	}
	return umgui::UMGUIScenePtr();
}

void main_loop()
{
	// draw
	UMViewer::call_paint();

	if (UMViewer::draw_type() == umdraw::UMDraw::eOpenGL)
	{
		glfwSwapBuffers();
	}
	glfwPollEvents();
}

/**
 * main
 */
int UMWindow::main(int argc, char** argv)
{
	//FreeConsole();
	if (!glfwInit()) {
		exit( EXIT_FAILURE );
	}

	////glfwDefaultWindowHints();
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	////glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

	// create main window
	if (glfwOpenWindow(width_, height_, 8, 8, 8, 8, 24, 8, GLFW_WINDOW) != GL_TRUE)
	{
		glfwTerminate();
		exit( EXIT_FAILURE );
	}
	// create hide window
	//GLFWwindow* sub_window = glfwCreateWindow(1, 1, "Resource Loader", NULL, window);
	//if (!sub_window) {
	//	glfwTerminate();
	//	exit( EXIT_FAILURE );
	//}
	//glfwHideWindow(sub_window);

	//glfwMakeContextCurrent(window);
	// print environment
	printf("context renderer string: \"%s\"\n", glGetString(GL_RENDERER));
	printf("context vendor string: \"%s\"\n", glGetString(GL_VENDOR));
	printf("version string: \"%s\"\n", glGetString(GL_VERSION));
	
#ifndef WITH_EMSCRIPTEN
	glewExperimental = GL_TRUE;
	glewInit();
#endif
	
	//// init font
	//HWND hwnd = glfwGetWin32Window(window);
	//	umstring font_name = umbase::UMStringUtil::wstring_to_utf16(_T("メイリオ"));
	//	font->load_font(hwnd, font_name);
	//}

	// create umio scene
	umstring initial_file;
	if (argc > 1) {
		initial_file = umbase::UMStringUtil::utf8_to_utf16(argv[1]);
		if (!umbase::UMPath::exists(initial_file))
		{
			initial_file.clear();
		}
	}
	if (umdraw::UMScenePtr scene = create_initial_scene(width_, height_, initial_file))
	{
		if (umgui::UMGUIScenePtr gui_scene = create_initial_gui_scene(scene, width_, height_))
		{
			// create alembic scene
			//umabc::UMAbcSceneList abc_scene_list;
			//create_initial_abc_scene_list(abc_scene_list, scene);
			{
				// init viewer
				if (UMViewer::init(
					NULL,
					NULL,
					scene, 
					gui_scene, 
					//abc_scene_list, 
					umdraw::UMDraw::eOpenGL, 
					width_, 
					height_))
				{
					// set callback to viewer
					glfwSetKeyCallback(UMViewer::key_callback);
					glfwSetMouseButtonCallback(UMViewer::mouse_button_callback);
					glfwSetMousePosCallback(UMViewer::cursor_pos_callback);
					glfwSetWindowSizeCallback(UMViewer::window_size_callback );
					glfwSetWindowCloseCallback(UMViewer::window_close_callback );
					//glfwSetDropfilesCallback(UMViewer::drop_files_callback);
				}
			}
		}
	}
	glfwSwapInterval(1);

#ifdef WITH_EMSCRIPTEN
	emscripten_set_main_loop (main_loop, 0, 1);
#else
	// main loop
	for (;;) 
	{
		// draw
		UMViewer::call_paint();

		if (UMViewer::draw_type() == umdraw::UMDraw::eOpenGL)
		{
			glfwSwapBuffers();
		}
		glfwPollEvents();
		if (glfwGetKey(GLFW_KEY_ESC))
			break;
		if (!glfwGetWindowParam(GLFW_OPENED))
			break;
	}
#endif
	// call window close callback to delete resources.
	UMViewer::window_close_callback();
	//glfwDestroyWindow(sub_window);
	//glfwDestroyWindow(window);
	glfwTerminate();
	exit( EXIT_SUCCESS );
	return true;
}


} // test_viewer
