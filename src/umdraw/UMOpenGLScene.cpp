/**
 * @file UMOpenGLScene.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_OPENGL

#include "UMOpenGLScene.h"
#include "UMOpenGL.h"
#include "UMOpenGLShaderManager.h"
#include "UMOpenGLTexture.h"
#include "UMOpenGLCamera.h"
#include "UMOpenGLLight.h"
#include "UMOpenGLDrawParameter.h"
#include "UMOpenGLLine.h"
#include "UMOpenGLBoard.h"
#include "UMOpenGLNode.h"
#include "UMPath.h"
#include "UMStringUtil.h"
#include "UMAny.h"
#include "UMSoftwareEventType.h"

#include "UMScene.h"
#include "UMTime.h"
#include "UMImage.h"
#include "UMOpenGLIO.h"
#include "UMMathTypes.h"
#include "UMMatrix.h"

#include <GL/glew.h>

namespace umdraw
{

class UMOpenGLScene::Impl
{
	DISALLOW_COPY_AND_ASSIGN(Impl);
public:

	Impl() {}

	~Impl() {}

	bool init(UMScenePtr scene);

	void clear(UMScenePtr scene);
	
	bool clear()
	{
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		return true;
	}

	bool update();

	bool draw();

	bool load(UMScenePtr scene);

	UMScenePtr scene() const 
	{
		return scene_;
	}
	
	void update(umbase::UMEventType event_type, umbase::UMAny& parameter);

private:
	
	void create_shaders_for_deferred_render(UMScenePtr scene, UMOpenGLLightPtr light, UMOpenGLCameraPtr camera);
	
	void create_shaders_for_forward_render(UMScenePtr scene, UMOpenGLLightPtr light, UMOpenGLCameraPtr camera);

	// umdraw scene
	UMScenePtr scene_;
	
	// shader
	UMOpenGLShaderManagerPtr shader_manager_;

	// drawable objects
	UMOpenGLMeshGroupList gl_mesh_group_list_;
	UMOpenGLNodeList gl_node_list_;
	UMOpenGLLineList gl_line_list_;
	UMOpenGLBoardList gl_board_list_;
	UMOpenGLBoardPtr foreground_board_;
	UMOpenGLBoardPtr background_board_;

	UMOpenGLDrawParameterPtr draw_parameter_;
	UMOpenGLDrawParameterPtr deferred_board_draw_parameter_;
	UMOpenGLDrawParameterPtr board_draw_parameter_;
	UMOpenGLDrawParameterPtr deferred_mesh_geo_parameter_;

	// for deferred rendering
	UMOpenGLTexturePtr render_buffer_;
	UMOpenGLTexturePtr frame_buffer_;
	UMOpenGLTextureList frame_buffer_textures_;
};

/** 
 * init scene impl
 */
bool UMOpenGLScene::Impl::init(UMScenePtr scene)
{
	scene_ = scene;

	gl_mesh_group_list_.clear();
	gl_node_list_.clear();
	
	// light
	UMOpenGLLightPtr light = UMOpenGLIO::convert_light_to_gl_light( scene->light_list().at(0) );
	
	// camera
	UMOpenGLCameraPtr camera = UMOpenGLIO::convert_camera_to_gl_camera(scene->camera());

	draw_parameter_ = std::make_shared<UMOpenGLDrawParameter>();
	draw_parameter_->set_light(light);
	draw_parameter_->set_camera(camera);

	UMOpenGLShaderManagerPtr board_shader_manager = std::make_shared<UMOpenGLShaderManager>();
	if (board_shader_manager->init(UMOpenGLShaderManager::eBoard))
	{
		board_draw_parameter_ = std::make_shared<UMOpenGLDrawParameter>();
		board_draw_parameter_->set_light(light);
		board_draw_parameter_->set_shader_manager(board_shader_manager);
	}

	// shader
	create_shaders_for_forward_render(scene, light, camera);
	//create_shaders_for_deferred_render(scene, light, camera);
		
	// create board for texture drawing
	{
		foreground_board_ = std::make_shared<UMOpenGLBoard>(
			UMVec2f(-1.0f, 1.0f),
			UMVec2f( 1.0f, -1.0f),
			-0.5f);
	}
	return true;
}

/**
 * clear scene impl
 */
void UMOpenGLScene::Impl::clear(UMScenePtr scene)
{
	render_buffer_ = UMOpenGLTexturePtr();
	frame_buffer_ = UMOpenGLTexturePtr();
	frame_buffer_textures_.clear();
	gl_board_list_.clear();
	gl_mesh_group_list_.clear();
	gl_node_list_.clear();
	gl_line_list_.clear();
	if (draw_parameter_)
	{
		draw_parameter_->set_light(UMOpenGLLightPtr());
	}
	if (deferred_mesh_geo_parameter_)
	{
		deferred_mesh_geo_parameter_->set_light(UMOpenGLLightPtr());
	}
}

