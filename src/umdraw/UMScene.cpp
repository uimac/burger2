/**
 * @file UMScene.cpp
 * 3D scene including many objects.
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#include "UMScene.h"

#include "UMEvent.h"
#include "UMVector.h"
#include "UMMesh.h"

#include "UMStringUtil.h"
#include "UMPath.h"
#include "UMIO.h"
#include "UMSoftwareIO.h"
#include "UMSoftwareEventType.h"

namespace umdraw
{

/** 
 * init scene
 */
void UMScene::init(int width, int height)
{
	camera_change_event_ = std::make_shared<umbase::UMEvent>(eSoftwareEventCameraChaged);

	width_ = width;
	height_ = height;

	light_list_.clear();
	camera_ = UMCameraPtr(new UMCamera(false, width, height));
}

/**
 * load model data
 */
bool UMScene::load(const std::u16string& absolute_file_path)
{
	umio::UMIO io;
	umio::UMObjectPtr obj;

	umio::UMIOSetting setting = umio::UMIOSetting();
	setting.set_bl_imp_bool_prop(umio::UMIOSetting::eUMImpTriangulate, true);
	setting.set_system_unit_type(umio::UMIOSetting::eFbxSystemUnitM);
	obj = io.load(umbase::UMStringUtil::utf16_to_utf8(absolute_file_path), setting);
	if (!obj) return false;
	
	// import to umdraw
	UMMeshGroupPtr mesh_group(std::make_shared<UMMeshGroup>());
	{
		if (!UMSoftwareIO::import_mesh_list(
			mesh_group->mutable_mesh_list(), 
			obj,
			absolute_file_path))
		{
			return false;
		}
		mutable_mesh_group_list().push_back(mesh_group);
	}
	return true;
}

/**
 * load model data from memory
 */
bool UMScene::load_from_memory(const std::string& src)
{
	umio::UMIO io;
	umio::UMObjectPtr obj;

	umio::UMIOSetting setting = umio::UMIOSetting();
	setting.set_bl_imp_bool_prop(umio::UMIOSetting::eUMImpTriangulate, true);
	setting.set_system_unit_type(umio::UMIOSetting::eFbxSystemUnitM);
	obj = io.load_bos_from_memory(src, setting);
	if (!obj) return false;
	
	// import to umdraw
	UMMeshGroupPtr mesh_group(std::make_shared<UMMeshGroup>());
	{
		if (!UMSoftwareIO::import_mesh_list(
			mesh_group->mutable_mesh_list(), 
			obj,
			std::u16string()))
		{
			return false;
		}
		mutable_mesh_group_list().push_back(mesh_group);
	}
	return true;
}

/**
 * get total polygons
 */
size_t UMScene::total_polygon_size() const
{
	size_t polygons = 0;
	UMMeshGroupList::const_iterator it = mesh_group_list().begin();
	for (; it != mesh_group_list().end(); ++it)
	{
		UMMeshList::const_iterator mt = (*it)->mesh_list().begin();
		for (; mt != (*it)->mesh_list().end(); ++mt)
		{
			polygons += (*mt)->face_list().size();
		}
	}
	return polygons;
}

/**
 * set camera
 */
void UMScene::set_camera(UMCameraPtr camera)
{
	camera_ = camera;
	umbase::UMEvent::Parameter parameter(camera);
	camera_change_event_->set_parameter(parameter);
	camera_change_event_->notify();
}

} // umdraw
