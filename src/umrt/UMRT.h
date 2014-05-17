/**
 * @file UMRT.h
 * rays interface
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#pragma once

#include <memory>
#include "UMMacro.h"
#include "UMVector.h"
#include "UMMathTypes.h"

namespace umabc
{
	class UMAbcScene;
	typedef std::shared_ptr<UMAbcScene> UMAbcScenePtr;
} // umabc

namespace umdraw
{
	class UMScene;
	typedef std::shared_ptr<UMScene> UMScenePtr;
} // umdraw

namespace umimage
{
	class UMImage;
	typedef std::shared_ptr<UMImage> UMImagePtr;
} // umimage

namespace umrt
{

class UMSceneAccess;
typedef std::shared_ptr<UMSceneAccess> UMSceneAccessPtr;

class UMRT;
typedef std::shared_ptr<UMRT> UMRTPtr;

/**
 * rays
 */
class UMRT 
{
	DISALLOW_COPY_AND_ASSIGN(UMRT);
public:
	UMRT();
	~UMRT();

	/**
	 * init
	 */
	bool init();

	/**
	 * add umdraw scene
	 */
	bool add_scene(umdraw::UMScenePtr scene);
	
	/**
	 * add umabc scene
	 */
	bool add_abc_scene(umabc::UMAbcScenePtr abc_scene);
	
	/**
	 * render
	 */
	umimage::UMImagePtr render();

	/**
	 * subdiv test
	 */
	bool subdiv_test();

	/**
	 * get scene access
	 */
	UMSceneAccessPtr scene_access() { return scene_access_; }

private:
	UMSceneAccessPtr scene_access_;
};

} // umrt
