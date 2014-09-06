/**
 * @file UMRayScene.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#include "UMRT.h"
#include <string>
#include <assert.h>

#include "UMStringUtil.h"
#include "UMPath.h"
#include "UMMesh.h"
#include "UMMeshGroup.h"
#include "UMMaterial.h"
#include "UMImage.h"
#include "UMLight.h"
#include "UMVertexParameter.h"
#include "UMSceneAccess.h"
#include "UMRenderParameter.h"
#include "UMBvh.h"
#include "UMRenderer.h"

namespace umrt
{

/**
 * constructor
 */
UMRT::UMRT()
	: scene_access_(std::make_shared<UMSceneAccess>())
{
}

/**
 * destructor
 */
UMRT::~UMRT()
{
}

/**
 * init
 */
bool UMRT::init()
{
	return scene_access_->init();
}

/**
 * add umdraw scene
 */ 
bool UMRT::add_scene(umdraw::UMScenePtr scene)
{
	scene_access_->add_scene(scene);
	if (scene_access_->update_bvh())
	{
		return true;
	}
	return false;
}

/**
 * add umdraw scene
 */ 
bool UMRT::add_abc_scene(umabc::UMAbcScenePtr scene)
{
	scene_access_->add_abc_scene(scene);
	if (scene_access_->update_bvh())
	{
		UMBvhPtr bvh = scene_access_->bvh();
		scene_access_->mutable_render_primitive_list().clear();
		scene_access_->mutable_render_primitive_list().push_back(bvh);
		return true;
	}
	return false;
}


/**
 * render
 */
UMImagePtr UMRT::render()
{
	if (!scene_access_) return UMImagePtr();
	
	UMRenderParameter param(800, 600);
	{
		UMRendererPtr renderer = UMRenderer::create(UMRenderer::eToonRender);
		renderer->set_width(800);
		renderer->set_height(600);
		renderer->render(scene_access_, param);
	}
	return param.output_image();
}

/**
 * subdiv test
 */
bool UMRT::subdiv_test()
{
	return scene_access_->subdivide(48, 2);
}


} // umrt
