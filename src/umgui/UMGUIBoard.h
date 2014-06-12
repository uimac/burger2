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
#include "UMMesh.h"
#include "UMMathTypes.h"

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
	UMGUIBoard() : x_(0), y_(0), width_(0), height_(0), is_root_(false), depth_(0) {}
	UMGUIBoard(int depth) : x_(0), y_(0), width_(0), height_(0), is_root_(false), depth_(depth) {}
	~UMGUIBoard() {}

	/**
	 * root board or not
	 */
	bool is_root() const { return is_root_; }

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
	
	///**
	// * add check box panel
	// */
	//int add_check_box(
	//	int screen_width, 
	//	int screen_height, 
	//	int x, 
	//	int y, 
	//	int width, 
	//	int height,
	//	int radius,
	//	const UMVec4d& color);

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
		UMImagePtr image);

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
	 * get mesh
	 */
	umdraw::UMMeshPtr mesh() { return mesh_; }

private:
	int x_;
	int y_;
	int width_;
	int height_;
	int depth_;
	bool is_root_;
	umdraw::UMMeshPtr mesh_;
};

} // umgui
