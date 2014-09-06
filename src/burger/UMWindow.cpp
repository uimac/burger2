/**
 * @file UMWindow.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#include <GL/glew.h>

#include "UMWindow.h"
#include "UMViewer.h"
#include "UMTime.h"
#include "UMScene.h"
#include "UMGUIScene.h"
#include "UMBurgerGUI.h"
#include "UMPath.h"
#include "UMStringUtil.h"
#include "UMResource.h"
#include "UMShaderEntry.h"
#include "UMLine.h"
#include "UMFont.h"
#include "UMStringUtil.h"
#include "UMTga.h"
#include "UMAreaLight.h"
#include "UMAbcIO.h"
#include <GL/glfw3.h>
#include <GL/glfw3native.h>

namespace burger
{
	using namespace umresource;

bool is_raytracing_camp = true;

/**
 * create initial scene
 */
static umdraw::UMScenePtr create_initial_scene(int width, int height)
{
	// create scene
	umdraw::UMScenePtr scene = (std::make_shared<umdraw::UMScene>());
	scene->init(width, height);
	
	// set light
	umdraw::UMLightPtr umlight = std::make_shared<umdraw::UMLight>(UMVec3d(200, 50, 500));
	
	UMVec3d pos(-50, 200, -50);
	UMVec3d edge1(200, 0, 0);
	UMVec3d edge2(0, 0, 200);
	UMVec3d normal = -UMVec3d(-10, 10,10).normalized();

	umdraw::UMLightPtr umarealight = umdraw::UMLightPtr(
		umrt::UMAreaLightPtr(new umrt::UMAreaLight(pos, edge1, edge2, normal, 0.0, 0.0, 1.0)));
	scene->mutable_light_list().push_back(umlight);
	scene->mutable_light_list().push_back(umarealight);

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
			std::cout << "---\"Alembic\" LICENSE--------------------------------------" << std::endl;
			std::cout << UMResource::find_resource_data(resource, ("Alembic-LICENSE.txt")).c_str() << std::endl;
			std::cout << "---\"snappy\" LICENSE---------------------------------------------" << std::endl;
			std::cout << UMResource::find_resource_data(resource, ("snappy-LICENSE.txt")).c_str() << std::endl;
		}

		if (const umimage::UMFont* font = umimage::UMFont::instance())
		{
			if (font->load_font_from_memory(
				umbase::UMStringUtil::utf8_to_utf16("KodomoRounded"), 
				UMResource::find_resource_data(resource, ("KodomoRounded.ttf"))))
			{
				std::cout << "font load success!" << std::endl;
			}
		}
	}

	{
		//resource.unpack();
		//scene->load_from_memory(UMResource::find_resource_data(resource, ("monkey2.bos")));
		//UMImagePtr image = svg->create_image(50, 50);
		//umimage::UMTga tga;
		//tga.save("hoge.tga", *image);
		
		//umstring box = umbase::UMPath::resource_absolute_path(umbase::UMStringUtil::utf8_to_utf16("box.obj"));
		//scene->load(box);
		//umstring board = umbase::UMPath::resource_absolute_path(umbase::UMStringUtil::utf8_to_utf16("board.obj"));
		//scene->load(board);
		//// load model
		//if (data_list.size() > 10)
		//{
		//	for (size_t i = 10, size = data_list.size(); i < size; ++i)
		//	{
		//		std::string& data = data_list.at(i);
		//		scene->load_from_memory(data);
		//	}
		//}
	}
	return scene;
}

