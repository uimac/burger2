/**
 * @file UMDirectX11GUIScene.h
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

namespace umgui
{

class UMDirectX11GUIScene;
typedef std::shared_ptr<UMDirectX11GUIScene> UMDirectX11GUIScenePtr;

class UMDirectX11GUIScene : public UMGUI
{
	DISALLOW_COPY_AND_ASSIGN(UMDirectX11GUIScene);
public:
	UMDirectX11GUIScene();
	~UMDirectX11GUIScene();
	
	/**
	 * initialize
	 */
	bool init(UMGUIScenePtr scene);

	/**
	 * create view/depth
	 */
	bool clear();
	
	/**
	 * update
	 */
	bool update();

	/**
	 * draw scene
	 */
	bool draw();
	
	/**
	 * resize
	 */
	void resize(int width, int height);

private:
	class SceneImpl;
	std::unique_ptr<SceneImpl> impl_;
};

} // umabc
