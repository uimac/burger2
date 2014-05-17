/**
 * @file UMOpenGLGUIScene.h
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
	
class UMGUIScene;
typedef std::shared_ptr<UMGUIScene> UMGUIScenePtr;

class UMOpenGLGUIScene;
typedef std::shared_ptr<UMOpenGLGUIScene> UMOpenGLGUIScenePtr;

class UMOpenGLGUIScene : public UMGUI
{
	DISALLOW_COPY_AND_ASSIGN(UMOpenGLGUIScene);
public:
	UMOpenGLGUIScene();
	~UMOpenGLGUIScene();
	
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

} // umgui