static bool create_initial_abc_scene_list(
	umabc::UMAbcSceneList& scene_list, 
	umdraw::UMScenePtr draw_scene)
{
	if (!is_raytracing_camp)
	{
		return true;
	}
	umabc::UMAbcIO abcio;
	//umstring filename(umbase::UMStringUtil::wstring_to_utf16(_T("out2\\alembic_file.abc")));
	//umstring absolute_path = umbase::UMPath::resource_absolute_path(filename);

	//umstring filename(umbase::UMStringUtil::wstring_to_utf16(_T("C:\\tmp\\alembic_export.abc")));
	//umstring absolute_path = filename;
	
	std::string filename = "\\model\\alembic_file.abc";
	umstring utf16name = umbase::UMStringUtil::utf8_to_utf16(filename);
	umstring absolute_path = umbase::UMPath::get_base_folder(umbase::UMPath::module_absolute_path());
	absolute_path = absolute_path.append(utf16name);

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

static UMBurgerGUIPtr create_initial_gui_scene(
	umdraw::UMScenePtr scene,
	int width, 
	int height)
{
	UMBurgerGUIPtr gui_scene = std::make_shared<UMBurgerGUI>();
	gui_scene->set_umdraw_scene(scene);
	if (gui_scene->init(width, height))
	{
		return gui_scene;
	}
	return UMBurgerGUIPtr();
}

static void glewInfo()
{
	int err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "Error [main]: glewInit failed: %s\n", glewGetErrorString(err));
	}

	if (GLEW_VERSION_1_1){
		printf("We have OpenGL 1.1\n");
	}

	if (GLEW_VERSION_1_2){
		printf("We have OpenGL 1.2\n");
	}

	if (GLEW_VERSION_1_3){
		printf("We have OpenGL 1.3\n");
	}

	if (GLEW_VERSION_1_4){
		printf("We have OpenGL 1.4\n");
	}

	if (GLEW_VERSION_1_5){
		printf("We have OpenGL 1.5\n");
	}

	if (GLEW_VERSION_2_0){
		printf("We have OpenGL 2.0\n");
	}

	if (GLEW_VERSION_2_1){
		printf("We have OpenGL 2.1\n");
	}

	if (GLEW_VERSION_3_0){
		printf("We have OpenGL 3.0\n");
	} else {
		printf("No OpenGL 3.0\n");
		if (GLEW_ARB_texture_rg){
			printf("  OpenGL 3.0 GL_ARB_texture_rg extension present\n");
		}
		if (GLEW_ARB_texture_compression_rgtc){
			printf("  OpenGL 3.0 GL_ARB_texture_compression_rgtc extension present\n");
		}
		if (GLEW_ARB_color_buffer_float){
			printf("  OpenGL 3.0 GL_ARB_color_buffer_float extension present\n");
		}
		if (GLEW_ARB_half_float_pixel){
			printf("  OpenGL 3.0 GL_ARB_half_float_pixel extension present\n");
		}
		if (GLEW_ARB_texture_float){
			printf("  OpenGL 3.0 GL_ARB_texture_float extension present\n");
		}
		if (GLEW_ARB_depth_buffer_float){
			printf("  OpenGL 3.0 GL_ARB_depth_buffer_float extension present\n");
		}
		if (GLEW_ARB_framebuffer_object){
			printf("  OpenGL 3.0 GL_ARB_framebuffer_object extension present\n");
		}
		if (GLEW_ARB_map_buffer_range){
			printf("  OpenGL 3.0 GL_ARB_map_buffer_range extension present\n");
		}
		if (GLEW_ARB_vertex_array_object){
			printf("  OpenGL 3.0 GL_ARB_vertex_array_object extension present\n");
		}
		if (GLEW_EXT_bindable_uniform){
			printf("  OpenGL 3.0 GL_EXT_bindable_uniform extension present\n");
		}
		if (GLEW_EXT_draw_buffers2){
			printf("  OpenGL 3.0 GL_EXT_draw_buffers2 extension present\n");
		}
		if (GLEW_EXT_geometry_shader4){
			printf("  OpenGL 3.0 GL_EXT_geometry_shader4 extension present\n");
		}
		if (GLEW_EXT_gpu_shader4){
			printf("  OpenGL 3.0 GL_EXT_gpu_shader4 extension present\n");
		}
		if (GLEW_EXT_framebuffer_sRGB){
			printf("  OpenGL 3.0 GL_EXT_framebuffer_sRGB extension present\n");
		}
		if (GLEW_EXT_texture_compression_rgtc){
			printf("  OpenGL 3.0 GL_EXT_texture_compression_rgtc extension present\n");
		}


	}

	if (GLEW_VERSION_3_1){
		printf("We have OpenGL 3.1\n");
	} else {
		printf("No OpenGL 3.1\n");
		if (GLEW_ARB_texture_rectangle){
			printf("  OpenGL 3.1 GL_ARB_texture_rectangle extension present\n");
		}
		if (GLEW_ARB_draw_instanced){
			printf("  OpenGL 3.1 GL_ARB_draw_instanced extension present\n");
		}
		if (GLEW_ARB_texture_buffer_object){
			printf("  OpenGL 3.1 GL_ARB_texture_buffer_object extension present\n");
		}
		if (GLEW_ARB_uniform_buffer_object){
			printf("  OpenGL 3.1 GL_ARB_uniform_buffer_object extension present\n");
		}
		if (GLEW_ARB_compatibility){
			printf("  OpenGL 3.1 GL_ARB_compatibility extension present\n");
		}
		if (GLEW_ARB_copy_buffer){
			printf("  OpenGL 3.1 GL_ARB_copy_buffer extension present\n");
		}
		if (GLEW_ARB_shader_texture_lod){
			printf("  OpenGL 3.1 GL_ARB_shader_texture_lod extension present\n");
		}
	}

	if (GLEW_VERSION_3_2){
		printf("We have OpenGL 3.2\n");
	} else {
		printf("No OpenGL 3.2\n");
		if (GLEW_ARB_depth_clamp){
			printf("  OpenGL 3.2 GL_ARB_depth_clamp extension present\n");
		}
		if (GLEW_ARB_draw_elements_base_vertex){
			printf("  OpenGL 3.2 GL_ARB_draw_elements_base_vertex extension present\n");
		}
		if (GLEW_ARB_fragment_coord_conventions){
			printf("  OpenGL 3.2 GL_ARB_fragment_coord_conventions extension present\n");
		}
		if (GLEW_ARB_provoking_vertex){
			printf("  OpenGL 3.2 GL_ARB_provoking_vertex extension present\n");
		}
		if (GLEW_ARB_seamless_cube_map){
			printf("  OpenGL 3.2 GL_ARB_seamless_cube_map extension present\n");
		}
		if (GLEW_ARB_sync){
			printf("  OpenGL 3.2 GL_ARB_sync extension present\n");
		}
		if (GLEW_ARB_texture_multisample){
			printf("  OpenGL 3.2 GL_ARB_texture_multisample extension present\n");
		}
		if (GLEW_ARB_vertex_array_bgra){
			printf("  OpenGL 3.2 GL_ARB_vertex_array_bgra extension present\n");
		}
	}

	if (GLEW_VERSION_3_3){
		printf("We have OpenGL 3.3\n");
	} else {
		printf("No OpenGL 3.3\n");
		if (GLEW_ARB_blend_func_extended){
			printf("  OpenGL 3.3 GL_ARB_blend_func_extended extension present\n");
		}
		if (GLEW_ARB_explicit_attrib_location){
			printf("  OpenGL 3.3 GL_ARB_explicit_attrib_location extension present\n");
		}
		if (GLEW_ARB_occlusion_query2){
			printf("  OpenGL 3.3 GL_ARB_occlusion_query2 extension present\n");
		}
		if (GLEW_ARB_sampler_objects){
			printf("  OpenGL 3.3 GL_ARB_sampler_objects extension present\n");
		}
		if (GLEW_ARB_shader_bit_encoding){
			printf("  OpenGL 3.3 GL_ARB_shader_bit_encoding extension present\n");
		}
		if (GLEW_ARB_texture_rgb10_a2ui){
			printf("  OpenGL 3.3 GL_ARB_texture_rgb10_a2ui extension present\n");
		}
		if (GLEW_ARB_texture_swizzle){
			printf("  OpenGL 3.3 GL_ARB_texture_swizzle extension present\n");
		}
		if (GLEW_ARB_timer_query){
			printf("  OpenGL 3.3 GL_ARB_timer_query extension present\n");
		}
		if (GLEW_ARB_vertex_type_2_10_10_10_rev){
			printf("  OpenGL 3.3 GL_ARB_vertex_type_2_10_10_10_rev extension present\n");
		}
	}

	if (GLEW_VERSION_4_0){
		printf("We have OpenGL 4.0\n");
	} else {
		printf("No OpenGL 4.0\n");
	}

	if (GLEW_VERSION_4_1){
		printf("We have OpenGL 4.1\n");
	} else {
		printf("No OpenGL 4.1\n");
	}
}

/**
 * just a wrapper of main
 */
int UMWindow::main(int argc, char** argv)
{
	//FreeConsole();
	if (!glfwInit()) {
		exit( EXIT_FAILURE );
	}

	//glfwDefaultWindowHints();
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	//glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);


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
	glewInfo();
#endif

	// create umio scene
	if (umdraw::UMScenePtr scene = create_initial_scene(width_, height_))
	{
		if (UMBurgerGUIPtr gui_scene = create_initial_gui_scene(scene, width_, height_))
		{
			// create alembic scene
			umabc::UMAbcSceneList abc_scene_list;
			if (create_initial_abc_scene_list(abc_scene_list, scene))
			{
				// init viewer
				if (UMViewer::init(
					window,
					sub_window,
					scene, 
					gui_scene, 
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
					glfwSetScrollCallback(window, UMViewer::scroll_callback);
				}
			}
		}
	}

	glfwSwapInterval(1);

	// main loop
	for (;;) 
	{
		// draw
		if (!UMViewer::call_paint())
		{
			break;
		}

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

} // burger
