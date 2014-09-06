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

	
UMGUIBoard::UMGUIBoard()
	: x_(0)
	, y_(0)
	, width_(0)
	, height_(0)
	, depth_(0) 
	, is_left_dragging_(false)
{}

UMGUIBoard::UMGUIBoard(int depth)
	: x_(0)
	, y_(0)
	, width_(0)
	, height_(0)
	, depth_(depth) 
	, is_left_dragging_(false)
{}

/**
 * create empty board
 */
UMGUIBoardPtr UMGUIBoard::create_board(int depth)
{
	UMGUIBoardPtr board(new UMGUIBoard(depth));
	board->self_ = board;
	return board;
}

/**
 * create root board
 */
UMGUIBoardPtr UMGUIBoard::create_root_board(
	int screen_width, 
	int screen_height)
{
	UMGUIBoardPtr board(new UMGUIBoard());
	board->x_ = 0;
	board->y_ = 0;
	board->width_ = screen_width;
	board->height_ = screen_height;
	board->is_root_ = true;
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

	material->set_polygon_count(material->polygon_count() + 2);
	material->set_diffuse(color);
	mesh->mutable_material_list().push_back(material);
	mesh->update_box();
	
	box_.extend(
		UMBox(
			mesh->box().minimum() + UMVec3d(hw, 2 * y - hh + height, 0),
			mesh->box().maximum() + UMVec3d(hw, 2 * y - hh + height, 0)
		));
	initial_box_ = box_;
	return 0;
}

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
#if defined(WITH_FREETYPE) || defined(WITH_STBTRUETYPE)
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
		
		const int height = font_size;

		// create text
		for (size_t i = 0, size = text.size(); i < size; ++i)
		{
			UMVec4ui rect = atlas->text_rect(text[i]);
			const double uv_left = rect.x * inv_image_width;
			const double uv_top = rect.w * inv_image_height;
			const double uv_right = rect.z * inv_image_width;
			const double uv_bottom = rect.y * inv_image_height;
			const int width = (rect.z - rect.x);
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

		material->set_polygon_count(material->polygon_count() + 2 * static_cast<int>(text.size()));
		material->mutable_texture_list().push_back(image);
		mesh->mutable_material_list().push_back(material);
		mesh->update_box();
		box_.extend(
			UMBox(
				mesh->box().minimum() + UMVec3d(hw, 2 * y - hh + height, 0),
				mesh->box().maximum() + UMVec3d(hw, 2 * y - hh + height, 0)
			));
		initial_box_ = box_;
	}

	return 0;
#else
	return 0;
#endif
}
	
/**
 * add text node panel
 */
int UMGUIBoard::add_text_node_panel(
	int screen_width, 
	int screen_height, 
	int x, 
	int y, 
	int width, 
	int height,
	const UMVec4d& color)
{
	add_color_panel(
		screen_width, 
		screen_height,
		x, 
		y, 
		width,
		height,
		color);
	is_node_ = true;

	return 0;
}

void UMGUIBoard::on_left_button_down(double x, double y)
{
	if (is_root()) { return; }
	if (is_node())
	{
		printf("left mouse down %f, %f\n", x, y);
		is_left_dragging_ = true;
	}
}

static void move_children(UMGUIObjectPtr obj, double x, double y)
{
	umbase::UMMat44d& local = obj->mutable_local_transform();
	umbase::UMMat44d& global = obj->mutable_global_transform();
	const UMBox& intial_box = obj->initial_box();
	local.m[3][0] = x - intial_box.minimum().x;
	local.m[3][1] = -(y - intial_box.minimum().y);
	global.m[3][0] = x - intial_box.minimum().x;
	global.m[3][1] = -(y - intial_box.minimum().y);
	UMGUIObjectList::iterator it = obj->mutable_children().begin();
	for (; it != obj->mutable_children().end(); ++it)
	{
		move_children(*it, x, y);
	}
}

void UMGUIBoard::on_left_button_move(double x, double y)
{
	if (is_root()) { return; }
	if (is_node() && is_left_dragging_)
	{
		printf("left draging %f, %f\n", x, y);
		UMGUIBoardPtr board = self_.lock();
		if (board)
		{
			move_children(board, x, y);
			update(true);
			update_box(true);
		}
	}
}

void UMGUIBoard::on_left_button_up(double x, double y)
{
	if (is_root()) { return; }
	if (is_node())
	{
		if (intersect(x, y))
		{
			printf("inner up\n");
		}
		else
		{
			printf("outer up\n");
		}
		is_left_dragging_ = false;
	}
}

} // umgui