/**
 * update scene impl
 */
bool UMOpenGLScene::Impl::update()
{
	if (!shader_manager_) return false;
	
	if (scene_ && scene_->is_enable_deform())
	{
		// deform mesh
		if (scene_->is_visible(UMScene::eMesh))
		{
			// update original mesh
			UMMeshGroupList::iterator it = scene_->mutable_mesh_group_list().begin();
			for (; it != scene_->mutable_mesh_group_list().end(); ++it)
			{
				UMMeshGroupPtr group = *it;
				UMMeshList::iterator mt = group->mutable_mesh_list().begin();
				for (; mt != group->mutable_mesh_list().end(); ++mt)
				{
					UMMeshPtr mesh = *mt;
					if (mesh)
					{
						mesh->update();
					}
				}
			}
			if (gl_mesh_group_list_.size() == scene_->mesh_group_list().size())
			{
				UMMeshGroupList::const_iterator it = scene_->mesh_group_list().begin();
				for (int i = 0; it != scene_->mesh_group_list().end(); ++it, ++i)
				{
					UMMeshGroupPtr mesh_group = *it;
					UMOpenGLMeshGroupPtr gl_mesh_group = gl_mesh_group_list_.at(i);
					UMMeshList::const_iterator mt = mesh_group->mesh_list().begin();
					for (int k = 0; mt != mesh_group->mesh_list().end(); ++mt, ++k)
					{
						UMOpenGLMeshPtr gl_mesh = gl_mesh_group->mutable_gl_mesh_list().at(k);
						if (gl_mesh)
						{
							UMOpenGLIO::deformed_mesh_to_gl_mesh(gl_mesh, *mt);
						}
					}
				}
			}
		}

		// deform node
		if (scene_->is_visible(UMScene::eNode))
		{
			if (gl_node_list_.size() == scene_->node_list().size())
			{
				UMNodeList::const_iterator it = scene_->node_list().begin();
				for (int i = 0; it != scene_->node_list().end(); ++it, ++i)
				{
					UMOpenGLNodePtr gl_node = gl_node_list_.at(i);
					if (gl_node)
					{
						UMOpenGLIO::deformed_node_to_gl_node(gl_node, *it);
					}
				}
			}
		}
	}

	if (draw_parameter_)
	{
		// update lights
		{
			if (UMOpenGLLightPtr light = draw_parameter_->light())
			{
				light->update();
			}
		}

		// update camera
		if (UMOpenGLCameraPtr camera = draw_parameter_->camera())
		{
			camera->update();
		}
	}
	// update lines
	if (scene_->is_visible(UMScene::eLine))
	{
		UMOpenGLLineList::iterator it = gl_line_list_.begin();
		for (; it != gl_line_list_.end(); ++it)
		{
			(*it)->update();
		}
	}
	// update models
	if (scene_->is_visible(UMScene::eMesh))
	{
		UMOpenGLMeshGroupList::iterator it = gl_mesh_group_list_.begin();
		for (; it != gl_mesh_group_list_.end(); ++it)
		{
			(*it)->update();
		}
	}
	// update foreground
	if (foreground_board_ && foreground_board_->texture())
	{
		foreground_board_->update();
	}
	// update boards
	{
		UMOpenGLBoardList::iterator it = gl_board_list_.begin();
		for (; it != gl_board_list_.end(); ++it)
		{
			(*it)->update();
		}
	}
	return true;
}

/**
 * draw scene impl
 */
