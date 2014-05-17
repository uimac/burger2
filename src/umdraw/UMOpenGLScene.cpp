﻿/**
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

#include <tchar.h>
#include <shlwapi.h>
#include <GL/glew.h>
#include "UMIO.h"

namespace umdraw
{

class UMOpenGLScene::Impl
{
	DISALLOW_COPY_AND_ASSIGN(Impl);
public:

	Impl() {}

	~Impl() {}

	bool init(UMScenePtr scene)
	{
		scene_ = scene;

		gl_mesh_group_list_.clear();
	
		// light
		UMOpenGLLightPtr light = UMOpenGLIO::convert_light_to_gl_light( scene->light_list().at(0) );
	
		// camera
		UMOpenGLCameraPtr camera = UMOpenGLIO::convert_camera_to_gl_camera(scene->camera());

		draw_parameter_ = std::make_shared<UMOpenGLDrawParameter>();
		draw_parameter_->set_light(light);
		draw_parameter_->set_camera(camera);

		// shader
		create_shaders_for_forward_render(scene, light, camera);
		//create_shaders_for_deferred_render(scene, light, camera);
		return true;
	}

	void clear(UMScenePtr scene)
	{
		render_buffer_ = UMOpenGLTexturePtr();
		frame_buffer_ = UMOpenGLTexturePtr();
		frame_buffer_textures_.clear();
		gl_board_list_.clear();
		gl_mesh_group_list_.clear();
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
	
	bool clear()
	{
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		return true;
	}

	bool update()
	{
		if (!shader_manager_) return false;
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
		{
			UMOpenGLLineList::iterator it = gl_line_list_.begin();
			for (; it != gl_line_list_.end(); ++it)
			{
				(*it)->update();
			}
		}
		// update models
		{
			UMOpenGLMeshGroupList::iterator it = gl_mesh_group_list_.begin();
			for (; it != gl_mesh_group_list_.end(); ++it)
			{
				(*it)->update();
			}
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

	bool draw()
	{
		if (!shader_manager_) return false;
		
		// deferred
		if (shader_manager_->is_deferred() && frame_buffer_)
		{
			/// pass1
			glBindFramebuffer( GL_FRAMEBUFFER, frame_buffer_->frame_buffer_id() );
			glDrawBuffers(static_cast<int>(frame_buffer_textures_.size()), 
				&(*frame_buffer_->frame_buffer_attachments().begin()));
			
			clear();

			// draw lines
			{
				UMOpenGLLineList::iterator it = gl_line_list_.begin();
				for (; it != gl_line_list_.end(); ++it)
				{
					(*it)->draw(deferred_mesh_geo_parameter_);
				}
			}
			// draw models
			{
				UMOpenGLMeshGroupList::iterator it = gl_mesh_group_list_.begin();
				for (; it != gl_mesh_group_list_.end(); ++it)
				{
					(*it)->draw(deferred_mesh_geo_parameter_);
				}
			}
		}
		else
		{
			// draw lines
			{
				UMOpenGLLineList::iterator it = gl_line_list_.begin();
				for (; it != gl_line_list_.end(); ++it)
				{
					draw_parameter_->set_shader_manager((*it)->shader_manager());
					(*it)->draw(draw_parameter_);
				}
			}
			// draw models
			{
				UMOpenGLMeshGroupList::iterator it = gl_mesh_group_list_.begin();
				for (; it != gl_mesh_group_list_.end(); ++it)
				{
					draw_parameter_->set_shader_manager(UMOpenGLShaderManagerPtr());
					(*it)->draw(draw_parameter_);
				}
			}
		}

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

		return true;
	}

	bool load(UMScenePtr scene)
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

	UMScenePtr scene() const 
	{
		return scene_;
	}
	
	void update(umbase::UMEventType event_type, umbase::UMAny& parameter)
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
	}

private:
	
	void create_shaders_for_deferred_render(UMScenePtr scene, UMOpenGLLightPtr light, UMOpenGLCameraPtr camera)
	{
		shader_manager_  = std::make_shared<UMOpenGLShaderManager>();
		if (shader_manager_->init(UMOpenGLShaderManager::eBoardForDeferred))
		{
			deferred_board_draw_parameter_ = std::make_shared<UMOpenGLDrawParameter>();
			deferred_board_draw_parameter_->set_light(light);
			deferred_board_draw_parameter_->set_shader_manager(shader_manager_);

			UMOpenGLShaderManagerPtr board_shader_manager = std::make_shared<UMOpenGLShaderManager>();
			if (board_shader_manager->init(UMOpenGLShaderManager::eBoard))
			{
				board_draw_parameter_ = std::make_shared<UMOpenGLDrawParameter>();
				board_draw_parameter_->set_light(light);
				board_draw_parameter_->set_shader_manager(board_shader_manager);
			}

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
			if (render_buffer_ = UMOpenGLTexture::create_render_buffer(width, height))
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
	
	void create_shaders_for_forward_render(UMScenePtr scene, UMOpenGLLightPtr light, UMOpenGLCameraPtr camera)
	{
		shader_manager_  = std::make_shared<UMOpenGLShaderManager>();
		shader_manager_->init(UMOpenGLShaderManager::eModel);
	}

	// umdraw scene
	UMScenePtr scene_;
	
	// shader
	UMOpenGLShaderManagerPtr shader_manager_;

	// drawable objects
	UMOpenGLMeshGroupList gl_mesh_group_list_;
	UMOpenGLLineList gl_line_list_;
	UMOpenGLBoardList gl_board_list_;

	UMOpenGLDrawParameterPtr draw_parameter_;
	UMOpenGLDrawParameterPtr deferred_board_draw_parameter_;
	UMOpenGLDrawParameterPtr board_draw_parameter_;
	UMOpenGLDrawParameterPtr deferred_mesh_geo_parameter_;

	// for deferred rendering
	UMOpenGLTexturePtr render_buffer_;
	UMOpenGLTexturePtr frame_buffer_;
	UMOpenGLTextureList frame_buffer_textures_;
};

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