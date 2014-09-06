/**
 * @file UMDirectX11AbcScene.cpp
 * any
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_DIRECTX

#include "UMDirectX11AbcScene.h"

#include <memory>
#include <Alembic/Abc/All.h>
#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreFactory/All.h>

#include "UMAbcObject.h"
#include "UMStringUtil.h"
#include "UMPath.h"
#include "UMAbcCallback.h"
#include "UMMaterial.h"
#include "UMAbcIO.h"
#include "UMAbcMesh.h"
#include "UMAbcPoint.h"

#include "UMSoftwareEventType.h"
#include "UMListenerConnector.h"
#include "UMListener.h"
#include "UMAny.h"

#include "UMScene.h"
#include "UMCamera.h"
#include "UMDirectX11.h"
#include "UMDirectX11Mesh.h"
#include "UMDirectX11Material.h"
#include "UMDirectX11AbcMesh.h"
#include "UMDirectX11ShaderManager.h"
#include "UMDirectX11DrawParameter.h"
#include "UMAbcScene.h"
#include "UMDirectX11IO.h"

namespace umabc
{
	using namespace Alembic::Abc;
	using namespace Alembic::AbcGeom;
	using namespace umdraw;
	
class UMDirectX11AbcSceneListener;
typedef std::shared_ptr<UMDirectX11AbcSceneListener> UMDirectX11AbcSceneListenerPtr;
class UMDirectX11AbcSceneListener : public umbase::UMListener
{
public:
	UMDirectX11AbcSceneListener(
		UMAbcScenePtr scene, 
		umdraw::UMDirectX11DrawParameterPtr parameter)
		: scene_(scene)
		, parameter_(parameter)
	{
	}

	virtual void update(umbase::UMEventType event_type, umbase::UMAny& parameter)
	{
		if (event_type == eSoftwareEventCameraChaged)
		{
			if (ID3D11Device* device_pointer = reinterpret_cast<ID3D11Device*>(umdraw::UMDirectX11::current_device_pointer()))
			{
				umdraw::UMCameraPtr scene_camera = umbase::any_cast<umdraw::UMCameraPtr>(parameter);
				if (scene_ && parameter_)
				{
					// camera
					UMDirectX11CameraPtr camera = UMDirectX11IO::convert_camera_to_dx11_camera(device_pointer, scene_camera);
					parameter_->set_camera(camera);
				}
			}
		}
	}
private:
	UMAbcScenePtr scene_;
	umdraw::UMDirectX11DrawParameterPtr parameter_;
};

class UMDirectX11AbcScene::SceneImpl : public umbase::UMListenerConnector
{
	DISALLOW_COPY_AND_ASSIGN(SceneImpl);
public:

	SceneImpl() {}
	~SceneImpl() {
	}
	
	bool init(UMAbcScenePtr scene)
	{
		if (!scene) return false;

		if (scene->init(scene))
		{
			if (umdraw::UMScenePtr draw_scene = scene->umdraw_scene())
			{
				draw_scene_ = draw_scene;
				scene_ = scene;
				init_shader_manager();
				assign_shaders(scene->root_object());
				assign_material_map(scene->root_object(), scene->material_map());
				
				listener_ = std::make_shared<UMDirectX11AbcSceneListener>(scene, dx_draw_parameter_);
				mutable_event_list().clear();
				mutable_event_list().push_back(draw_scene->camera_change_event());
				connect(listener_);

				return true;
			}
		}
		return false;
	}

	bool dispose()
	{
		if (UMAbcScenePtr scene = scene_)
		{
			return scene->dispose();
		}
		return false;
	}
	
	bool draw()
	{
		if (UMAbcScenePtr scene = scene_)
		{
			if (UMAbcObjectPtr root = scene->root_object())
			{
				root->draw(true, UMAbc::eDirectX);
			}
		}
		return false;
	}
	
	bool update(unsigned long time)
	{
		if (UMAbcScenePtr scene = scene_)
		{
			return scene_->update(time);
		}
		return false;
	}
	
	bool clear()
	{
		return true;
	}

private:
	UMDirectX11AbcSceneListenerPtr listener_;
	UMAbcScenePtr scene_;
	UMDirectX11ShaderManagerPtr dx_shader_manager_;
	umdraw::UMDirectX11DrawParameterPtr dx_draw_parameter_;
	umdraw::UMSceneWeakPtr draw_scene_;

	void init_shader_manager()
	{
		dx_shader_manager_ = std::make_shared<UMDirectX11ShaderManager>();
			
		// init shader manager
		dx_shader_manager_->init(
			reinterpret_cast<ID3D11Device*>(umdraw::UMDirectX11::current_device_pointer()), 
			UMDirectX11ShaderManager::eModel);

		dx_draw_parameter_ = std::make_shared<umdraw::UMDirectX11DrawParameter>();
		dx_draw_parameter_->set_shader_manager(dx_shader_manager_);
	}

	void assign_shaders(UMAbcObjectPtr root)
	{
		if (!root) return;
		
		if (UMAbcMeshPtr child = std::dynamic_pointer_cast<UMAbcMesh>(root))
		{
			if (UMDirectX11AbcMeshPtr dx_abc_mesh = child->directx_mesh())
			{
				if (UMDirectX11MeshPtr dx_mesh = dx_abc_mesh->directx_mesh())
				{
					dx_mesh->set_draw_parameter(dx_draw_parameter_);
				}
			}
		}
		else if (UMAbcPointPtr child = std::dynamic_pointer_cast<UMAbcPoint>(root))
		{
			//if (draw_type == UMAbc::eDirectX)
			//{
			//	if (UMDirectX11AbcMeshPtr dx_abc_point = child->directx_point())
			//	{
			//		if (UMDirectX11MeshPtr dx_point = dx_abc_point->directx_point())
			//		{
			//			dx_point->set_draw_parameter(dx_shader_manager_);
			//		}
			//	}
			//}
		}

		for (UMAbcObjectList::iterator it = root->mutable_children().begin();
			it != root->mutable_children().end(); 
			++it)
		{
			UMAbcObjectPtr child_object = *it;
			assign_shaders(child_object);
		}
	}

	void assign_material_map(
		UMAbcObjectPtr root,
		umdraw::UMMaterialMap& material_map)
	{
		if (!root) return;
		
		if (UMAbcMeshPtr child = std::dynamic_pointer_cast<UMAbcMesh>(root))
		{
			umstring name = umbase::UMStringUtil::utf8_to_utf16(child->name());

			if (material_map.find(name) != material_map.end())
			{
				if (umdraw::UMMaterialPtr material = material_map[name])
				{
					material->set_polygon_count(child->polygon_count());
					if (UMDirectX11AbcMeshPtr dx_abc_mesh = child->directx_mesh())
					{
						if (UMDirectX11MeshPtr dx_mesh = dx_abc_mesh->directx_mesh())
						{
							if (dx_mesh->mutable_material_list().empty())
							{
								UMDirectX11MaterialPtr dx_material = std::make_shared<UMDirectX11Material>(material);
								dx_material->init(reinterpret_cast<ID3D11Device*>(umdraw::UMDirectX11::current_device_pointer()));
									
								// use diffuse only for view
								dx_material->set_specular(UMVec4f(0.9f, 0.9f, 0.9f, 1.0f));
								dx_material->set_ambient(UMVec4f(0.3f, 0.3f, 0.3f, 1.0f));

								dx_mesh->mutable_material_list().push_back(dx_material);
							}
						}
					}
				}
			}
		}

		for (UMAbcObjectList::iterator it = root->mutable_children().begin();
			it != root->mutable_children().end(); 
			++it)
		{
			UMAbcObjectPtr child_object = *it;
			assign_material_map(child_object, material_map);
		}
	}
};

/**
 * factory
 */
UMDirectX11AbcScenePtr UMDirectX11AbcScene::create()
{
	return std::make_shared<UMDirectX11AbcScene>();
}

/**
 * constructor
 */
UMDirectX11AbcScene::UMDirectX11AbcScene() :
	impl_(new UMDirectX11AbcScene::SceneImpl())
{
}

/**
 * destructor
 */
UMDirectX11AbcScene::~UMDirectX11AbcScene()
{
}

/**
 * initialize
 */
bool UMDirectX11AbcScene::init(UMAbcScenePtr scene)
{
	return impl_->init(scene);
}

/**
 * release all scenes. call this function before delete.
 */
bool UMDirectX11AbcScene::dispose()
{
	impl_->dispose();
	impl_ = SceneImplPtr();
	return true;
}

/**
 * update
 */
bool UMDirectX11AbcScene::update(unsigned long time)
{
	return impl_->update(time);
}

/**
 * draw
 */
bool UMDirectX11AbcScene::draw()
{
	return impl_->draw();
}

/**
 * clear view/depth
 */
bool UMDirectX11AbcScene::clear()
{
	return impl_->clear();
}

} // umabc

#endif // WITH_DIRECTX