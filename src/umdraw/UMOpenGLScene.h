/**
 * @file UMOpenGLScene.h
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <string>
#include <memory>
#include "UMMacro.h"
#include "UMScene.h"

#include "UMListener.h"

#include "UMMeshGroup.h"
#include "UMOpenGLMeshGroup.h"
#include "UMOpenGLShaderManager.h"
#include "UMOpenGLTexture.h"
#include "UMOpenGLLight.h"
#include "UMOpenGLCamera.h"
#include "UMTime.h"

namespace umdraw
{

class UMOpenGLScene;
typedef std::shared_ptr<UMOpenGLScene> UMOpenGLScenePtr;

class UMOpenGLDrawParameter;
typedef std::shared_ptr<UMOpenGLDrawParameter> UMOpenGLDrawParameterPtr;

class UMOpenGLLine;
typedef std::shared_ptr<UMOpenGLLine> UMOpenGLLinePtr;
typedef std::vector<UMOpenGLLinePtr> UMOpenGLLineList;

/**
 * opengl scene
 */
class UMOpenGLScene : public umbase::UMListener
{
	DISALLOW_COPY_AND_ASSIGN(UMOpenGLScene);

public:

	UMOpenGLScene();

	~UMOpenGLScene();
	
	/**
	 * init scene
	 */
	bool init(UMScenePtr scene);
	
	/**
	 * clear scene
	 */
	void clear(UMScenePtr scene);

	/**
	 * load scene
	 */
	bool load(UMScenePtr scene);
	
	/**
	 * update scene
	 */
	bool update();

	/**
	 * clear
	 */
	bool clear();

	/**
	 * draw scene
	 */
	bool draw();
	
	/**
	 * get umdraw renderer scene
	 */
	UMScenePtr scene() const;

	/**
	 * update event
	 */
	virtual void update(umbase::UMEventType event_type, umbase::UMAny& parameter);

private:
	class Impl;
	typedef std::unique_ptr<Impl> ImplPtr;
	ImplPtr impl_;
};

} // umdraw
