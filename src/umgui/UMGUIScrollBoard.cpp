/**
 * @file UMGUIScrollBoard.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */

#include "UMGUIScrollBoard.h"
#include "UMMesh.h"
#include "UMMaterial.h"
#include "UMPath.h"
#include "UMStringUtil.h"
#include "UMVector.h"
#include "UMResource.h"
#include "UMMatrix.h"

#if !defined(WITH_EMSCRIPTEN)
	#include <tchar.h>
#endif

#include "UMFont.h"
#include "UMTextureAtlas.h"

namespace umgui
{

/**
 * add color panel
 */
UMGUIScrollBoardPtr UMGUIScrollBoard::create_board(
	int screen_width, 
	int screen_height, 
	int x, 
	int y, 
	int depth,
	int width, 
	int height,
	const UMVec4d& color)
{
	UMGUIScrollBoardPtr board = std::make_shared<UMGUIScrollBoard>();
	board->gui_board_ = UMGUIBoard::create_board(depth);
	board->gui_board_->add_color_panel(screen_width, screen_height, x, y, width, height, color);
	board->box_ = board->gui_board_->box();
	board->mutable_children().push_back(board->gui_board_);
	board->self_ = board;
	return board;
}

/**
 * add scroll bar
 */
int UMGUIScrollBoard::add_scroll_bar(
	int screen_width, 
	int screen_height, 
	int depth,
	const UMVec4d& color)
{
	UMGUIScrollBoardPtr board = self_.lock();
	if (board)
	{
		bar_board_ = UMGUIBoard::create_board(depth - 1);
		int x = static_cast<int>(gui_board_->box().maximum().x - 14);
		int y = static_cast<int>(gui_board_->box().minimum().y + 2);
		int width = 12;
		UMBox box = whole_box();
		double board_height = gui_board_->box().maximum().y - gui_board_->box().minimum().y - 4;
		double whole_height = box.maximum().y - box.minimum().y;
		int height = static_cast<int>(board_height * board_height / whole_height);
		bar_board_->add_color_panel(screen_width, screen_height, x, y, width, height, color);
		board->mutable_children().push_back(bar_board_);
	}
	return 0;
}

void UMGUIScrollBoard::on_left_button_down(double x, double y)
{
	printf("left mouse down %f, %f\n", x, y);
	if (bar_board_->intersect(x, y))
	{
		is_bar_pressed_ = true;
		pre_y_ = y;
	}
}

void UMGUIScrollBoard::on_left_button_move(double x, double y)
{
	if (is_bar_pressed_)
	{
		double yscroll = (y - pre_y_) * 2.0;
		scroll(yscroll);
		pre_y_ = y;
	}
}

void UMGUIScrollBoard::on_left_button_up(double x, double y)
{
	if (intersect(x, y))
	{
		printf("inner up\n");
	}
	else
	{
		printf("outer up\n");
	}
	is_bar_pressed_ = false;
}

void UMGUIScrollBoard::scroll_recusive(UMGUIObjectPtr obj, double xscroll, double yscroll)
{
	if (!obj) { return; }
	umbase::UMMat44d& local = obj->mutable_local_transform();
	umbase::UMMat44d& global = obj->mutable_global_transform();
	local.m[3][1] += yscroll;
	global.m[3][1] += yscroll;
	
	UMGUIObjectList::const_iterator it = obj->mutable_children().begin();
	for (; it != obj->mutable_children().end(); ++it)
	{
		UMGUIObjectPtr child = *it;
		scroll_recusive(child, 0, yscroll);
	}
}

void UMGUIScrollBoard::set_visible_recusive(UMGUIObjectPtr obj)
{
	if (!obj) { return; }
	int top_y = static_cast<int>(obj->box().minimum().y);
	int bottom_y = static_cast<int>(obj->box().maximum().y);
	int half_height = static_cast<int>((obj->box().maximum().y - obj->box().minimum().y) - 2);
	if  ( (bottom_y - half_height) <= box_.minimum().y
		|| (top_y + half_height) >= box_.maximum().y)
	{
		obj->set_visible(false);
	}
	else
	{
		obj->set_visible(true);
	}
	
	UMGUIObjectList::const_iterator it = obj->mutable_children().begin();
	for (; it != obj->mutable_children().end(); ++it)
	{
		UMGUIObjectPtr child = *it;
		set_visible_recusive(child);
	}
}

void UMGUIScrollBoard::extend_box_recusive(UMGUIObjectPtr obj, UMBox& box)
{
	box.extend(obj->box());
	UMGUIObjectList::const_iterator it = obj->mutable_children().begin();
	for (; it != obj->mutable_children().end(); ++it)
	{
		UMGUIObjectPtr child = *it;
		extend_box_recusive(child, box);
	}
}

/**
 * get whole box
 */
UMBox UMGUIScrollBoard::whole_box()
{
	UMBox box;
	UMGUIObjectList::const_iterator it = mutable_children().begin();
	for (; it != mutable_children().end(); ++it)
	{
		UMGUIObjectPtr obj = *it;
		if (obj != gui_board_ && obj != bar_board_)
		{
			extend_box_recusive(obj, box);
		}
	}
	return box;
}

/**
 * scroll event
 */
void UMGUIScrollBoard::on_scroll(double x, double y)
{
	//printf("on_scroll %f, %f\n", x, y);

	double yscroll = - y * 40;
	scroll(yscroll);
}

/**
 * do scroll
 */
void UMGUIScrollBoard::scroll(double yscroll)
{
	double bar_ration = 0.0;
	
	// get box of children 
	{
		UMBox box = whole_box();
		if (box.minimum().y - yscroll > box_.minimum().y) 
		{
			yscroll = box.minimum().y - box_.minimum().y;
		}
		if (box.maximum().y - yscroll < box_.maximum().y)
		{
			yscroll = box.maximum().y - box_.maximum().y;
		}
		double board_height = gui_board_->box().maximum().y - gui_board_->box().minimum().y - 4;
		double whole_height = box.maximum().y - box.minimum().y;
		bar_ration = board_height / whole_height;
	}

	// scroll bar
	{
		umbase::UMMat44d& local = bar_board_->mutable_local_transform();
		umbase::UMMat44d& global = bar_board_->mutable_global_transform();
		local.m[3][1] -= yscroll * bar_ration;
		global.m[3][1] -= yscroll * bar_ration;
		bar_board_->update(false);
		bar_board_->update_box(false);
	}

	// scroll childrens
	{
		UMGUIObjectList::const_iterator it = mutable_children().begin();
		for (; it != mutable_children().end(); ++it)
		{
			UMGUIObjectPtr obj = *it;
			if (obj != gui_board_ && obj != bar_board_)
			{
				scroll_recusive(obj, 0, yscroll);
				obj->update(true);
				obj->update_box(true);
				set_visible_recusive(obj);
			}
		}
	}
}

} // umgui
