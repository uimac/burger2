/**
 * @file UMGUIBoard.h
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <vector>
#include <memory>
#include "UMMacro.h"
#include "UMGUIObject.h"
#include "UMMathTypes.h"

namespace umdraw
{
class UMMesh;
typedef std::shared_ptr<UMMesh> UMMeshPtr;
}

namespace umimage
{
class UMImage;
typedef std::shared_ptr<UMImage> UMImagePtr;
}

namespace umgui
{

class UMGUIBoard;
typedef std::shared_ptr<UMGUIBoard> UMGUIBoardPtr;
typedef std::weak_ptr<UMGUIBoard> UMBoardWeakPtr;
typedef std::vector<UMGUIBoardPtr> UMGUIBoardList;

/**
 * a gui board
 */
class UMGUIBoard : public UMGUIObject
{
	DISALLOW_COPY_AND_ASSIGN(UMGUIBoard);
public:
	~UMGUIBoard() {}

	/**
	 * add root board
	 */
	static UMGUIBoardPtr create_root_board(
		int screen_width, 
		int screen_height);
	
	/**
	 * add color circle board
	 */
	static UMGUIBoardPtr create_color_circle_board(
		int screen_width, 
		int screen_height,
		int x, 
		int y, 
		int width, 
		int height,
		int depth);
	
	/**
	 * create empty board
	 */
	static UMGUIBoardPtr create_board(int depth);

	/**
	 * add color panel
	 */
	int add_color_panel(
		int screen_width, 
		int screen_height, 
		int x, 
		int y, 
		int width, 
		int height,
		const UMVec4d& color);
	
	/**
	 * add uv panel
	 */
	int add_uv_panel(
		int screen_width, 
		int screen_height, 
		int x, 
		int y, 
		int width, 
		int height);

	/**
	 * add texture panel
	 */
	int add_texture_panel(
		int screen_width, 
		int screen_height, 
		int x, 
		int y, 
		int width, 
		int height,
		const umstring& path);
	
	/**
	 * add texture panel
	 */
	int add_texture_panel(
		int screen_width, 
		int screen_height, 
		int x, 
		int y, 
		int width, 
		int height,
		umimage::UMImagePtr image);

	/**
	 * add text panel
	 */
	int add_text_panel(
		int screen_width, 
		int screen_height,
		int x, 
		int y, 
		int font_size,
		const umtextstring& text);
	
	/**
	 * add text node panel
	 */
	int add_text_node_panel(
		int screen_width, 
		int screen_height, 
		int x, 
		int y, 
		int width, 
		int height,
		const UMVec4d& color);
	
	/**
	 * get mesh
	 */
	virtual umdraw::UMMeshPtr mesh() { return mesh_; }


protected:

	virtual void on_left_button_down(double x, double y);
	virtual void on_left_button_move(double x, double y);
	virtual void on_left_button_up(double x, double y);

private:
	UMGUIBoard();
	UMGUIBoard(int depth);

	int x_;
	int y_;
	int width_;
	int height_;
	int depth_;
	bool is_left_dragging_;
	umdraw::UMMeshPtr mesh_;
	UMBoardWeakPtr self_;
};

} // umgui
