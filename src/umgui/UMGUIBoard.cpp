/**
 * @file UMGUIBoard.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */

#include "UMGUIBoard.h"
#include "UMMesh.h"
#include "UMMaterial.h"
#include "UMPath.h"
#include "UMStringUtil.h"
#include "UMVector.h"
#include "UMResource.h"

#if !defined(WITH_EMSCRIPTEN)
	#include <tchar.h>
#endif

#include "UMFont.h"
#include "UMTextureAtlas.h"

namespace umgui
{

namespace
{
	//const int circle_devide = 32;
	
	///// (sin, cos) point on circle
	//std::vector<UMVec2d> circle_verts;
	//
	//void init_circle_verts()
	//{
	//	circle_verts.clear();
	//	for (int i = 0; i < circle_devide; ++i)
	//	{
	//		double p = i / static_cast<double>(circle_devide) * M_PI * 2;
	//		UMVec2d vert( sin(p), cos(p) );
	//		circle_verts.push_back(p);
	//	}
	//}

} // anonymouse namespace
	
/**
 * create root board
 */
UMGUIBoardPtr UMGUIBoard::create_root_board(
	int screen_width, 
	int screen_height)
{
	UMGUIBoardPtr board = std::make_shared<UMGUIBoard>();
	board->x_ = 0;
	board->y_ = 0;
	board->width_ = screen_width;
	board->height_ = screen_height;
	board->is_root_ = true;

	//if (circle_verts.empty())
	//{
	//	init_circle_verts();
	//}

	return board;
}

/**
 * create color circle board
 */
UMGUIBoardPtr UMGUIBoard::create_color_circle_board(
	int screen_width, 
	int screen_height,
	int x, 
	int y, 
	int width, 
	int height,
	int depth)
{
	UMGUIBoardPtr board = UMGUIBoard::create_root_board(screen_width, screen_height);
	board->is_root_ = false;
	board->depth_ = depth;
	board->add_uv_panel(screen_width, screen_height, x, y, width, height);

	if (!board->mesh_) 
	{
		return UMGUIBoardPtr();
	}
	umdraw::UMMeshPtr mesh = board->mesh_;
	
	// set color circle shader
	umresource::UMResource& resource = umresource::UMResource::instance();
	umdraw::UMShaderEntryPtr entry = std::make_shared<umdraw::UMShaderEntry>();
	entry->set_gl_vertex_shader(umresource::UMResource::find_resource_data(resource, "UMColorCircle.vs"));
	entry->set_gl_fragment_shader(umresource::UMResource::find_resource_data(resource, "UMColorCircle.fs"));
	mesh->set_shader_entry(entry);
	
	//if (circle_verts.empty())
	//{
	//	init_circle_verts();
	//}

	return board;
}

/**
 * add color panel
 */
int UMGUIBoard::add_color_panel(
	int screen_width, 
	int screen_height, 
	int x, 
	int y, 
	int width, 
	int height,
	const UMVec4d& color)
{
	umdraw::UMMaterialPtr material = umdraw::UMMaterial::default_material();
	if (!mesh_)
	{
		mesh_ = std::make_shared<umdraw::UMMesh>();
	}
	umdraw::UMMeshPtr mesh = mesh_;
	const double hw = screen_width / 2.0;
	const double hh = screen_height / 2.0;

	// front +z
	// v0--v1
	//  |   |
	// v2--v3
	UMVec3d point(x - hw, - y + hh - height, depth_);
	UMVec3d v0 = UMVec3d(     0, 0, 0.0) + point;
	UMVec3d v1 = UMVec3d( width, 0, 0.0) + point;
	UMVec3d v2 = UMVec3d(     0, height, 0.0) + point;
	UMVec3d v3 = UMVec3d( width, height, 0.0) + point;
	mesh->mutable_vertex_list().push_back(v0);
	mesh->mutable_vertex_list().push_back(v1);
	mesh->mutable_vertex_list().push_back(v2);
	mesh->mutable_vertex_list().push_back(v2);
	mesh->mutable_vertex_list().push_back(v1);
	mesh->mutable_vertex_list().push_back(v3);

	UMVec3d normal(0, 0, 1);
	mesh->mutable_normal_list().push_back(normal);
	mesh->mutable_normal_list().push_back(normal);
	mesh->mutable_normal_list().push_back(normal);
	mesh->mutable_normal_list().push_back(normal);
	mesh->mutable_normal_list().push_back(normal);
	mesh->mutable_normal_list().push_back(normal);

	material->set_polygon_count(2);
	material->set_diffuse(color);
	mesh->mutable_material_list().push_back(material);
	mesh->update_box();
	return 0;
}

///**
// * add check box panel
// */
//int UMGUIBoard::add_check_box(
//	int screen_width, 
//	int screen_height, 
//	int x, 
//	int y, 
//	int width, 
//	int height,
//	int radius,
//	const UMVec4d& color)
//{
//	umdraw::UMMaterialPtr material = umdraw::UMMaterial::default_material();
//	if (!mesh_)
//	{
//		mesh_ = std::make_shared<umdraw::UMMesh>();
//	}
//	umdraw::UMMeshPtr mesh = mesh_;
//	const double hw = screen_width / 2.0;
//	const double hh = screen_height / 2.0;
//
//	int n = circle_devide / 4;
//	UMVec3d point(x - hw, - y + hh - height, depth_);
//
//	// right-top corner 
//	//  vn     
//	//  |   v3
//	//  |  /   v2
//	//  | /      -
//	//  v0-------v1
//	//
//	for (int i = 0; i <= n; ++i)
//	{
//		UMVec3d v0 = UMVec3d(width - radius, radius, 0.0) + point;
//		UMVec3d v1 = UMVec3d(
//			radius * circle_verts[i][0],
//			radius * circle_verts[i][1], 
//			0.0) + v0;
//		UMVec3d v2 = UMVec3d(
//			radius * circle_verts[i+1][0],
//			radius * circle_verts[i+1][1],
//			0.0) + v0;
//		mesh->mutable_vertex_list().push_back(v0);
//		mesh->mutable_vertex_list().push_back(v1);
//		mesh->mutable_vertex_list().push_back(v2);
//	}
//	//     vn1 _vn
//	//       /   |
//	//   vn2_    |
//	//   /   - _ |
//	//  vn*2-----v0
//	//
//	for (int i = n; i <= n*2; ++i)
//	{
//		UMVec3d v0 = UMVec3d(radius, radius, 0.0) + point;
//		UMVec3d v1 = UMVec3d(
//			radius * circle_verts[i][0],
//			radius * circle_verts[i][1], 
//			0.0) + v0;
//		UMVec3d v2 = UMVec3d(
//			radius * circle_verts[i+1][0],
//			radius * circle_verts[i+1][1],
//			0.0) + v0;
//		mesh->mutable_vertex_list().push_back(v0);
//		mesh->mutable_vertex_list().push_back(v1);
//		mesh->mutable_vertex_list().push_back(v2);
//	}
//
//	//// front +z
//	//// 
//	////   v0--v1
//	////    |   |
//	////   v2--v3
//	//UMVec3d v0 = UMVec3d(radius, radius, 0.0) + point;
//	//UMVec3d v1 = UMVec3d(width - radius, 0, 0.0) + point;
//	//UMVec3d v2 = UMVec3d(     0, height - radius, 0.0) + point;
//	//UMVec3d v3 = UMVec3d(width - radius, height - radius, 0.0) + point;
//	//mesh->mutable_vertex_list().push_back(v0);
//	//mesh->mutable_vertex_list().push_back(v1);
//	//mesh->mutable_vertex_list().push_back(v2);
//	//mesh->mutable_vertex_list().push_back(v2);
//	//mesh->mutable_vertex_list().push_back(v1);
//	//mesh->mutable_vertex_list().push_back(v3);
//
//	UMVec3d normal(0, 0, 1);
//	const int vertex_count = static_cast<int>(mesh->mutable_vertex_list().size());
//	for (int i = 0; i < vertex_count; ++i)
//	{
//		mesh->mutable_normal_list().push_back(normal);
//	}
//
//	material->set_polygon_count(vertex_count / 3);
//	material->set_diffuse(color);
//	mesh->mutable_material_list().push_back(material);
//	mesh->update_box();
//	return 0;
//}

/**
 * add uv panel
 */
int UMGUIBoard::add_uv_panel(
	int screen_width, 
	int screen_height, 
	int x, 
	int y, 
	int width, 
	int height)
{
	add_color_panel(screen_width, screen_height, x, y, width, height, UMVec4d(1));

	if (!mesh_) 
	{
		return -1;
	}
	
	umdraw::UMMeshPtr mesh = mesh_;
	UMVec2d uv0 = UMVec2d(0.0, 0.0);
	UMVec2d uv1 = UMVec2d(1.0, 0.0);
	UMVec2d uv2 = UMVec2d(0.0, 1.0);
	UMVec2d uv3 = UMVec2d(1.0, 1.0);
	mesh->mutable_uv_list().push_back(uv0);
	mesh->mutable_uv_list().push_back(uv1);
	mesh->mutable_uv_list().push_back(uv2);
	mesh->mutable_uv_list().push_back(uv2);
	mesh->mutable_uv_list().push_back(uv1);
	mesh->mutable_uv_list().push_back(uv3);
	return 0;
}

/**
 * add texture panel
 */
int UMGUIBoard::add_texture_panel(
	int screen_width, 
	int screen_height, 
	int x, 
	int y, 
	int width, 
	int height,
	const umstring& path)
{
	add_uv_panel(screen_width, screen_height, x, y, width, height);

	if (!mesh_) 
	{
		return -1;
	}
	if (mesh_->mutable_material_list().empty())
	{
		return -1;
	}
	
	umdraw::UMMeshPtr mesh = mesh_;
	umdraw::UMMaterialPtr material = mesh->mutable_material_list().back();
	material->mutable_texture_path_list().push_back(path);

	return 0;
}

/**
 * add texture panel
 */
int UMGUIBoard::add_texture_panel(
	int screen_width, 
	int screen_height, 
	int x, 
	int y, 
	int width, 
	int height,
	UMImagePtr image)
{
	add_uv_panel(screen_width, screen_height, x, y, width, height);

	if (!mesh_) 
	{
		return -1;
	}
	if (mesh_->mutable_material_list().empty())
	{
		return -1;
	}
	
	umdraw::UMMeshPtr mesh = mesh_;
	umdraw::UMMaterialPtr material = mesh->mutable_material_list().back();
	material->mutable_texture_list().push_back(image);

	return 0;
}

/**
 * add text panel
 */
int UMGUIBoard::add_text_panel(
	int screen_width, 
	int screen_height,
	int x, 
	int y, 
	int font_size,
	const umtextstring& text)
{
#ifdef WITH_FREETYPE
	const umimage::UMFont* font = umimage::UMFont::instance();
	if (!font) return -1;

	x_ = x;
	y_ = y;

	// use application registered font
	umstring font_name;
	
	if (umimage::UMTextureAtlasPtr atlas = font->font_atlas(font_name, text, font_size))
	{
		umimage::UMImagePtr image = atlas->atlas_image();
		const double inv_image_width = 1.0 / image->width();
		const double inv_image_height = 1.0 / image->height();
		const double hw = screen_width / 2.0;
		const double hh = screen_height / 2.0;

		// create text mesh
		if (!mesh_)
		{
			mesh_ = std::make_shared<umdraw::UMMesh>();
		}
		umdraw::UMMeshPtr mesh = mesh_;

		// create text
		for (size_t i = 0, size = text.size(); i < size; ++i)
		{
			UMVec4ui rect = atlas->text_rect(text[i]);
			const double uv_left = rect.x * inv_image_width;
			const double uv_top = rect.w * inv_image_height;
			const double uv_right = rect.z * inv_image_width;
			const double uv_bottom = rect.y * inv_image_height;
			const int width = (rect.z - rect.x);
			const int height = font_size;
			//const int height = (rect.w - rect.y);
			
			// front +z
			// v0--v1
			//  |   |
			// v2--v3
			UMVec3d point(x - hw, - y + hh - height, depth_);
			UMVec3d v0 = UMVec3d(     0, 0, 0.0) + point;
			UMVec3d v1 = UMVec3d( width, 0, 0.0) + point;
			UMVec3d v2 = UMVec3d(     0, height, 0.0) + point;
			UMVec3d v3 = UMVec3d( width, height, 0.0) + point;
			mesh->mutable_vertex_list().push_back(v0);
			mesh->mutable_vertex_list().push_back(v1);
			mesh->mutable_vertex_list().push_back(v2);
			mesh->mutable_vertex_list().push_back(v2);
			mesh->mutable_vertex_list().push_back(v1);
			mesh->mutable_vertex_list().push_back(v3);

			UMVec3d normal(0, 0, 1);
			mesh->mutable_normal_list().push_back(normal);
			mesh->mutable_normal_list().push_back(normal);
			mesh->mutable_normal_list().push_back(normal);
			mesh->mutable_normal_list().push_back(normal);
			mesh->mutable_normal_list().push_back(normal);
			mesh->mutable_normal_list().push_back(normal);
			
			UMVec2d uv0 = UMVec2d(uv_left, uv_top);
			UMVec2d uv1 = UMVec2d(uv_right, uv_top);
			UMVec2d uv2 = UMVec2d(uv_left, uv_bottom);
			UMVec2d uv3 = UMVec2d(uv_right, uv_bottom);
			mesh->mutable_uv_list().push_back(uv0);
			mesh->mutable_uv_list().push_back(uv1);
			mesh->mutable_uv_list().push_back(uv2);
			mesh->mutable_uv_list().push_back(uv2);
			mesh->mutable_uv_list().push_back(uv1);
			mesh->mutable_uv_list().push_back(uv3);

			x += width;
		}

		umdraw::UMMaterialPtr material = umdraw::UMMaterial::default_material();

		material->set_polygon_count(2 * static_cast<int>(text.size()));
		material->mutable_texture_list().push_back(image);
		mesh->mutable_material_list().push_back(material);
		mesh->update_box();
	}

	return 0;
#else
	return 0;
#endif
}


} // umgui
