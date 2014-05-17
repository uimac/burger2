/**
 * @file UMDirectX11AbcScene.h
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

class UMDirectX11AbcScene;
typedef std::shared_ptr<UMDirectX11AbcScene> UMDirectX11AbcScenePtr;
typedef std::vector<UMDirectX11AbcScenePtr> UMDirectX11AbcSceneList;

class UMDirectX11AbcSceneImpl;
typedef std::shared_ptr<UMDirectX11AbcSceneImpl> UMDirectX11AbcSceneImplPtr;

class UMDirectX11AbcScene : public UMAbc
{
	DISALLOW_COPY_AND_ASSIGN(UMDirectX11AbcScene);
public:

	UMDirectX11AbcScene();
	~UMDirectX11AbcScene();
	
	static UMDirectX11AbcScenePtr create();

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

	/** 
	 * set scene
	 */
	void set_umdraw_scene(umdraw::UMScenePtr scene);
	
private:
	class SceneImpl;
	typedef std::unique_ptr<SceneImpl> SceneImplPtr;
	SceneImplPtr impl_;
};

} // umabc
