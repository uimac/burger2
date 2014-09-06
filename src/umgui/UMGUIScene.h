/**
 * @file UMGUIScene.h
 * any
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <memory>
#include <string>
#include <vector>
#include "UMMacro.h"
#include "UMGUI.h"

namespace umdraw
{
	class UMCamera;
	typedef std::shared_ptr<UMCamera> UMCameraPtr;
	typedef std::weak_ptr<UMCamera> UMCameraWeakPtr;

	class UMLight;
	typedef std::shared_ptr<UMLight> UMLightPtr;

	class UMScene;
	typedef std::shared_ptr<UMScene> UMScenePtr;
	typedef std::weak_ptr<UMScene> UMSceneWeakPtr;
} // umdraw

/// uimac gui library
namespace umgui
{

class UMGUIScene;
typedef std::shared_ptr<UMGUIScene> UMGUIScenePtr;
typedef std::vector<UMGUIScenePtr> UMGUISceneList;

class UMGUIObject;
typedef std::shared_ptr<UMGUIObject> UMGUIObjectPtr;

class UMGUIScene
{
	DISALLOW_COPY_AND_ASSIGN(UMGUIScene);
public:

	UMGUIScene();
	virtual ~UMGUIScene();
	
	/**
	 * initialize gui components
	 */
	virtual bool init(int width, int height);

	/** 
	 * set umdraw scene
	 */
	void set_umdraw_scene(umdraw::UMScenePtr scene);

	/**
	 * get umdraw scene
	 */
	umdraw::UMScenePtr umdraw_scene();

	/**
	 * get camera
	 */
	umdraw::UMCameraPtr camera();
	
	/**
	 * get light
	 */
	umdraw::UMLightPtr light();

	/**
	 * get root object;
	 */
	UMGUIObjectPtr root_object();
	
	/**
	 * keyboard
	 */
	bool on_keyboard(int key, int action);

	/**
	 * mouse button up/down
	 */
	bool on_mouse(int button, int action);

	/**
	 * mouse move
	 */
	bool on_mouse_move(double x, double y);
	
	/**
	 * scroll
	 */
	bool on_scroll(double x, double y);

private:
	class SceneImpl;
	std::unique_ptr<SceneImpl> impl_;
};

} // UMGUI