bool UMOpenGLScene::Impl::draw()
{
	if (!shader_manager_) return false;
		
	// deferred
#if !defined(WITH_EMSCRIPTEN)
	if (shader_manager_->is_deferred() && frame_buffer_)
	{
		/// pass1
		glBindFramebuffer( GL_FRAMEBUFFER, frame_buffer_->frame_buffer_id() );
		glDrawBuffers(static_cast<int>(frame_buffer_textures_.size()), 
			&(*frame_buffer_->frame_buffer_attachments().begin()));
			
		clear();

		// draw lines
		if (scene_->is_visible(UMScene::eLine))
		{
			UMOpenGLLineList::iterator it = gl_line_list_.begin();
			for (; it != gl_line_list_.end(); ++it)
			{
				(*it)->draw(deferred_mesh_geo_parameter_);
			}
		}
		// draw models
		if (scene_->is_visible(UMScene::eMesh))
		{
			UMOpenGLMeshGroupList::iterator it = gl_mesh_group_list_.begin();
			for (; it != gl_mesh_group_list_.end(); ++it)
			{
				(*it)->draw(deferred_mesh_geo_parameter_);
			}
		}
	}
	else
#endif // !defined(WITH_EMSCRIPTEN)
	{
		{
			// draw lines
			if (scene_->is_visible(UMScene::eLine))
			{
				UMOpenGLLineList::iterator it = gl_line_list_.begin();
				for (; it != gl_line_list_.end(); ++it)
				{
					draw_parameter_->set_shader_manager((*it)->shader_manager());
					(*it)->draw(draw_parameter_);
				}
			}
			// draw models
			if (scene_->is_visible(UMScene::eMesh))
			{
				UMOpenGLMeshGroupList::iterator it = gl_mesh_group_list_.begin();
				for (; it != gl_mesh_group_list_.end(); ++it)
				{
					draw_parameter_->set_shader_manager(UMOpenGLShaderManagerPtr());
					(*it)->draw(draw_parameter_);
				}
			}
			// draw nodes
			if (scene_->is_visible(UMScene::eNode))
			{
				UMOpenGLNodeList::iterator it = gl_node_list_.begin();
				for (; it != gl_node_list_.end(); ++it)
				{
					draw_parameter_->set_shader_manager((*it)->shader_manager());
					(*it)->draw(draw_parameter_);
				}
			}

			// draw foreground
			if (foreground_board_ && foreground_board_->texture())
			{
				glDepthMask(GL_FALSE);
				foreground_board_->draw(board_draw_parameter_);
				glDepthMask(GL_TRUE);
			}
		}
	}
		
#if !defined(WITH_EMSCRIPTEN)
	// deferred
	if (shader_manager_->is_deferred())
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
			
		glDepthMask(GL_FALSE);
		// draw board
		{
			gl_board_list_[0]->draw(deferred_board_draw_parameter_);
		}
		// draw debug board
		{
			for (int i = 1; i < static_cast<int>(gl_board_list_.size()); ++i)
			{
				gl_board_list_.at(i)->draw(board_draw_parameter_);
			}
		}
		glDepthMask(GL_TRUE);
	}
#endif // !defined(WITH_EMSCRIPTEN)

	return true;
}

/**
 * load scene
 */
bool UMOpenGLScene::Impl::load(UMScenePtr scene)
{
	{
		UMMeshGroupList::const_iterator it = scene->mesh_group_list().begin();
		for (; it != scene->mesh_group_list().end(); ++it)
		{
			UMMeshGroupPtr mesh_group = *it;
			UMOpenGLMeshGroupPtr gl_mesh_group = UMOpenGLIO::convert_mesh_group_to_gl_mesh_group(mesh_group);
			if (gl_mesh_group) {
				gl_mesh_group_list_.push_back(gl_mesh_group);
			}
		}
	}

	{
		UMNodeList::const_iterator it = scene->node_list().begin();
		for (; it != scene->node_list().end(); ++it)
		{
			UMNodePtr node = *it;
			UMOpenGLNodePtr gl_node = UMOpenGLIO::convert_node_to_gl_node(node);
			if (gl_node) {
				gl_node->init();
				gl_node_list_.push_back(gl_node);
			}
		}
	}

	{
		UMLineList::const_iterator it = scene->line_list().begin();
		for (; it != scene->line_list().end(); ++it)
		{
			UMLinePtr line = *it;
			UMOpenGLLinePtr gl_line = UMOpenGLIO::convert_line_to_gl_line(line);
			if (gl_line) {
				gl_line->init();
				gl_line_list_.push_back(gl_line);
			}
		}
	}
	return true;
}

/**
 * update
 */
void UMOpenGLScene::Impl::update(umbase::UMEventType event_type, umbase::UMAny& parameter)
{
	if (event_type == eSoftwareEventCameraChaged)
	{
		if (scene_ && draw_parameter_)
		{
			// camera
			UMOpenGLCameraPtr camera = UMOpenGLIO::convert_camera_to_gl_camera(scene_->camera());
			draw_parameter_->set_camera(camera);
		}
	}
	else if (event_type == eSoftwareEventForegroundChaged)
	{
		if (scene_ && draw_parameter_)
		{
			UMImagePtr image = umbase::any_cast<UMImagePtr>(parameter);
			if (image)
			{
				UMOpenGLTexturePtr texture = std::make_shared<UMOpenGLTexture>(false);
				texture->convert_from_image(*image);
				foreground_board_->set_texture(texture);
			}
			else
			{
				foreground_board_->set_texture(UMOpenGLTexturePtr());
			}
		}
	}
}

/**
 * create shader for deferred
 */
