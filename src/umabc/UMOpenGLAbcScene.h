/**
 * @file UMOpenGLAbcScene.h
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
#include "UMAbcSetting.h"
#include "UMAbcCallback.h"
#include "UMAbc.h"

namespace umdraw
{
	class UMScene;
	typedef std::shared_ptr<UMScene> UMScenePtr;
	typedef std::weak_ptr<UMScene> UMSceneWeakPtr;
} // umdraw

/// uimac alembic library
namespace umabc
{

class UMOpenGLAbcScene;
typedef std::shared_ptr<UMOpenGLAbcScene> UMOpenGLAbcScenePtr;
typedef std::vector<UMOpenGLAbcScenePtr> UMOpenGLAbcSceneList;

class UMOpenGLAbcScene : public UMAbc
{
	DISALLOW_COPY_AND_ASSIGN(UMOpenGLAbcScene);
public:

	UMOpenGLAbcScene();
	~UMOpenGLAbcScene();

	static UMOpenGLAbcScenePtr create();
	
	/**
	 * initialize
	 */
	virtual bool init(UMAbcScenePtr scene);

	/**
	 * release all resources. call this function before delete.
	 */
	virtual bool dispose();
	
	/** 
	 * refresh scene
	 */
	virtual bool draw();

	/**
	 * update scene
	 */
	virtual bool update(unsigned long time);
	
	/**
	 * clear frame
	 */
	virtual bool clear();

private:
	class SceneImpl;
	typedef std::unique_ptr<SceneImpl> SceneImplPtr;
	SceneImplPtr impl_;
};

} // umabc
