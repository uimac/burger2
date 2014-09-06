/**
 * @file UMGUIScrollBoard.h
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
#include "UMGUIBoard.h"

namespace umdraw
{
class UMMesh;
typedef std::shared_ptr<UMMesh> UMMeshPtr;
}


namespace umgui
{

class UMGUIScrollBoard;
typedef std::shared_ptr<UMGUIScrollBoard> UMGUIScrollBoardPtr;
typedef std::weak_ptr<UMGUIScrollBoard> UMGUIScrollBoardWeakPtr;

/**
 * a gui board
 */
class UMGUIScrollBoard : public UMGUIObject
{
	DISALLOW_COPY_AND_ASSIGN(UMGUIScrollBoard);
public:
	UMGUIScrollBoard() 
		: is_bar_pressed_(false)
		, pre_y_(0) 
	{}
	~UMGUIScrollBoard() {}

	/**
	 * add color panel
	 */
	static UMGUIScrollBoardPtr create_board(
		int screen_width, 
		int screen_height, 
		int x, 
		int y, 
		int depth,
		int width, 
		int height,
		const UMVec4d& color);
	
	int add_scroll_bar(
		int screen_width, 
		int screen_height, 
		int depth,
		const UMVec4d& color);

	/**
	 * get mesh
	 */
	virtual umdraw::UMMeshPtr mesh() { return umdraw::UMMeshPtr(); }

protected:
	virtual void on_left_button_down(double x, double y);
	virtual void on_left_button_move(double x, double y);
	virtual void on_left_button_up(double x, double y);
	virtual void on_scroll(double x, double y);

private:
	UMBox whole_box();
	UMGUIBoardPtr gui_board_;
	UMGUIBoardPtr bar_board_;
	UMGUIScrollBoardWeakPtr self_;
	void scroll_recusive(UMGUIObjectPtr obj, double xscroll, double yscroll);
	void set_visible_recusive(UMGUIObjectPtr obj);
	void extend_box_recusive(UMGUIObjectPtr obj, UMBox& box);
	bool is_bar_pressed_;
	double pre_y_;
	void scroll(double yscroll);
};

} // umgui