void UMOpenGLScene::Impl::create_shaders_for_deferred_render(UMScenePtr scene, UMOpenGLLightPtr light, UMOpenGLCameraPtr camera)
{
	shader_manager_  = std::make_shared<UMOpenGLShaderManager>();
	if (shader_manager_->init(UMOpenGLShaderManager::eBoardForDeferred))
	{
		deferred_board_draw_parameter_ = std::make_shared<UMOpenGLDrawParameter>();
		deferred_board_draw_parameter_->set_light(light);
		deferred_board_draw_parameter_->set_shader_manager(shader_manager_);

		UMOpenGLShaderManagerPtr mesh_geo_shader_manager = std::make_shared<UMOpenGLShaderManager>();
		if (mesh_geo_shader_manager->init(UMOpenGLShaderManager::eModelDeferredGeo))
		{
			deferred_mesh_geo_parameter_ = std::make_shared<UMOpenGLDrawParameter>();
			deferred_mesh_geo_parameter_->set_light(light);
			deferred_mesh_geo_parameter_->set_camera(camera);
			deferred_mesh_geo_parameter_->set_shader_manager(mesh_geo_shader_manager);
		}

		const int width = scene->width();
		const int height = scene->height();
		render_buffer_ = UMOpenGLTexture::create_render_buffer(width, height);
		if (render_buffer_)
		{
			UMOpenGLTexture::Format format;
			format.internal_format = GL_RGBA32F;
			for (int i = 0; i < 4; ++i)
			{
				if (UMOpenGLTexturePtr texture = UMOpenGLTexture::create_texture(width, height, format))
				{
					frame_buffer_textures_.push_back(texture);
				}
			}
			// create board for texture drawing
			{
				UMOpenGLBoardPtr board = std::make_shared<UMOpenGLBoard>(
					UMVec2f(-1.0f, 1.0f),
					UMVec2f( 1.0f, -1.0f),
					-0.5f);
				board->set_color_attachments(frame_buffer_textures_);
				gl_board_list_.push_back(board);
			}
			// create board for debug drawing
			{
				UMOpenGLBoardPtr board = std::make_shared<UMOpenGLBoard>(
					UMVec2f(-0.2f + 0.6f, 0.2f + 0.6f),
					UMVec2f( 0.2f + 0.6f, -0.2f + 0.6f),
					-0.2f);
				board->set_texture(frame_buffer_textures_.at(0));
				gl_board_list_.push_back(board);
			}
			{
				UMOpenGLBoardPtr board = std::make_shared<UMOpenGLBoard>(
					UMVec2f(-0.2f + 0.6f, 0.2f + 0.2f),
					UMVec2f( 0.2f + 0.6f, -0.2f + 0.2f),
					-0.2f);
				board->set_texture(frame_buffer_textures_.at(1));
				gl_board_list_.push_back(board);
			}
			{
				UMOpenGLBoardPtr board = std::make_shared<UMOpenGLBoard>(
					UMVec2f(-0.2f + 0.6f, 0.2f - 0.2f),
					UMVec2f( 0.2f + 0.6f, -0.2f - 0.2f),
					-0.2f);
				board->set_texture(frame_buffer_textures_.at(2));
				gl_board_list_.push_back(board);
			}
			frame_buffer_ = UMOpenGLTexture::create_frame_buffer(frame_buffer_textures_, render_buffer_);
		}
	}
}

/**
 * create shader for forward
 */
void UMOpenGLScene::Impl::create_shaders_for_forward_render(UMScenePtr scene, UMOpenGLLightPtr light, UMOpenGLCameraPtr camera)
{
	shader_manager_  = std::make_shared<UMOpenGLShaderManager>();
	shader_manager_->init(UMOpenGLShaderManager::eModel);
}

UMOpenGLScene::UMOpenGLScene()
	: impl_(new UMOpenGLScene::Impl())
{
}

UMOpenGLScene::~UMOpenGLScene()
{
}

/**
 * init scene
 */
bool UMOpenGLScene::init(UMScenePtr scene)
{
	return impl_->init(scene);
}

/**
 * clear scene
 */
void UMOpenGLScene::clear(UMScenePtr scene)
{
	impl_->clear(scene);
}

/**
 * update scene
 */
bool UMOpenGLScene::update()
{
	return impl_->update();
}

bool UMOpenGLScene::clear()
{
	return impl_->clear();
}

/**
 * draw scene
 */
bool UMOpenGLScene::draw()
{
	return impl_->draw();
}

/**
 * load scene
 */
bool UMOpenGLScene::load(UMScenePtr scene)
{
	return impl_->load(scene);
}

/**
 * get umdraw renderer scene
 */
UMScenePtr UMOpenGLScene::scene() const 
{
	return impl_->scene();
}

/**
 * update event
 */
void UMOpenGLScene::update(umbase::UMEventType event_type, umbase::UMAny& parameter)
{
	impl_->update(event_type, parameter);
}

} // umdraw

#endif // WITH_OPENGL
