/**
 * @file UMOpenGLAbcScene.cpp
 * any
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_OPENGL

#include "UMOpenGLAbcScene.h"

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
#include "UMAbcScene.h"

#include "UMSoftwareEventType.h"
#include "UMListenerConnector.h"
#include "UMListener.h"
#include "UMAny.h"

#include "UMScene.h"
#include "UMCamera.h"
#include "UMOpenGLAbcMesh.h"
#include "UMOpenGLAbcPoint.h"
#include "UMOpenGLMesh.h"
#include "UMOpenGLMaterial.h"
#include "UMOpenGLShaderManager.h"
#include "UMOpenGLLight.h"
#include "UMOpenGLCamera.h"
#include "UMOpenGLPoint.h"
#include "UMOpenGLDrawParameter.h"

#include <GL/glew.h>

#include "UMOpenGLIO.h"

namespace umabc
{
	using namespace Alembic::Abc;
	using namespace Alembic::AbcGeom;
	using namespace umdraw;

class UMOpenGLAbcSceneListener;
typedef std::shared_ptr<UMOpenGLAbcSceneListener> UMOpenGLAbcSceneListenerPtr;
class UMOpenGLAbcSceneListener : public umbase::UMListener
{
public:
	UMOpenGLAbcSceneListener(
		UMAbcScenePtr scene, 
		umdraw::UMOpenGLDrawParameterPtr parameter)
		: scene_(scene)
		, parameter_(parameter)
	{
	}

	virtual void update(umbase::UMEventType event_type, umbase::UMAny& parameter)
	{
		if (event_type == eSoftwareEventCameraChaged)
		{
			umdraw::UMCameraPtr scene_camera = umbase::any_cast<umdraw::UMCameraPtr>(parameter);
			if (scene_ && parameter_)
			{
				// camera
				UMOpenGLCameraPtr camera = UMOpenGLIO::convert_camera_to_gl_camera(scene_camera);
				parameter_->set_camera(camera);
			}
		}
	}
private:
	UMAbcScenePtr scene_;
	umdraw::UMOpenGLDrawParameterPtr parameter_;
};

class UMOpenGLAbcScene::SceneImpl : public umbase::UMListenerConnector
{
	DISALLOW_COPY_AND_ASSIGN(SceneImpl);
public:

	SceneImpl() {}

	~SceneImpl() {}
	
	bool init(UMAbcScenePtr scene)
	{
		if (!scene) return false;

		if (scene->init(scene))
		{
			if (umdraw::UMScenePtr draw_scene = scene->umdraw_scene())
			{
				draw_scene_ = draw_scene;
				scene_ = scene;
				init_gl();
				init_shader_manager();
				assign_shaders(scene->root_object());
				assign_material_map(scene->root_object(), scene->material_map());
				
				listener_ = std::make_shared<UMOpenGLAbcSceneListener>(scene, gl_draw_parameter_);
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
		// draw models
		if (UMAbcScenePtr scene = scene_)
		{
			if (UMAbcObjectPtr root = scene->root_object())
			{
				root->draw(true, UMAbc::eOpenGL);
			}
		}
		return false;
	}
	
	bool update(unsigned long time)
	{
		//update_material(scene_->root_object());
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
	UMOpenGLAbcSceneListenerPtr listener_;
	UMAbcScenePtr scene_;
	UMOpenGLShaderManagerPtr gl_shader_manager_;
	umdraw::UMOpenGLDrawParameterPtr gl_draw_parameter_;
	umdraw::UMSceneWeakPtr draw_scene_;

	void init_gl()
	{
		if (umdraw::UMScenePtr scene = draw_scene_.lock())
		{
			UMOpenGLLightPtr light = umdraw::UMOpenGLIO::convert_light_to_gl_light( scene->light_list().at(0) );

			// camera
			UMCameraPtr camera = scene->camera();
			UMOpenGLCameraPtr gl_camera = umdraw::UMOpenGLIO::convert_camera_to_gl_camera(camera);
			
			gl_draw_parameter_ = std::make_shared<umdraw::UMOpenGLDrawParameter>();
			gl_draw_parameter_->set_camera(gl_camera);
			gl_draw_parameter_->set_light(light);
		}
	}

	void init_shader_manager()
	{
		gl_shader_manager_ = std::make_shared<UMOpenGLShaderManager>();

		// init shader manager for constants
		gl_shader_manager_->init(UMOpenGLShaderManager::eConstants);
		// init shader manager
		gl_shader_manager_->init(UMOpenGLShaderManager::eModel);
		// init shader manager
		gl_shader_manager_->init(UMOpenGLShaderManager::ePoint);
		gl_draw_parameter_->set_shader_manager(gl_shader_manager_);
	}

	void assign_shaders(UMAbcObjectPtr root)
	{
		if (!root) return;
		
		if (UMAbcMeshPtr child = std::dynamic_pointer_cast<UMAbcMesh>(root))
		{
			if (UMOpenGLAbcMeshPtr gl_abc_mesh = child->opengl_mesh())
			{
				if (UMOpenGLMeshPtr gl_mesh = gl_abc_mesh->opengl_mesh())
				{
					gl_mesh->set_draw_parameter(gl_draw_parameter_);
				}
			}
		}
		else if (UMAbcPointPtr child = std::dynamic_pointer_cast<UMAbcPoint>(root))
		{
			if (UMOpenGLAbcPointPtr gl_abc_point = child->opengl_point())
			{
				if (UMOpenGLPointPtr gl_point = gl_abc_point->opengl_point())
				{
					gl_point->set_draw_parameter(gl_draw_parameter_);
				}
			}
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
			
			const std::vector<umstring>& faceset_name_list = child->faceset_name_list();
			if (faceset_name_list.empty())
			{
				if (UMMaterialPtr material = find_material(material_map, name))
				{
					assign_material(child, material, child->polygon_count());
				}
				else
				{
					UMMaterialPtr default_mat = UMMaterial::default_material();
					default_mat->set_name(name);
					assign_material(child, default_mat, child->polygon_count());
				}
			}
			else
			{
				for (int i = 0, size = static_cast<int>(faceset_name_list.size()); i < size; ++i)
				{
					const umstring& faceset_name = faceset_name_list.at(i);
					int polycount = child->faceset_polycount_list().at(i);
					if (UMMaterialPtr material = find_material(material_map, faceset_name))
					{
						assign_material(child, material, polycount);
					}
					else
					{
						UMMaterialPtr default_mat = UMMaterial::default_material();
						default_mat->set_name(faceset_name);
						assign_material(child, default_mat, polycount);
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

	void update_material(UMAbcObjectPtr root)
	{
		if (UMAbcMeshPtr child = std::dynamic_pointer_cast<UMAbcMesh>(root))
		{
			if (UMOpenGLAbcMeshPtr gl_abc_mesh = child->opengl_mesh())
			{
				if (UMOpenGLMeshPtr gl_mesh = gl_abc_mesh->opengl_mesh())
				{
					umdraw::UMOpenGLMaterialList sorted_list;
					sorted_list.reserve(gl_mesh->material_list().size());
					for (umdraw::UMMaterialList::iterator ct = child->mutable_material_list().begin(); ct != child->mutable_material_list().end(); ++ct)
					{
						UMMaterialPtr mat = *ct;
						for (umdraw::UMOpenGLMaterialList::iterator it = gl_mesh->mutable_material_list().begin(); it != gl_mesh->mutable_material_list().end(); ++it)
						{
							umdraw::UMOpenGLMaterialPtr gl_material = *it;
							if (mat->name() == gl_material->name())
							{
								gl_material->set_polygon_count(
									gl_material->ummaterial()->polygon_count());
								sorted_list.push_back(gl_material);
								break;
							}
						}
					}
					if (gl_mesh->material_list().size() == sorted_list.size())
					{
						gl_mesh->mutable_material_list() = sorted_list;
					}
				}
			}
		}
		for (UMAbcObjectList::iterator it = root->mutable_children().begin();
			it != root->mutable_children().end(); 
			++it)
		{
			UMAbcObjectPtr child_object = *it;
			update_material(child_object);
		}
	}


	void assign_material(
		UMAbcMeshPtr target, 
		umdraw::UMMaterialPtr material,
		int polygon_count)
	{
		material->set_polygon_count(polygon_count);
		if (UMOpenGLAbcMeshPtr gl_abc_mesh = target->opengl_mesh())
		{
			if (UMOpenGLMeshPtr gl_mesh = gl_abc_mesh->opengl_mesh())
			{
				//if (gl_mesh->mutable_material_list().empty())
				{
					UMOpenGLMaterialPtr gl_material = std::make_shared<UMOpenGLMaterial>(material);
					gl_material->init();

					// use diffuse only for view
					gl_material->set_specular(UMVec4f(0.9f, 0.9f, 0.9f, 1.0f));
					gl_material->set_ambient(UMVec4f(0.3f, 0.3f, 0.3f, 1.0f));

					gl_mesh->mutable_material_list().push_back(gl_material);
								
					material->set_specular(UMVec4d(0.9f, 0.9f, 0.9f, 1.0f));
					material->set_ambient(UMVec4d(0.3f, 0.3f, 0.3f, 1.0f));

					target->mutable_material_list().push_back(material);
				}
			}
		}
	}

	umdraw::UMMaterialPtr find_material(
		umdraw::UMMaterialMap& material_map, 
		const umstring& name)
	{
		if (material_map.find(name) != material_map.end())
		{
			return material_map[name];
		}
		else
		{
			umdraw::UMMaterialMap::iterator it = material_map.begin();
			std::u16string separate_str = umbase::UMStringUtil::utf8_to_utf16("_");
			for (; it != material_map.end(); ++it)
			{
				umstring material_name = it->first;
				std::u16string::size_type pos = material_name.find_last_of(separate_str);
				if (pos != std::u16string::npos)
				{
					std::u16string subname = material_name.substr(pos + 1);
					// TODO
					if (name == subname)
					{
						it->second->set_name(name);
						return it->second;
					}
				}
			}
		}
		return umdraw::UMMaterialPtr();
	}

	void get_name_list_recursive(std::vector<std::string>& name_list, UMAbcObjectPtr object)
	{
		for (UMAbcObjectList::const_iterator it = scene_->root_object()->children().begin();
			it != scene_->root_object()->children().end();
			++it)
		{
			name_list.push_back((*it)->name());
		}
	}
};

/**
 * factory
 */
UMOpenGLAbcScenePtr UMOpenGLAbcScene::create()
{
	return std::make_shared<UMOpenGLAbcScene>();
}

/**
 * constructor
 */
UMOpenGLAbcScene::UMOpenGLAbcScene() :
	impl_(new UMOpenGLAbcScene::SceneImpl())
{
}

/**
 * destructor
 */
UMOpenGLAbcScene::~UMOpenGLAbcScene()
{
}

/**
 * initialize
 */
bool UMOpenGLAbcScene::init(UMAbcScenePtr scene)
{
	return impl_->init(scene);
}

/**
 * release all scenes. call this function before delete.
 */
bool UMOpenGLAbcScene::dispose()
{
	impl_->dispose();
	impl_ = SceneImplPtr();
	return true;
}

/**
 * update
 */
bool UMOpenGLAbcScene::update(unsigned long time)
{
	return impl_->update(time);
}

/**
 * draw
 */
bool UMOpenGLAbcScene::draw()
{
	return impl_->draw();
}

/**
 * clear view/depth
 */
bool UMOpenGLAbcScene::clear()
{
	return impl_->clear();
}

} // umabc

#endif // WITH_OPENGL
