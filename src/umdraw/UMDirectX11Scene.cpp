/**
 * @file UMDirectX11Scene.cpp
 *
 * @author tori31001 at gmail.com
 *
 * Copyright (C) 2013 Kazuma Hatta
 * Licensed  under the MIT license. 
 *
 */
#ifdef WITH_DIRECTX

#include "UMDirectX11Scene.h"

#include "UMMesh.h"
#include "UMMeshGroup.h"
#include "UMTga.h"
#include "UMAny.h"
#include "UMSoftwareEventType.h"

#include "UMIO.h"
#include "UMStringUtil.h"
#include "UMTime.h"
#include "UMScene.h"

#include "UMDirectX11.h"
#include "UMDirectX11ShaderManager.h"
#include "UMDirectX11Texture.h"
#include "UMDirectX11Board.h"
#include "UMDirectX11IO.h"
#include "UMDirectX11Line.h"
#include "UMDirectX11MeshGroup.h"

namespace umdraw
{

class UMDirectX11Scene::Impl
{
	DISALLOW_COPY_AND_ASSIGN(Impl);
public:
	Impl()
		: shader_manager_(std::make_shared<UMDirectX11ShaderManager>())
		, line_shader_manager_(std::make_shared<UMDirectX11ShaderManager>())
		, line_draw_parameter_(std::make_shared<UMDirectX11DrawParameter>())
		, default_sampler_state_pointer_(NULL)
	{}

	~Impl() 
	{
		SAFE_RELEASE(default_sampler_state_pointer_);
	}

	bool init(ID3D11Device *device_pointer, ID3D11Texture2D* back_buffer, UMScenePtr scene)
	{
		if (!device_pointer) return false;
		if (!back_buffer) return false;
		if (!scene) return false;
		if (!shader_manager_) return false;
		if (!line_shader_manager_) return false;
	
		scene_ = scene;

		ID3D11DeviceContext *device_context_pointer = NULL;
		device_pointer->GetImmediateContext(&device_context_pointer);

		dx11_mesh_group_list_.clear();
		dx11_light_list_.clear();
	
		// light
		UMDirectX11LightPtr light = UMDirectX11IO::convert_light_to_dx11_light(device_pointer, scene->light_list().at(0));
		dx11_light_list_.push_back(light);

		// camera
		camera_ = UMDirectX11IO::convert_camera_to_dx11_camera(device_pointer, scene->camera());

		// init shader manager for constants
		shader_manager_->init(device_pointer, UMDirectX11ShaderManager::eConstants);
	
		// init line shader manager
		line_shader_manager_->init(device_pointer, UMDirectX11ShaderManager::eLine);
		line_draw_parameter_->set_shader_manager(line_shader_manager_);
	
		{
			D3D11_SAMPLER_DESC sampler_desc;
			::ZeroMemory(&sampler_desc, sizeof(D3D11_SAMPLER_DESC));
			sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			sampler_desc.MipLODBias = 0;
			sampler_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
			sampler_desc.MinLOD = 0;
			sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

			if FAILED(device_pointer->CreateSamplerState(
				&sampler_desc, 
				&default_sampler_state_pointer_ ))
			{
				SAFE_RELEASE(device_context_pointer);
				return false;
			}
			line_draw_parameter_->set_default_sampler_state(default_sampler_state_pointer_);
		}

		// create render taget
		depth_stencil_texture_ = UMDirectX11Texture::create_depth_stencil(device_pointer, back_buffer);
		render_target_texture_ = UMDirectX11Texture::create_render_target(device_pointer, back_buffer, depth_stencil_texture_);
		if (!depth_stencil_texture_) { return false; }
		if (!render_target_texture_) { return false; }
		
		// set render target
		{
			// set render target view
			ID3D11RenderTargetView* render_target = render_target_texture_->render_target_view_pointer();
			ID3D11DepthStencilView* depth_scentil = depth_stencil_texture_->depth_stencil_view_pointer();
			device_context_pointer->OMSetRenderTargets( 1, &render_target, depth_scentil ); 
		}

		SAFE_RELEASE(device_context_pointer);

		return true;
	}

