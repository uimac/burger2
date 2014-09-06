/**
 * @file UMGUI.h
 * gui interface
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <memory>
#include <vector>
#include "UMMacro.h"
#include "UMVector.h"
#include "UMMathTypes.h"

namespace umgui
{
	
class UMGUIScene;
typedef std::shared_ptr<UMGUIScene> UMGUIScenePtr;
typedef std::vector<UMGUIScenePtr> UMGUISceneList;

class UMGUI;
typedef std::shared_ptr<UMGUI> UMGUIPtr;

/**
 * gui
 */
class UMGUI
{
	DISALLOW_COPY_AND_ASSIGN(UMGUI);
public:
	enum DrawType {
		eSoftware,
		eOpenGL,
		eDirectX,
	};

	~UMGUI();
	
	/**
	 * release all scenes. call this function before delete.
	 */
	void dispose(){}

	/**
	 * create instance
	 */
	static UMGUIPtr create(DrawType type);

	/**
	 * init
	 */
	virtual bool init(UMGUIScenePtr scene) = 0;

	/**
	 * update
	 */
	virtual bool update() = 0;

	/**
	 * draw frame
	 */
	virtual bool draw() = 0;

	/**
	 * resize
	 */
	virtual void resize(int width, int height) = 0;
	
	/**
	 * keyboard
	 * @return stop propagation or not
	 */
	virtual bool on_keyboard(int key, int action) = 0;
	
	/**
	 * mouse button up/down
	 * @return stop propagation or not
	 */
	virtual bool on_mouse(int button, int action) = 0;
	
	/**
	 * mouse move
	 * @return stop propagation or not
	 */
	virtual bool on_mouse_move(double x, double y) = 0;
	
	/**
	 * scroll
	 * @return stop propagation or not
	 */
	virtual bool on_scroll(double x, double y) = 0;

protected:
	UMGUI() {}
	
private:
	//UMGUISceneList scene_list_;
	DrawType draw_type_;
};

} // umgui
