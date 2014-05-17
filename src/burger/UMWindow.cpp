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
	#include <GLES2/gl2.h>
	#include <EGL/egl.h>
#else
	#include <GL/glew.h>
#endif

#include "UMWindow.h"
#include "UMViewer.h"
#include "UMTime.h"
#include "UMScene.h"
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
#include <GL/glfw3.h>
#include <GL/glfw3native.h>


namespace test_viewer
{
	using namespace umresource;

/**
 * create initial scene
 */
static umdraw::UMScenePtr create_initial_scene(int width, int height)
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
	if (resource.unpack_to_memory(UMResource::default_resource_path()))
	{
		UMResource::UnpackedNameList& name_list = resource.unpacked_name_list();
		UMResource::UnpackedDataList& data_list = resource.unpacked_data_list();

		// load shader
		{
			umdraw::UMShaderEntry& entry = umdraw::UMShaderEntry::instance();
			entry.set_gl_vertex_shader(UMResource::find_resource_data(resource, ("UMModelShader.vs")));
			entry.set_gl_fragment_shader(UMResource::find_resource_data(resource, ("UMModelShader.fs")));
			entry.set_gl_point_vertex_shader(UMResource::find_resource_data(resource, ("UMPointShader.vs")));
			entry.set_gl_point_fragment_shader(UMResource::find_resource_data(resource, ("UMPointShader.fs")));
			entry.set_gl_board_vertex_shader(UMResource::find_resource_data(resource, ("UMBoardShader.vs")));
			entry.set_gl_board_fragment_shader(UMResource::find_resource_data(resource, ("UMBoardShader.fs")));

			entry.set_gl_board_light_pass_vertex_shader(UMResource::find_resource_data(resource, ("UMBoardLightPass.vs")));
			entry.set_gl_board_light_pass_fragment_shader(UMResource::find_resource_data(resource, ("UMBoardLightPass.fs")));
			entry.set_gl_vertex_geo_shader(UMResource::find_resource_data(resource, ("UMModelGeometryPass.vs")));
			entry.set_gl_fragment_geo_shader(UMResource::find_resource_data(resource, ("UMModelGeometryPass.fs")));

			entry.set_dx_include_shader(UMResource::find_resource_data(resource, ("common.hlsl")));
			entry.set_dx_vertex_shader(UMResource::find_resource_data(resource, ("model_vs.hlsl")));
			entry.set_dx_pixel_shader(UMResource::find_resource_data(resource, ("model_ps.hlsl")));
		}
	}
	return scene;
}

static bool create_initial_abc_scene_list(
	umabc::UMAbcSceneList& scene_list, 
	umdraw::UMScenePtr draw_scene)
{
	umabc::UMAbcIO abcio;
	std::u16string filename(umbase::UMStringUtil::wstring_to_utf16(_T("out2\\alembic_file.abc")));
	//std::u16string filename(umbase::UMStringUtil::wstring_to_utf16(_T("particle1.abc")));
	std::u16string absolute_path = umbase::UMPath::resource_absolute_path(filename);
	umabc::UMAbcSetting setting;
	setting.set_reference_scene(draw_scene);
	if (umabc::UMAbcScenePtr abc_scene = abcio.load(
			absolute_path,
			setting))
	{
		scene_list.push_back(abc_scene);
		return true;
	}
	return false;
}

/**
 * main
 */
int UMWindow::main()
{
	//FreeConsole();
	if (!glfwInit()) {
		exit( EXIT_FAILURE );
	}

	//glfwDefaultWindowHints();
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);


	// create main window
	GLFWwindow* window = glfwCreateWindow(width_, height_, "burger", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit( EXIT_FAILURE );
	}
	// create hide window
	GLFWwindow* sub_window = glfwCreateWindow(1, 1, "Resource Loader", NULL, window);
	if (!sub_window) {
		glfwTerminate();
		exit( EXIT_FAILURE );
	}
	glfwHideWindow(sub_window);

	glfwMakeContextCurrent(window);
	// print environment
	printf("context renderer string: \"%s\"\n", glGetString(GL_RENDERER));
	printf("context vendor string: \"%s\"\n", glGetString(GL_VENDOR));
	printf("version string: \"%s\"\n", glGetString(GL_VERSION));
	
#ifndef WITH_EMSCRIPTEN
	glewExperimental = GL_TRUE;
	glewInit();
#endif
	
	// init font
	HWND hwnd = glfwGetWin32Window(window);
	if (const umimage::UMFont* font = umimage::UMFont::instance())
	{
		std::u16string font_name = umbase::UMStringUtil::wstring_to_utf16(_T("メイリオ"));
		font->load_font(hwnd, font_name);
	}

	// create umio scene
	if (umdraw::UMScenePtr scene = create_initial_scene(width_, height_))
	{
		{
			// create alembic scene
			umabc::UMAbcSceneList abc_scene_list;
			//create_initial_abc_scene_list(abc_scene_list, scene);
			{
				// init viewer
				if (UMViewer::init(
					window,
					sub_window,
					scene, 
					abc_scene_list, 
					umdraw::UMDraw::eOpenGL, 
					width_, 
					height_))
				{
					// set callback to viewer
					glfwSetKeyCallback(window, UMViewer::key_callback);
					glfwSetMouseButtonCallback(window, UMViewer::mouse_button_callback);
					glfwSetCursorPosCallback(window, UMViewer::cursor_pos_callback);
					glfwSetWindowSizeCallback(window, UMViewer::window_size_callback );
					glfwSetWindowCloseCallback(window, UMViewer::window_close_callback );
					glfwSetDropfilesCallback(window, UMViewer::drop_files_callback);
				}
			}
		}
	}
	glfwSwapInterval(1);

	// main loop
	for (;;) 
	{
		// draw
		UMViewer::call_paint();

		if (UMViewer::draw_type() == umdraw::UMDraw::eOpenGL)
		{
			glfwSwapBuffers(window);
		}
		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_ESCAPE))
			break;
		if (glfwWindowShouldClose(window))
			break;
	}
	// call window close callback to delete resources.
	UMViewer::window_close_callback(window);
	glfwDestroyWindow(sub_window);
	glfwDestroyWindow(window);
	glfwTerminate();
	exit( EXIT_SUCCESS );
	return true;
}

} // test_viewer