	void clear(ID3D11Device *device_pointer)
	{
		ID3D11DeviceContext* device_context_pointer;
		device_pointer->GetImmediateContext(&device_context_pointer);
		float clear_color[] = { 0.21f, 0.21f, 0.21f, 1.0f };
		device_context_pointer->ClearRenderTargetView( render_target_texture_->render_target_view_pointer(), clear_color );
		device_context_pointer->ClearDepthStencilView( depth_stencil_texture_->depth_stencil_view_pointer(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );
		SAFE_RELEASE(device_context_pointer);
	}

	bool update(ID3D11Device* device_pointer)
	{
		if (!device_pointer) return false;
	
		// update lights
		{
			UMDirectX11LightList::iterator it = dx11_light_list_.begin();
			for (; it != dx11_light_list_.end(); ++it)
			{
				UMDirectX11LightPtr light = (*it);
				light->update(device_pointer);
			}
		}
	
		// update camera
		if (camera_)
		{
			camera_->update(device_pointer);
		}
	
		// update models
		{
			UMDirectX11MeshGroupList::iterator it = dx11_mesh_group_list_.begin();
			for (; it != dx11_mesh_group_list_.end(); ++it)
			{
				(*it)->update(device_pointer);
			}
		}

		return true;
	}

	bool draw(ID3D11Device* device_pointer)
	{
		if (!device_pointer) return false;
		if (!shader_manager_) return false;

		ID3D11DeviceContext *device_context_pointer = NULL;
		device_pointer->GetImmediateContext(&device_context_pointer);
	
		// apply constant buffer
		const UMDirectX11ShaderManager::BufferPointerList& constant_buffer = shader_manager_->constant_buffer_list();
		if (!constant_buffer.empty())
		{
			// put camera to constant buffer
			{
				ID3D11Buffer* constant = constant_buffer[0];
				UMDirectX11CameraPtr camera = camera_;
				device_context_pointer->UpdateSubresource(constant, 0, NULL, &(*camera), 0, 0);
				device_context_pointer->VSSetConstantBuffers( 0, 1, &constant );
			}

			// put light to constant buffer
			{
				ID3D11Buffer* constant = constant_buffer[1];
				UMDirectX11LightPtr light = dx11_light_list_.front();
				device_context_pointer->UpdateSubresource(constant, 0, NULL, &(*light), 0, 0);
				device_context_pointer->VSSetConstantBuffers( 1, 1, &constant );
				device_context_pointer->PSSetConstantBuffers( 1, 1, &constant );
			}
		}
	
		// draw lines
		if (scene_->is_visible(UMScene::eLine))
		{
			UMDirectX11LineList::const_iterator it = dx11_line_list_.begin();
			for (; it != dx11_line_list_.end(); ++it)
			{
				(*it)->draw(device_pointer, line_draw_parameter_);
			}
		}
		// draw models
		if (scene_->is_visible(UMScene::eMesh))
		{
			UMDirectX11MeshGroupList::iterator it = dx11_mesh_group_list_.begin();
			for (; it != dx11_mesh_group_list_.end(); ++it)
			{
				(*it)->draw(device_pointer, UMDirectX11DrawParameterPtr());
			}
		}

		SAFE_RELEASE(device_context_pointer);

		return true;
	}

	bool load(
		ID3D11Device* device_pointer, 
		UMScenePtr scene)
	{
		{
			UMMeshGroupList::const_iterator it = scene->mesh_group_list().begin();
			for (; it != scene->mesh_group_list().end(); ++it)
			{
				UMMeshGroupPtr mesh_group = *it;
				// import to directx11
				UMDirectX11MeshGroupPtr dx11_mesh_group = UMDirectX11IO::convert_mesh_group_to_dx11_mesh_group(device_pointer, mesh_group);
				if (dx11_mesh_group) {
					dx11_mesh_group_list_.push_back(dx11_mesh_group);
				}
			}
		}
		{
			UMLineList::const_iterator it = scene->line_list().begin();
			for (; it != scene->line_list().end(); ++it)
			{
				UMLinePtr line = *it;
				UMDirectX11LinePtr dx11_line = UMDirectX11IO::convert_line_to_dx11_line(device_pointer, line);
				if (dx11_line) {
					dx11_line_list_.push_back(dx11_line);
				}
			}
		}
		return true;
	}

	void update(umbase::UMEventType event_type, umbase::UMAny& parameter)
	{
		if (event_type == eSoftwareEventCameraChaged)
		{
			if (scene_)
			{
				if (ID3D11Device* device_pointer = reinterpret_cast<ID3D11Device*>(umdraw::UMDirectX11::current_device_pointer()))
				{
					// camera
					camera_ = UMDirectX11IO::convert_camera_to_dx11_camera(device_pointer, scene_->camera());
				}
			}
		}
	}
	
	UMScenePtr scene() const
	{
		return scene_;
	}

private:

	// umdraw scene
	UMScenePtr scene_;
	
	// shader
	UMDirectX11ShaderManagerPtr shader_manager_;
	UMDirectX11ShaderManagerPtr line_shader_manager_;
	UMDirectX11DrawParameterPtr line_draw_parameter_;
	
	UMDirectX11DrawParameterPtr deferred_board_draw_parameter_;
	UMDirectX11DrawParameterPtr deferred_mesh_geo_parameter_;

	// drawable objects
	UMDirectX11MeshGroupList dx11_mesh_group_list_;
	UMDirectX11LineList dx11_line_list_;
	UMDirectX11LightList dx11_light_list_;
	UMDirectX11BoardPtr board_;
	UMDirectX11CameraPtr camera_;

	ID3D11SamplerState * default_sampler_state_pointer_;

	// render targets
	UMDirectX11TexturePtr render_target_texture_;
	UMDirectX11TexturePtr depth_stencil_texture_;
	// for deferred rendering
	UMDirectX11TexturePtr deferred_depth_stencil_texture_;
	UMDirectX11TextureList deferred_textures_;
};

UMDirectX11Scene::UMDirectX11Scene()
	: impl_(new UMDirectX11Scene::Impl)
{
}

UMDirectX11Scene::~UMDirectX11Scene()
{
}

/**
 * init scene
 */
bool UMDirectX11Scene::init(ID3D11Device *device_pointer, ID3D11Texture2D* back_buffer, UMScenePtr scene)
{
	return impl_->init(device_pointer, back_buffer, scene);
}

void UMDirectX11Scene::clear(ID3D11Device *device_pointer)
{
	impl_->clear(device_pointer);
}

/**
 * update scene
 */
bool UMDirectX11Scene::update(ID3D11Device* device_pointer)
{
	return impl_->update(device_pointer);
}

/**
 * draw scene
 */
bool UMDirectX11Scene::draw(ID3D11Device* device_pointer)
{
	return impl_->draw(device_pointer);
}

/**
 * load scene
 */
bool UMDirectX11Scene::load(
	ID3D11Device* device_pointer, 
	UMScenePtr scene)
{
	return impl_->load(device_pointer, scene);
}

/**
 * update event
 */
void UMDirectX11Scene::update(umbase::UMEventType event_type, umbase::UMAny& parameter)
{
	impl_->update(event_type, parameter);
}

/** 
 * get scene
 */
UMScenePtr UMDirectX11Scene::scene() const
{
	return impl_->scene();
}

} // umdraw

#endif // WITH_DIRECTX
